#include "thread3.hpp"

namespace sf
{
namespace native_mod
{
namespace Thread
{

static Vec<ThreadHandle *> v_handles; /* stores indexes of worker jobs */
static Vec<size_t> idx_avl;           /* indices available */

static std::mutex cre_mutex; /* mutex for create () and close () */
static std::mutex wl_mutex;  /* mutex for worker_loop */
static std::mutex thr_mutex; /* threadhandle->run mutex */

static std::condition_variable t_cv;
static std::queue<ThreadHandle *> q_jobs;
static std::vector<std::thread> v_workers;
static bool shutting_down = false;

void
ThreadHandle::run ()
{
  {
    std::lock_guard<std::mutex> lock (thr_mutex);
    q_jobs.push (this);
  }
  t_cv.notify_one ();
}

static void
worker_loop ()
{
  while (1)
    {
      ThreadHandle *th = nullptr;

      {
        std::unique_lock<std::mutex> lock (wl_mutex);
        t_cv.wait (lock, [] { return shutting_down || !q_jobs.empty (); });

        if (shutting_down && q_jobs.empty ())
          {
            return;
          }

        th = q_jobs.front ();
        q_jobs.pop ();
      }

      if (th == nullptr)
        return;

      Module *m = new Module (ModuleType::Function);
      m->set_parent (th->get_mod ());
      m->get_code_lines () = th->get_mod ()->get_code_lines ();
      m->get_file_path () = th->get_mod ()->get_file_path ();

      try
        {
          Object *ret = call_func (
              *m, th->get_name (),
              static_cast<ArrayObject *> (th->get_args ())->get_vals ());

          if (m->get_saw_ambig ())
            {
              Object *amb = m->get_ambig ();

              std::cerr << "Error thrown in a thread:\n";

              std::cerr << "\n====== EXECUTION ERROR ======\n";
              std::cerr << "Error: Uncaught Ambiguity\n";

              if (OBJ_IS_AMBIG (amb))
                {
                  AmbigObject *ao = static_cast<AmbigObject *> (amb);
                  if (ao->get_val () != nullptr)
                    std::cerr
                        << "Value: " << ao->get_val ()->get_stdout_repr ()
                        << "\n";
                }
              else if (amb != nullptr)
                {
                  std::cerr << "Associated Object: ";
                  amb->print ();
                }

              std::cerr << "\n------ Backtrace ------\n";
              if (!m->get_backtrace ().get_size ())
                {
                  std::cerr << "No backtrace information available.\n";
                }
              else
                {
                  // for (int i = 0; i < m->get_backtrace ().get_size (); i++)
                  //   {
                  //     std::cerr << "#" << (i + 1) << " ";
                  //     if (m->get_backtrace ()[i] < m->get_code_lines
                  //     ().get_size
                  //     ()
                  //         && m->get_backtrace ()[i] - 1
                  //                < m->get_code_lines ().get_size ())
                  //       {

                  //       }
                  //     else
                  //       {
                  //         std::cerr << "Line " << m->get_backtrace ()[i]
                  //                   << ": <source not available>\n";
                  //       }
                  //   }

                  for (int i = 0; i < m->get_backtrace ().get_size (); i++)
                    {
                      std::string s = m->get_backtrace ()[i]
                                          .second.first.to_std_string ()
                                          .c_str ();
                      while (s.front () == ' ' || s.front () == '\t')
                        s.erase (s.begin ());

                      std::cerr << "Line "
                                << (m->get_backtrace ()[i].first + 1) << ": "
                                << s << "\n";
                    }
                }
              std::cerr << "========================\n\n";
            }
          // else
          DR (ret);
        }
      catch (const std::exception &e)
        {
          std::cerr << e.what () << '\n';
        }

      th->get_done () = true;

      DR (th->get_name ());
      DR (th->get_args ());

      /**
       * closed before thread finished
       * user likely called .close () before .join ()
       * detached thread
       */
      if (th->get_is_closed ())
        {
          size_t id = th->get_id ();

          {
            std::lock_guard<std::mutex> lock (cre_mutex);
            delete v_handles[id];
            v_handles[id] = nullptr;
            idx_avl.push_back (id);
          }
        }

      m->set_parent (nullptr);
      delete m;
    }
}

SF_API void
init_runtime_threads ()
{
  size_t n = std::thread::hardware_concurrency ();

  if (!n)
    n = 4;

  for (size_t i = 0; i < n; i++)
    v_workers.emplace_back (worker_loop);
}

SF_API Object *
create (Module *mod)
{
  /**
   * Since we are writing to v_handles,
   * we need a mutex lock
   */
  std::lock_guard<std::mutex> lock (cre_mutex);

  Object *o_fname = mod->get_variable ("fname");
  Object *o_fargs = mod->get_variable ("fargs");

  assert (o_fname->get_type () == ObjectType::FuncObject);
  assert (o_fargs->get_type () == ObjectType::ArrayObj);

  // std::cout << "lines: "
  //           << static_cast<FunctionObject *> (o_fname)
  //                  ->get_v ()
  //                  ->get_parent ()
  //                  ->get_code_lines ()
  //                  .get_size ()
  //           << '\n';
  ThreadHandle *th = new ThreadHandle (
      o_fname, o_fargs,
      static_cast<FunctionObject *> (o_fname)->get_v ()->get_parent ());

  /**
   * check if any index is available
   * Index availability means the worker
   * job at that index has completed.
   * We can reuse that vector position
   */
  size_t idx;
  if (idx_avl.get_size ())
    {
      size_t p = idx_avl.pop_back ();
      v_handles[p] = th;
      idx = p;
    }
  else
    {
      idx = v_handles.get_size ();
      v_handles.push_back (th);
    }

  th->get_id () = idx;

  Object *ret = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new IntegerConstant (static_cast<int> (idx)))));

  IR (ret);
  return ret;
}

SF_API Object *
run (Module *mod)
{
  Object *o_id = mod->get_variable ("id");
  assert (OBJ_IS_INT (o_id));

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  //   std::cout << id << '\t' << v_handles.get_size () << '\n';
  assert (id < v_handles.get_size ());
  v_handles[id]->run ();

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
join (Module *mod)
{
  Object *o_id = mod->get_variable ("id");
  assert (OBJ_IS_INT (o_id));

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  assert (id < v_handles.get_size ());

  ThreadHandle *&th = v_handles[id];

  /* TODO: change this, wastes CPU cycles */
  while (!th->get_done ())
    ;

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
join_all (Module *mod)
{
  for (ThreadHandle *&th : v_handles)
    {
      if (th == nullptr)
        continue;

      while (!th->get_done ())
        ;
    }

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
close (Module *mod)
{
  std::lock_guard<std::mutex> close (cre_mutex);
  Object *o_id = mod->get_variable ("id");
  assert (OBJ_IS_INT (o_id));

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  ThreadHandle *&th = v_handles[id];

  if (id < v_handles.get_size ()
      && th != nullptr) /* release only when thread execution is done */
    {
      if (th->get_done () && !th->get_is_closed ())
        {
          delete th;
          th = nullptr;
          idx_avl.push_back (id);
        }
      else if (!th->get_done ())
        {
          th->get_is_closed () = true; /* detach */
        }
    }

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
__sf_thread_cleanup ()
{
  {
    std::lock_guard<std::mutex> lock (wl_mutex);
    shutting_down = true;
  }

  t_cv.notify_all ();

  for (std::thread &t : v_workers)
    if (t.joinable ())
      t.join ();

  v_workers.clear ();
}
} // namespace Thread
} // namespace native_mod
} // namespace sf

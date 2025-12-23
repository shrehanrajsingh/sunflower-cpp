#include "thread2.hpp"

namespace sf
{
namespace native_mod
{
namespace Thread
{
static std::queue<ThreadTask *> q_tasks;
static std::mutex q_mutex;
static std::condition_variable q_cv;

static std::vector<std::thread> workers;

static bool shutting_down = false;
static size_t tmap_id = 1;

static std::map<size_t, ThreadHandle *> threadmap;

ThreadHandle::ThreadHandle (Object *fname, Object *fargs, Module *mod)
{
  IR (fname);
  IR (fargs);

  task = new ThreadTask{ fname, fargs, mod /* , std::promise<Object *> () */ };
  //   fut = task->res.get_future ();
  id = ++tmap_id;
  threadmap[id] = this;
}

void
ThreadHandle::run ()
{
  if (scheduled)
    return;

  scheduled = true;
  {
    std::lock_guard<std::mutex> lock (q_mutex);
    q_tasks.push (task);
  }

  q_cv.notify_one ();
}

ThreadHandle::~ThreadHandle ()
{
  DR (task->fname);
  DR (task->args);

  delete task;
  //   if (threadmap.count (id))
}

static void
worker_loop ()
{
  while (1)
    {
      ThreadTask *tt = nullptr;

      {
        std::unique_lock<std::mutex> lock (q_mutex);
        q_cv.wait (lock, [] { return shutting_down || !q_tasks.empty (); });

        if (shutting_down && q_tasks.empty ())
          return;

        tt = q_tasks.front ();
        q_tasks.pop ();
      }

      assert (tt != nullptr);
      Module *m = new Module (ModuleType::Function);
      m->set_parent (tt->mod);

      try
        {
          ArrayObject *ao = static_cast<ArrayObject *> (tt->args);

          Object *ret
              = call_func (*m, tt->fname,
                           static_cast<ArrayObject *> (tt->args)->get_vals ());

          //   DR (ret);
          // tt->res.set_value (nullptr);
        }
      catch (const std::exception &e)
        {
          std::cerr << e.what () << '\n';
        }

      tt->done = true;
      //   tt->res.set_value (nullptr);
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
    workers.emplace_back (worker_loop);
}

SF_API Object *
create (Module *mod)
{
  Object *fname = mod->get_variable ("fname");
  Object *fargs = mod->get_variable ("fargs");

  assert (fname->get_type () == ObjectType::FuncObject);
  assert (fargs->get_type () == ObjectType::ArrayObj);

  ThreadHandle *handle = new ThreadHandle (fname, fargs, mod->get_parent ());

  Object *ret
      = static_cast<Object *> (new ConstantObject (static_cast<Constant *> (
          new IntegerConstant (static_cast<int> (handle->get_id ())))));

  IR (ret);
  return ret;
}

void
_run_cf_rt (ThreadHandle *th, std::promise<Object *> &prm, Module *m,
            Object *fname, Vec<Object *> &vobj)
{
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

  assert (threadmap.count (id));

  //   Object *res = threadmap[id]->join ();
  //   Object *robj = threadmap[id]->join ();
  //   threadmap[id]->join (); /* returns nullptr */
  while (!threadmap[id]->get_task ()->done)
    ;

  delete threadmap[id];
  threadmap.erase (id);

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

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

  assert (threadmap.count (id));
  threadmap[id]->run ();

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
join_all (Module *mod)
{
  for (auto &[id, th] : threadmap)
    {
      Object *o = th->join ();
      DR (o);

      delete th;
    }

  threadmap.clear ();

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
close (Module *mod)
{
  Object *o_id = mod->get_variable ("id");
  assert (OBJ_IS_INT (o_id));

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  if (threadmap.count (id))
    {
      delete threadmap[id];
      threadmap.erase (id);
    }

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
__sf_thread_cleanup ()
{
}
} // namespace Thread
} // namespace native_mod
} // namespace sf

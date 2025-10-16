#include "thread.hpp"

namespace sf
{
namespace native_mod
{
namespace Thread
{
static std::map<size_t, ThreadHandle *> threadmap;
static size_t tmap_id = 0;

SF_API Object *
create (Module *mod)
{
  /**
   * create (function_name, array_containing_function_args)
   */

  Object *fname = mod->get_variable ("fname");
  Object *fargs = mod->get_variable ("fargs");

  assert (fname->get_type () == ObjectType::FuncObject
          && "Callable in thread must be a function");

  ThreadHandle *th = new ThreadHandle (
      fname, fargs,
      mod->get_parent ()); /* TODO: mod instance might be different */

  th->get_future () = th->get_promise ().get_future ();

  size_t mapid = tmap_id;
  threadmap[tmap_id++] = th;

  Object *res
      = static_cast<Object *> (new ConstantObject (static_cast<Constant *> (
          new IntegerConstant (static_cast<int> (mapid)))));

  IR (res);
  return res;
}

void
_run_cf_rt (ThreadHandle *th, std::promise<Object *> &prm, Module *m,
            Object *fname, Vec<Object *> &vobj)
{
  prm.set_value (th->get_ret () = call_func (*m, fname, vobj));
  th->set_has_result (true);
  IR (th->get_ret ());
  delete m;
}

SF_API Object *
join (Module *mod)
{
  Object *o_id = mod->get_variable ("id");

  assert (OBJ_IS_INT (o_id) && "thread id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  assert (threadmap.count (id) && "invalid thread id");

  ThreadHandle *th = threadmap[id];

  //   assert (th->get_th ().joinable () && "thread is not joinable");
  if (!th->get_th ().joinable ())
    {
      Object *o = new ConstantObject (new NoneConstant ());
      IR (o);

      return o;
    }

  th->get_th ().join ();
  Object *ret = th->get_ret ();
  assert (ret != nullptr);

  return ret;

  //   Object *o = new ConstantObject (new NoneConstant ());
  //   IR (o);

  //   return o;
}

SF_API Object *
run (Module *mod)
{
  Object *o_id = mod->get_variable ("id");

  assert (OBJ_IS_INT (o_id) && "thread id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  assert (threadmap.count (id) && "invalid thread id");

  ThreadHandle *th = threadmap[id];

  Object *fname = th->get_fname ();
  Object *fargs = th->get_fargs ();
  Module *fmod = th->get_mod ();

  Module *nmod = new Module (ModuleType::Function);
  nmod->set_parent (fmod);

  assert (fargs->get_type () == ObjectType::ArrayObj
          && "parameters object is not an array");

  //   Object *ret = call_func (*nmod, fname,
  //                            static_cast<ArrayObject *> (fargs)->get_vals
  //                            ());

  Vec<Object *> &vals = static_cast<ArrayObject *> (fargs)->get_vals ();
  th->get_th () = std::thread (_run_cf_rt, th, std::ref (th->get_promise ()),
                               nmod, fname, std::ref (vals));

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API Object *
join_all (Module *mod)
{
  for (std::map<size_t, ThreadHandle *>::iterator::value_type &i : threadmap)
    {
      if (i.second == nullptr)
        continue;

      if (i.second->get_th ().joinable ())
        {
          i.second->get_th ().join ();
          delete i.second;
        }
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

  assert (OBJ_IS_INT (o_id) && "thread id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (o_id)->get_c ().get ())
          ->get_value ());

  if (!threadmap.count (id))
    {
      Object *ret = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (ret);
      return ret;
    }

  ThreadHandle *th = threadmap[id];
  delete th;

  threadmap.erase (id);

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}
} // namespace Thread
} // namespace native_mod
} // namespace sf

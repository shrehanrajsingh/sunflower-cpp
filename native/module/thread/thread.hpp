#pragma once

#include "../../../header.hpp"
#include "../../../module.hpp"

namespace sf
{
namespace native_mod
{
namespace Thread
{
class ThreadHandle
{
private:
  Object *fname;
  Object *fargs;
  Module *mod;
  bool has_result = false;
  Object *ret = nullptr;

  std::promise<Object *> promise;
  std::thread th_handle;
  std::future<Object *> future;

public:
  ThreadHandle (Object *_Fname, Object *_Fargs, Module *_Mod)
      : fname{ _Fname }, fargs{ _Fargs }, mod{ _Mod }
  {
    IR (fname);
    IR (fargs);
  }

  inline Object *&
  get_fname ()
  {
    return fname;
  }

  inline Object *&
  get_fargs ()
  {
    return fargs;
  }

  inline Module *&
  get_mod ()
  {
    return mod;
  }

  inline bool &
  get_has_result ()
  {
    return has_result;
  }

  void
  set_has_result (bool b)
  {
    get_has_result () = b;
  }

  inline Object *&
  get_ret ()
  {
    return ret;
  }

  inline std::thread &
  get_th ()
  {
    return th_handle;
  }

  inline std::promise<Object *> &
  get_promise ()
  {
    return promise;
  }

  inline std::future<Object *> &
  get_future ()
  {
    return future;
  }

  inline Object *
  get_return ()
  {
    if (get_ret () == nullptr)
      get_ret () = get_future ().get ();
    return get_ret ();
  }

  ~ThreadHandle ()
  {
    if (get_has_result ())
      {
        if (get_ret () != nullptr)
          {
            DR (get_ret ());
          }
      }

    DR (fname);
    DR (fargs);
  }
};

SF_API Object *create (Module *);
SF_API Object *join (Module *);
SF_API Object *run (Module *);
SF_API Object *detach (Module *);
SF_API Object *join_all (Module *);
SF_API Object *close (Module *);
} // namespace Thread
} // namespace native_mod
} // namespace sf

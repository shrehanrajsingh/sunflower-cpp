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
    return get_future ().get ();
  }

  ~ThreadHandle ()
  {
    DR (fname);
    DR (fargs);
  }
};

SF_API Object *create (Module *);
SF_API Object *join (Module *);
SF_API Object *run (Module *);
SF_API Object *detach (Module *);
SF_API Object *join_all (Module *);
} // namespace Thread
} // namespace native_mod
} // namespace sf

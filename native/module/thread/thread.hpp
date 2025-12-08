#pragma once

#include "../../../header.hpp"
#include "../../../module.hpp"

#define SF_MOD_THREAD_LIMIT (10000)

namespace sf
{
namespace native_mod
{
namespace Thread
{
static std::mutex th_mutex;
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
  bool is_deleted;

public:
  ThreadHandle (Object *_Fname, Object *_Fargs, Module *_Mod)
      : fname{ _Fname }, fargs{ _Fargs }, mod{ _Mod }, is_deleted{ false }
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

  inline bool &
  get_is_deleted ()
  {
    return is_deleted;
  }

  ~ThreadHandle ()
  {
    std::lock_guard<std::mutex> thd_lock (th_mutex);
    if (get_has_result ())
      {
        if (get_ret () != nullptr)
          {
            DR (get_ret ());
          }

        if (get_th ().joinable ())
          get_th ().join ();
      }
    else
      {
        if (get_th ().joinable ())
          get_th ().join ();

        if (get_ret () != nullptr)
          {
            DR (get_ret ());
          }
      }

    // std::cout << fname->get_ref_count () << '\t' << fargs->get_ref_count ()
    //           << '\n';
    // ArrayObject *a_fargs = static_cast<ArrayObject *> (fargs);

    // for (Object *&i : a_fargs->get_vals ())
    //   std::cout << "(" << i->get_ref_count () << ")" << '\n';

    DR (fname);
    DR (fargs);
  }
};

SF_API Object *create (Module *);
SF_API Object *join (Module *);
SF_API Object *run (Module *);
SF_API Object *join_all (Module *);
SF_API Object *close (Module *);

SF_API void __sf_thread_cleanup ();
} // namespace Thread
} // namespace native_mod
} // namespace sf

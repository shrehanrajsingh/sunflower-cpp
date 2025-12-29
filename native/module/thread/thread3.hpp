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
class ThreadHandle
{
private:
  Object *name = nullptr;
  Object *args = nullptr;
  Module *mod = nullptr;
  bool done = false;
  bool is_closed = false;
  size_t id = 0;

public:
  ThreadHandle (Object *_Name, Object *_Args, Module *_Mod)
      : name{ _Name }, args{ _Args }, mod{ _Mod }
  {
    IR (name);
    IR (args);
  }

  Module *&
  get_mod ()
  {
    return mod;
  }

  Object *&
  get_name ()
  {
    return name;
  }

  Object *&
  get_args ()
  {
    return args;
  }

  inline bool &
  get_done ()
  {
    return done;
  }

  inline size_t &
  get_id ()
  {
    return id;
  }

  inline bool &
  get_is_closed ()
  {
    return is_closed;
  }

  void run ();

  ~ThreadHandle ()
  {
    // DR (name);
    // DR (args);
  }
};

SF_API void init_runtime_threads ();

SF_API Object *create (Module *);
SF_API Object *join (Module *);
SF_API Object *run (Module *);
SF_API Object *join_all (Module *);
SF_API Object *close (Module *);

SF_API void __sf_thread_cleanup ();
} // namespace Thread
} // namespace native_mod
} // namespace sf

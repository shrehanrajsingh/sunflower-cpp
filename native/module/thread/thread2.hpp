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

struct ThreadTask
{
  Object *fname;
  Object *args;
  Module *mod;

  //   std::promise<Object *> res;
  bool done = false;
};

class ThreadHandle
{
private:
  ThreadTask *task = nullptr;
  //   std::future<Object *> fut;
  bool scheduled = false;
  size_t id;

public:
  ThreadHandle (Object *_Fname, Object *_Fargs, Module *_Mod);

  void run ();

  inline const size_t &
  get_id () const
  {
    return id;
  }

  ThreadTask *&
  get_task ()
  {
    return task;
  }

  Object *
  join ()
  {
    // return fut.get ();
    while (!task->done)
      ;
    return nullptr;
  }

  ~ThreadHandle ();
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

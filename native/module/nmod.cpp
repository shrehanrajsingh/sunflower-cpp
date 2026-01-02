#include "nmod.hpp"

namespace sf
{
namespace native_mod
{
static std::map<std::string, std::function<Module *(void)>> modmap;
static bool destroy_called = false;

SF_API ModMap_Type &
get_modmap ()
{
  return modmap;
}

SF_API Module *
get_mod (std::string s)
{
  if (!modmap.count (s))
    return nullptr;

  return modmap[s]();
}

SF_API void
add_mod (std::string s, ModMap_ValType v)
{
  modmap[s] = v;
}

SF_API void
nmod_init ()
{
  add_mod ("_Native_File", File::make);
  add_mod ("_Native_Thread", Thread::make);
  add_mod ("_Native_Socket", Socket::make);
}

SF_API void
nmod_destroy ()
{
  if (!destroy_called)
    {
      destroy_called = true;
      File::destroy ();
      Thread::__sf_thread_cleanup ();
    }
}

} // namespace native_mod
} // namespace sf

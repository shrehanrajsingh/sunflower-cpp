#include "environment.hpp"

namespace sf
{
void
Environment::add_path (Str s)
{
  sys_paths.push_back (s);
}

void
Environment::add_arg (Str s)
{
  args.push_back (s);
}
} // namespace sf

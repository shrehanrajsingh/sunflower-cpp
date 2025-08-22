#pragma once

#include "header.hpp"
#include "str.hpp"
#include "vec.hpp"

namespace sf
{
class Environment
{
private:
  Vec<Str> sys_paths;
  Vec<Str> args;

public:
  Environment () {}

  void add_path (Str);
  void add_arg (Str);

  inline Vec<Str> &
  get_syspaths ()
  {
    return sys_paths;
  }

  inline Vec<Str> &
  get_args ()
  {
    return args;
  }

  ~Environment () {}
};
} // namespace sf

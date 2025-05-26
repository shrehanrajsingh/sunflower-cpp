/**
 * Standard output base classes for Sunflower objects
 */

#pragma once

#include "header.hpp"
#include "str.hpp"

namespace sf
{
class StdoutRepr
{
private:
  bool supports_stdout = false;

public:
  StdoutRepr () : supports_stdout (true) {}
  StdoutRepr (bool _SSTD) : supports_stdout (_SSTD) {}

  virtual std::string
  get_stdout_repr ()
  {
    return "<class internal::StdoutRepr>";
  }

  /**
   * Often when we print a string we only
   * desire the content of the string, and
   * not the quotes.
   * But, say, in a array, we desire the quotes
   * as well, so when we call get_stdout_repr
   * on an array we will, internally, call get_stdout_repr_in_container
   * on all the values
   */
  virtual std::string
  get_stdout_repr_in_container ()
  {
    return get_stdout_repr ();
  }

  ~StdoutRepr () {}
};
} // namespace sf

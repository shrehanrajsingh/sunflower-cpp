#include "native.hpp"

namespace sf
{
namespace native
{
void
add_natives (Vec<Statement *> &m)
{
  _add_native_write (m);
  _add_native_putln (m);
  _add_native_put (m);

  /* string methods */
  _add_native_replace (m);
}
} // namespace native
} // namespace sf

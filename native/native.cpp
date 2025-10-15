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
  _add_native_input (m);

  /* string methods */
  _add_native_replace (m);
  _add_native_find (m);
  _add_native_count (m);

  /* integer methods */
  _add_native_int_base (m);

  /* list methods */
  _add_native_list_push (m);
}
} // namespace native
} // namespace sf

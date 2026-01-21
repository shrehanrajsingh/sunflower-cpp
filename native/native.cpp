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
  _add_native_sleep (m);
  _add_native_len (m);
  _add_native_exit (m);

  /* string methods */
  _add_native_replace (m);
  _add_native_find (m);
  _add_native_count (m);
  _add_native_strip (m);
  _add_native_split (m);

  /* integer methods */
  _add_native_int_base (m);
  _add_native_int_to_string (m);

  /* list methods */
  _add_native_list_push (m);
  _add_native_list_pop (m);

  /* dict methods */
  _add_native_dict_to_string (m);
}
} // namespace native
} // namespace sf

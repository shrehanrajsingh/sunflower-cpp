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

  /* string methods */
  _add_native_replace (m);
  _add_native_find (m);
  _add_native_count (m);

  /* integer methods */
  _add_native_int_base (m);
  _add_native_int_to_string (m);

  /* list methods */
  _add_native_list_push (m);
  _add_native_list_pop (m);
  _add_native_list_clear (m);
  _add_native_list_copy (m);
  _add_native_list_count (m);
  _add_native_list_extend (m);
  _add_native_list_index (m);
  _add_native_list_insert (m);
  _add_native_list_remove (m);
  _add_native_list_reverse (m);
  _add_native_list_sort (m);
}
} // namespace native
} // namespace sf

#include "native.hpp"
#include "string/find.hpp"
#include "string/index.hpp"
#include "string/isalnum.hpp"
#include "string/isalpha.hpp"
#include "string/isascii.hpp"
#include "string/isdecimal.hpp"
#include "string/isdigit.hpp"
#include "string/isidentifier.hpp"
#include "string/islower.hpp"
#include "string/isnumeric.hpp"
#include "string/isprintable.hpp"
#include "string/isupper.hpp"
#include "string/lower.hpp"
#include "string/upper.hpp"

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

  _add_native_islower (m);
  _add_native_isupper (m);
  _add_native_isdigit (m);
  _add_native_isalpha (m);
  _add_native_isalnum (m);
  _add_native_replace (m);
  _add_native_find (m);
  _add_native_count (m);
  _add_native_lower (m);
  _add_native_upper (m);
  _add_native_isascii (m);
  _add_native_isprintable (m);
  _add_native_index (m);
  _add_native_isdecimal (m);
  _add_native_isnumeric (m);
  _add_native_isidentifier (m);

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

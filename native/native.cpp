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
}
} // namespace native
} // namespace sf

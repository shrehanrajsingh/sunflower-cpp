#include "const.hpp"
#include "expr.hpp"

namespace sf
{
void
AmbigConstant::print ()
{
  std::cout << "ambig\nval: ";
  if (val != nullptr)
    val->print ();
  else
    std::cout << "nullptr";
}
} // namespace sf

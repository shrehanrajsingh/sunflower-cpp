#include "func.hpp"

namespace sf
{
void
_sffunc_refcheck (Function *&f)
{
  if (f->get_ref_count () < 1)
    {
      delete f;
      f = nullptr;
    }
}
} // namespace sf

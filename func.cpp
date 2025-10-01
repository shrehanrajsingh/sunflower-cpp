#include "func.hpp"
#include "expr.hpp"
#include "stmt.hpp"

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

CodedFunction::~CodedFunction ()
{
  for (Statement *&i : body)
    delete i;

  for (Expr *&i : args)
    delete i;
}
} // namespace sf

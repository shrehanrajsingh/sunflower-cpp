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
  /**
   * CodedFunction takes a reference
   * to both body and args from FuncDeclStatement
   * So you cannot delete the pointers
   * in body and args in both the places
   */
  // for (Statement *&i : body)
  //   delete i;

  // for (Expr *&i : args)
  //   delete i;
}
} // namespace sf

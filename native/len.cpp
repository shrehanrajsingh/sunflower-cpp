#include "len.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_len (Module *m)
{
  Object *a_obj = m->get_variable ("a");

  int l = 0;
  switch (a_obj->get_type ())
    {
    case ObjectType::ArrayObj:
      {
        l = static_cast<ArrayObject *> (a_obj)->get_vals ().get_size ();
      }
      break;
    case ObjectType::Constant:
      {
        ConstantObject *c = static_cast<ConstantObject *> (a_obj);
        Constant *cc = c->get_c ().get ();

        switch (cc->get_type ())
          {
          case ConstantType::String:
            {
              l = static_cast<StringConstant *> (cc)->get_value ().size ();
            }
            break;

          default:
            break;
          }
      }
      break;
    case ObjectType::DictObj:
      {
        l = static_cast<DictObject *> (a_obj)->get_vals ().size ();
      }
      break;

    default:
      break;
    }

  Object *res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new IntegerConstant (l))));
  IR (res);

  return res;
}

SF_API void
_add_native_len (Vec<Statement *> &ast)
{
  NativeFunction *nv_len = new NativeFunction (_native_len, { "a" });

  /**
   * When testing for memory leaks
   * by looping mod_exec routine
   * infinite times, we preserve
   * the Function * in AST by increasing
   * its ref_count by 1 exactly so that
   * after each mod_exec the reference count
   * falls back to 1 and we do not free the object.
   * Freeing the object would result in SEG_FAULT in the
   * next iteration because the Function* is used in AST
   * (see the next statement)
   * and we would be storing a nullptr.
   * NOTE: In actual definition of I(X), I recommend
   * not using I(X) anywhere (use IR(X) instead), however
   * it is absolutely fine to use I(X) in this context since we are
   * using it with Function* and not Object*
   */
  I (nv_len);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("len")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_len))))));

  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("input")),
  //                    static_cast<Expr *> (new ConstantExpr (
  //                        static_cast<Constant *> (new NoneConstant ()))))));
}
} // namespace native

} // namespace sf

#include "exit.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_exit (Module *m)
{
  Object *a_obj = m->get_variable ("a");

  assert (OBJ_IS_INT (a_obj) && "exit code must be an integer");

  int code = static_cast<IntegerConstant *> (
                 static_cast<ConstantObject *> (a_obj)->get_c ().get ())
                 ->get_value ();

  ::_Exit (code);

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_exit (Vec<Statement *> &ast)
{
  NativeFunction *nv_exit = new NativeFunction (_native_exit, { "a" });

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
  I (nv_exit);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("exit")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_exit))))));

  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("exit")),
  //                    static_cast<Expr *> (new ConstantExpr (
  //                        static_cast<Constant *> (new NoneConstant ()))))));
}
} // namespace native

} // namespace sf

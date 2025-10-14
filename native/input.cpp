#include "input.hpp"
#include "put.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_input (Module *m)
{
  Object *a_obj = m->get_variable ("a");

  Object *npr = _native_put (m);
  DR (npr); /* write (input_message) */

  std::string s;
  std::getline (std::cin, s);

  Object *ret = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new StringConstant (s.c_str ()))));

  IR (ret);
  return ret;
}

SF_API void
_add_native_input (Vec<Statement *> &ast)
{
  NativeFunction *nv_input = new NativeFunction (_native_input, { "a" });

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
  I (nv_input);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("input")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_input))))));

  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("input")),
  //                    static_cast<Expr *> (new ConstantExpr (
  //                        static_cast<Constant *> (new NoneConstant ()))))));
}
} // namespace native

} // namespace sf

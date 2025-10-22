#include "write.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_sleep (Module *m)
{
  Object *a_obj = m->get_variable ("a");

  assert (OBJ_IS_INT (a_obj) && "time in ms must be an integer");

  int v = static_cast<IntegerConstant *> (
              static_cast<ConstantObject *> (a_obj)->get_c ().get ())
              ->get_value ();

  std::this_thread::sleep_for (std::chrono::milliseconds (v));

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (r);
  //   std::cout << r->get_ref_count () << '\n';
  return r;
}

SF_API void
_add_native_sleep (Vec<Statement *> &ast)
{
  NativeFunction *nv_sleep = new NativeFunction (_native_sleep, { "a" });

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
  I (nv_sleep);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("sleep")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_sleep))))));

  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("sleep")),
  //                    static_cast<Expr *> (new ConstantExpr (
  //                        static_cast<Constant *> (new NoneConstant ()))))));
}
} // namespace native

} // namespace sf

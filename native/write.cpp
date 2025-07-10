#include "write.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_write (Module *m)
{
  sf::Object *a;
  TC (a = m->get_variable ("a"));

  sf::ArrayObject *ao = static_cast<sf::ArrayObject *> (a);

  for (size_t i = 0; i < ao->get_vals ().get_size (); i++)
    {
      std::cout << ao->get_vals ()[i]->get_stdout_repr ();

      if (i != ao->get_vals ().get_size () - 1)
        std::cout << ' ';
      else
        std::cout << std::endl;
    }

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (r);
  //   std::cout << r->get_ref_count () << '\n';
  return r;
}

SF_API void
_add_native_write (Vec<Statement *> &ast)
{
  NativeFunction *nv_write = new NativeFunction (_native_write, { "a" });
  nv_write->set_va_args (true); /* any number of arguments */

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
  I (nv_write);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("write")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_write))))));
}
} // namespace native

} // namespace sf

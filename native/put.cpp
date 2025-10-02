#include "putln.hpp"

namespace sf
{
namespace native
{
SF_API Object *
_native_put (Module *m)
{
  sf::Object *a;
  TC (a = m->get_variable ("a"));

  // std::cout << "Inside native_putln!\n";
  std::cout << a->get_stdout_repr ();

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (r);
  //   std::cout << r->get_ref_count () << '\n';
  return r;
}

SF_API void
_add_native_put (Vec<Statement *> &ast)
{
  NativeFunction *nv_put = new NativeFunction (_native_put, { "a" });
  I (nv_put);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("put")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_put))))));
}
} // namespace native
} // namespace sf

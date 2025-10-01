#include "base.hpp"

namespace sf
{
SF_API Object *
_native_int_base (Module *mod)
{
  /* replace a with b in self */
  Object *self = mod->get_variable ("self");
  Object *a = mod->get_variable ("a");

  assert (OBJ_IS_INT (self) && "self must be an integer");
  assert (OBJ_IS_INT (a) && "a must be an integer");

  int self_val = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (self)->get_c ().get ())
                     ->get_value ();

  int a_val = static_cast<IntegerConstant *> (
                  static_cast<ConstantObject *> (a)->get_c ().get ())
                  ->get_value ();

  int base_conv = 0;
  int p = 0;
  while (self_val)
    {
      int a = 1;
      int pp = p++;
      while (pp--)
        a *= 10;

      base_conv += (self_val % a_val) * a;
      self_val /= a_val;
    }

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::IntegerConstant (base_conv))));

  IR (r);
  return r;
}

SF_API void
_add_native_int_base (Vec<Statement *> &ast)
{
  NativeFunction *nv_base
      = new NativeFunction (_native_int_base, { "self", "a" });
  nv_base->set_self_arg (true);

  // I (nv_base);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("0.base")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_base))))));
}
} // namespace sf

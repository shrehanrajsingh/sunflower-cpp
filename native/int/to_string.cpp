#include "base.hpp"

namespace sf
{
SF_API Object *
_native_int_to_string (Module *mod)
{
  /* replace a with b in self */
  Object *self = mod->get_variable ("self");

  assert (OBJ_IS_INT (self) && "self must be an integer");

  int self_val = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (self)->get_c ().get ())
                     ->get_value ();

  std::string s = "";
  while (self_val)
    {
      char p = static_cast<char> (self_val % 10) + '0';
      s = std::string{ p } + s;
      self_val /= 10;
    }

  sf::Object *r = static_cast<sf::Object *> (
      new sf::ConstantObject (static_cast<sf::Constant *> (
          new sf::StringConstant (Str{ s.c_str () }))));

  IR (r);
  return r;
}

SF_API void
_add_native_int_to_string (Vec<Statement *> &ast)
{
  NativeFunction *nv_to_string
      = new NativeFunction (_native_int_to_string, { "self" });
  nv_to_string->set_self_arg (true);

  I (nv_to_string);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("0.to_string")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_to_string))))));
}
} // namespace sf

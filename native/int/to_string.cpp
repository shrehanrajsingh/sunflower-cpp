#include "base.hpp"

namespace sf
{
SF_API Object *
_native_int_to_string (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (OBJ_IS_INT (self) && "self must be an integer");

  ConstantObject *cobj = static_cast<ConstantObject *> (self);
  IntegerConstant *iconst
      = static_cast<IntegerConstant *> (cobj->get_c ().get ());

  // cobj->print ();

  int value = iconst->get_value ();

  if (value == 0)
    {
      auto *r = static_cast<sf::Object *> (new sf::ConstantObject (
          static_cast<sf::Constant *> (new sf::StringConstant (Str{ "0" }))));
      IR (r);
      return r;
    }

  bool neg = value < 0;

  int64_t v = value;
  if (v < 0)
    v = -v;

  std::string s;
  while (v > 0)
    {
      s.push_back (char ('0' + (v % 10)));
      v /= 10;
    }

  if (neg)
    s.push_back ('-');

  std::reverse (s.begin (), s.end ());

  auto *r = static_cast<sf::Object *> (
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

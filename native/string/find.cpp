#include "replace.hpp"

namespace sf
{
SF_API Object *
_native_find (Module *mod)
{
  /* replace a with b in self */
  Object *self = mod->get_variable ("self");
  Object *a = mod->get_variable ("a");

  assert (OBJ_IS_STR (self) && "self must be a string");
  assert (OBJ_IS_STR (a) && "a must be a string");

  Str &self_val = static_cast<StringConstant *> (
                      static_cast<ConstantObject *> (self)->get_c ().get ())
                      ->get_value ();

  Str &a_val = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (a)->get_c ().get ())
                   ->get_value ();

  int idx = self_val.find (a_val);

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::IntegerConstant (idx))));

  IR (r);
  return r;
}

SF_API void
_add_native_find (Vec<Statement *> &ast)
{
  NativeFunction *nv_find = new NativeFunction (_native_find, { "self", "a" });
  nv_find->set_self_arg (true);

  I (nv_find);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.find")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_find))))));
}
} // namespace sf

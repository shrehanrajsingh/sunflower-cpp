#include "replace.hpp"

namespace sf
{
SF_API Object *
_native_replace (Module *mod)
{
  /* replace a with b in self */
  Object *self = mod->get_variable ("self");
  Object *a = mod->get_variable ("a");
  Object *b = mod->get_variable ("b");

  assert (OBJ_IS_STR (self) && "self must be a string");
  assert (OBJ_IS_STR (a) && "a must be a string");
  assert (OBJ_IS_STR (b) && "b must be a string");

  Str &self_val = static_cast<StringConstant *> (
                      static_cast<ConstantObject *> (self)->get_c ().get ())
                      ->get_value ();

  Str &a_val = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (a)->get_c ().get ())
                   ->get_value ();

  Str &b_val = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (b)->get_c ().get ())
                   ->get_value ();

  self_val.replace (a_val, b_val);

  // sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
  //     static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (self);
  return self;
}

SF_API void
_add_native_replace (Vec<Statement *> &ast)
{
  NativeFunction *nv_replace
      = new NativeFunction (_native_replace, { "self", "a", "b" });
  nv_replace->set_self_arg (true);

  I (nv_replace);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.replace")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_replace))))));
}
} // namespace sf

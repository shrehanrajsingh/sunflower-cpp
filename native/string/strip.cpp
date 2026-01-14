#include "strip.hpp"

namespace sf
{
SF_API Object *
_native_strip (Module *mod)
{
  Object *self = mod->get_variable ("self");

  assert (OBJ_IS_STR (self) && "self must be a string");

  Str &self_val = static_cast<StringConstant *> (
                      static_cast<ConstantObject *> (self)->get_c ().get ())
                      ->get_value ();

  std::string self_v = self_val.to_std_string ();

  size_t i_f = 0;
  while (i_f < self_v.size () && isspace (self_v[i_f]))
    i_f++;

  int i_b = self_v.size () - 1;
  while (i_b > -1 && isspace (self_v[i_b]))
    i_b--;

  self_v = self_v.substr (i_f, i_b - i_f + 1);

  Object *r = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new StringConstant (self_v.c_str ()))));

  IR (r);
  return r;
}

SF_API void
_add_native_strip (Vec<Statement *> &ast)
{
  NativeFunction *nv_strip = new NativeFunction (_native_strip, { "self" });
  nv_strip->set_self_arg (true);

  I (nv_strip);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.strip")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_strip))))));
}
} // namespace sf

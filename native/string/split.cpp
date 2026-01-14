#include "split.hpp"

namespace sf
{
SF_API Object *
_native_split (Module *mod)
{
  Object *self = mod->get_variable ("self");
  Object *a = mod->get_variable ("a");

  assert (OBJ_IS_STR (self) && "self is not a string");
  assert (OBJ_IS_STR (a) && "a is not a string");

  Str &self_val = static_cast<StringConstant *> (
                      static_cast<ConstantObject *> (self)->get_c ().get ())
                      ->get_value ();

  Str &a_val = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (a)->get_c ().get ())
                   ->get_value ();

  std::string self_v = self_val.to_std_string ();
  std::string a_v = a_val.to_std_string ();

  std::vector<std::string> toks;
  size_t start = 0, end;

  while ((end = self_v.find (a_v, start)) != std::string::npos)
    {
      toks.push_back (self_v.substr (start, end - start));
      start = end + a_v.length ();
    }

  toks.push_back (self_v.substr (start));

  Vec<Object *> ao_vals;
  for (std::string &v : toks)
    {
      Object *ov = static_cast<Object *> (new ConstantObject (
          static_cast<Constant *> (new StringConstant (v.c_str ()))));

      IR (ov);
      ao_vals.push_back (ov);
    }

  ArrayObject *ao = new ArrayObject (ao_vals);

  Object *r = static_cast<Object *> (ao);
  IR (r);

  return r;
}

SF_API void
_add_native_split (Vec<Statement *> &ast)
{
  NativeFunction *nv_split
      = new NativeFunction (_native_split, { "self", "a" });
  nv_split->set_self_arg (true);

  I (nv_split);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.split")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_split))))));
}
} // namespace sf

#include "lower.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_lower (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (OBJ_IS_STR (self) && "self must be a string");

  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  // Nayi string banayein
  std::string result_str = "";
  for (size_t i = 0; i < val.size (); ++i)
    {
      result_str += static_cast<char> (
          std::tolower (static_cast<unsigned char> (val[i])));
    }

  // Naya StringConstant return karein
  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new StringConstant (result_str))));
}

SF_API void
_add_native_lower (Vec<Statement *> &ast)
{
  NativeFunction *nv = new NativeFunction (_native_lower, { "self" });
  nv->set_self_arg (true);
  I (nv);
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.lower")),
                     static_cast<Expr *> (
                         new FunctionExpr (static_cast<Function *> (nv))))));
}
} // namespace sf
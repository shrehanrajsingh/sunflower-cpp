#include "isascii.hpp"

namespace sf
{
SF_API Object *
_native_isascii (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (OBJ_IS_STR (self) && "self must be a string");
  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  bool result = true; // Empty string is technically ASCII
  for (size_t i = 0; i < val.size (); ++i)
    {
      if (static_cast<unsigned char> (val[i]) > 127)
        {
          result = false;
          break;
        }
    }
  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new BooleanConstant (result))));
}

SF_API void
_add_native_isascii (Vec<Statement *> &ast)
{
  NativeFunction *nv = new NativeFunction (_native_isascii, { "self" });
  nv->set_self_arg (true);
  I (nv);
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.isascii")),
                     static_cast<Expr *> (
                         new FunctionExpr (static_cast<Function *> (nv))))));
}
}
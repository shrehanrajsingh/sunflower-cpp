#include "isalpha.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_isalpha (Module *mod)
{
  Object *self = mod->get_variable ("self");

  assert (OBJ_IS_STR (self) && "self must be a string");

  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  bool result = (val.size () > 0);
  for (size_t i = 0; i < val.size (); ++i)
    {
      if (!std::isalpha (static_cast<unsigned char> (val[i])))
        {
          result = false;
          break;
        }
    }

  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new BooleanConstant (result))));
}

SF_API void
_add_native_isalpha (Vec<Statement *> &ast)
{
  NativeFunction *nv_isalpha
      = new NativeFunction (_native_isalpha, { "self" });
  nv_isalpha->set_self_arg (true);

  I (nv_isalpha);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.isalpha")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_isalpha))))));
}
} // namespace sf
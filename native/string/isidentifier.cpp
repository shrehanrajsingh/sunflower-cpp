#include "isidentifier.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_isidentifier (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (OBJ_IS_STR (self) && "self must be a string");
  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  if (val.size () == 0)
    return static_cast<Object *> (new ConstantObject (
        static_cast<Constant *> (new BooleanConstant (false))));

  // Pehla character letter ya underscore hona chahiye
  bool result
      = (std::isalpha (static_cast<unsigned char> (val[0])) || val[0] == '_');

  if (result)
    {
      for (size_t i = 1; i < val.size (); ++i)
        {
          // Baki characters alphanumeric ya underscore ho sakte hain
          if (!std::isalnum (static_cast<unsigned char> (val[i]))
              && val[i] != '_')
            {
              result = false;
              break;
            }
        }
    }
  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new BooleanConstant (result))));
}

SF_API void
_add_native_isidentifier (Vec<Statement *> &ast)
{
  NativeFunction *nv = new NativeFunction (_native_isidentifier, { "self" });
  nv->set_self_arg (true);
  I (nv);
  ast.insert (0,
              static_cast<Statement *> (new VarDeclStatement (
                  static_cast<Expr *> (new VariableExpr ("''.isidentifier")),
                  static_cast<Expr *> (
                      new FunctionExpr (static_cast<Function *> (nv))))));
}
}
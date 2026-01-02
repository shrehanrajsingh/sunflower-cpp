#include "isdigit.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_isdigit (Module *mod)
{
  Object *self = mod->get_variable ("self");

  assert (OBJ_IS_STR (self) && "self must be a string");

  // Get the custom sf::Str object using the required static_casts
  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  // Alphanumeric logic: Check size and use a standard for loop
  bool result = (val.size () > 0);
  for (size_t i = 0; i < val.size (); ++i)
    {
      if (!std::isdigit (static_cast<unsigned char> (val[i])))
        {
          result = false;
          break;
        }
    }

  // Return a Boolean constant object
  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new BooleanConstant (result))));
}

SF_API void
_add_native_isdigit (Vec<Statement *> &ast)
{
  NativeFunction *nv_isdigit
      = new NativeFunction (_native_isdigit, { "self" });
  nv_isdigit->set_self_arg (true);

  I (nv_isdigit);

  // Registration using the AST insert pattern from replace.cpp
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.isdigit")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_isdigit))))));
}
} // namespace sf
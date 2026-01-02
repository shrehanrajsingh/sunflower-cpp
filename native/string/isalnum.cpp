#include "isalnum.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_isalnum (Module *mod)
{
  Object *self = mod->get_variable ("self");

  assert (OBJ_IS_STR (self) && "self must be a string");

  // Get the custom sf::Str object
  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  // FIX: sf::Str uses .size() and indexing [] instead of empty() and iterators
  bool result = true; // Initialize to true; empty string should return true
  for (size_t i = 0; i < val.size (); ++i)
    {
      if (!std::isalnum (static_cast<unsigned char> (val[i])))
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
_add_native_isalnum (Vec<Statement *> &ast)
{
  // FIX: Using the exact registration pattern from replace.cpp
  NativeFunction *nv_isalnum
      = new NativeFunction (_native_isalnum, { "self" });
  nv_isalnum->set_self_arg (true);

  I (nv_isalnum);

  // This fixes the "expected a type specifier" error by using VarDeclStatement
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.isalnum")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_isalnum))))));
}
} // namespace sf
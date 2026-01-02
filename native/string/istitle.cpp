#include "istitle.hpp"
#include <cctype>

namespace sf
{
SF_API Object *
_native_istitle (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (OBJ_IS_STR (self) && "self must be a string");

  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();

  bool result = (val.size () > 0);
  bool cased = false;
  bool previous_is_cased = false;

  for (size_t i = 0; i < val.size (); ++i)
    {
      unsigned char c = static_cast<unsigned char> (val[i]);
      if (std::isupper (c))
        {
          if (previous_is_cased)
            {
              result = false;
              break;
            }
          previous_is_cased = true;
          cased = true;
        }
      else if (std::islower (c))
        {
          if (!previous_is_cased)
            {
              result = false;
              break;
            }
          previous_is_cased = true;
          cased = true;
        }
      else
        {
          previous_is_cased = false;
        }
    }

  return static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new BooleanConstant (result && cased))));
}

SF_API void
_add_native_istitle (Vec<Statement *> &ast)
{
  NativeFunction *nv = new NativeFunction (_native_istitle, { "self" });
  nv->set_self_arg (true);
  I (nv);
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.istitle")),
                     static_cast<Expr *> (
                         new FunctionExpr (static_cast<Function *> (nv))))));
}
} // namespace sf
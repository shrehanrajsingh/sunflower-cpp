#include "copy.hpp"

namespace sf
{
SF_API Object *
_native_list_copy (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  Vec<Object *> new_vals;
  for (Object *obj : vals)
    {
      IR (obj);
      new_vals.push_back (obj);
    }

  ArrayObject *new_ao = new ArrayObject (std::move (new_vals));
  Object *ret = static_cast<Object *> (new_ao);

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_copy (Vec<Statement *> &ast)
{
  NativeFunction *nv_copy
      = new NativeFunction (_native_list_copy, { "self" });
  nv_copy->set_self_arg (true);

  I (nv_copy);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].copy")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_copy))))));
}
} // namespace sf

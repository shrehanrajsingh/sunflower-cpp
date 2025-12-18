#include "remove.hpp"

namespace sf
{
SF_API Object *
_native_list_remove (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");
  Object *value_obj = mod->get_variable ("value");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  for (size_t i = 0; i < vals.get_size (); ++i)
    {
      // Simple equality check; in a real implementation, you'd use a proper comparison
      if (vals[i] == value_obj)
        {
          DR (vals[i]);
          vals.remove (i);
          Object *ret = static_cast<Object *> (
              new ConstantObject (static_cast<Constant *> (new NoneConstant ())));
          IR (ret);
          return ret;
        }
    }

  // Value not found, raise an error (for simplicity, return None)
  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));
  IR (ret);
  return ret;
}

SF_API void
_add_native_list_remove (Vec<Statement *> &ast)
{
  NativeFunction *nv_remove
      = new NativeFunction (_native_list_remove, { "self", "value" });
  nv_remove->set_self_arg (true);

  I (nv_remove);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].remove")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_remove))))));
}
} // namespace sf

#include "index.hpp"

namespace sf
{
SF_API Object *
_native_list_index (Module *mod)
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
          Object *ret = static_cast<Object *> (
              new ConstantObject (static_cast<Constant *> (new IntegerConstant (i))));
          IR (ret);
          return ret;
        }
    }

  // Value not found, raise an error (for simplicity, return -1)
  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new IntegerConstant (-1))));
  IR (ret);
  return ret;
}

SF_API void
_add_native_list_index (Vec<Statement *> &ast)
{
  NativeFunction *nv_index
      = new NativeFunction (_native_list_index, { "self", "value" });
  nv_index->set_self_arg (true);

  I (nv_index);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].index")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_index))))));
}
} // namespace sf

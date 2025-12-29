#include "count.hpp"

namespace sf
{
SF_API Object *
_native_list_count (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");
  Object *value_obj = mod->get_variable ("value");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  int count = 0;
  for (Object *obj : vals)
    {
      // Simple equality check; in a real implementation, you'd use a proper comparison
      if (obj == value_obj)
        count++;
    }

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new IntegerConstant (count))));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_count (Vec<Statement *> &ast)
{
  NativeFunction *nv_count
      = new NativeFunction (_native_list_count, { "self", "value" });
  nv_count->set_self_arg (true);

  I (nv_count);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].count")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_count))))));
}
} // namespace sf

#include "extend.hpp"

namespace sf
{
SF_API Object *
_native_list_extend (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");
  Object *iterable_obj = mod->get_variable ("iterable");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");
  assert (iterable_obj->get_type () == ObjectType::ArrayObj
          && "iterable is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  ArrayObject *iterable_ao = static_cast<ArrayObject *> (iterable_obj);
  Vec<Object *> &vals = ao->get_vals ();
  Vec<Object *> &iterable_vals = iterable_ao->get_vals ();

  for (Object *obj : iterable_vals)
    {
      IR (obj);
      vals.push_back (obj);
    }

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_extend (Vec<Statement *> &ast)
{
  NativeFunction *nv_extend
      = new NativeFunction (_native_list_extend, { "self", "iterable" });
  nv_extend->set_self_arg (true);

  I (nv_extend);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].extend")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_extend))))));
}
} // namespace sf

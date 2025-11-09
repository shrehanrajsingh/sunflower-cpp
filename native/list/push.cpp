#include "push.hpp"

namespace sf
{
SF_API Object *
_native_list_push (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");
  Object *a_obj = mod->get_variable ("a");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  ao->get_vals ().push_back (a_obj);
  IR (a_obj);

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_push (Vec<Statement *> &ast)
{
  NativeFunction *nv_push
      = new NativeFunction (_native_list_push, { "self", "a" });
  nv_push->set_self_arg (true);

  I (nv_push);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].push")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_push))))));
}
} // namespace sf
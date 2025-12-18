#include "clear.hpp"

namespace sf
{
SF_API Object *
_native_list_clear (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  ao->get_vals ().clear ();

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_clear (Vec<Statement *> &ast)
{
  NativeFunction *nv_clear
      = new NativeFunction (_native_list_clear, { "self" });
  nv_clear->set_self_arg (true);

  I (nv_clear);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].clear")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_clear))))));
}
} // namespace sf

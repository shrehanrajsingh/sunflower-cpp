#include "reverse.hpp"

namespace sf
{
SF_API Object *
_native_list_reverse (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  vals.reverse ();

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_reverse (Vec<Statement *> &ast)
{
  NativeFunction *nv_reverse
      = new NativeFunction (_native_list_reverse, { "self" });
  nv_reverse->set_self_arg (true);

  I (nv_reverse);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].reverse")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_reverse))))));
}
} // namespace sf

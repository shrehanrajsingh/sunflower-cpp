#include "insert.hpp"

namespace sf
{
SF_API Object *
_native_list_insert (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");
  Object *index_obj = mod->get_variable ("index");
  Object *value_obj = mod->get_variable ("value");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");
  assert (OBJ_IS_INT (index_obj) && "index must be an integer");

  int idx = static_cast<IntegerConstant *> (
                static_cast<ConstantObject *> (index_obj)->get_c ().get ())
                ->get_value ();

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  if (idx < 0)
    idx = 0;
  if (idx > (int)vals.get_size ())
    idx = vals.get_size ();

  IR (value_obj);
  vals.insert (idx, value_obj);

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_insert (Vec<Statement *> &ast)
{
  NativeFunction *nv_insert
      = new NativeFunction (_native_list_insert, { "self", "index", "value" });
  nv_insert->set_self_arg (true);

  I (nv_insert);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].insert")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_insert))))));
}
} // namespace sf

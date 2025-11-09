#include "pop.hpp"

namespace sf
{
SF_API Object *
_native_list_pop (Module *mod)
{
  Object *self = mod->get_variable ("self");
  Object *idx = mod->get_variable ("idx");

  assert (self->get_type () == ObjectType::ArrayObj && "self is not an array");
  assert (OBJ_IS_INT (idx) && "Index must be an integer");

  int v_idx = static_cast<IntegerConstant *> (
                  static_cast<ConstantObject *> (idx)->get_c ().get ())
                  ->get_value ();

  ArrayObject *ao = static_cast<ArrayObject *> (self);
  Vec<Object *> &ao_obj = ao->get_vals ();

  Object *p;

  if (v_idx == -1)
    p = ao_obj.pop_back ();
  else
    {
    }

  /**
   * Data removed from array
   * DR
   * Data to be returned from function
   * IR
   * Net: 0
   */

  IR (p);
  return p;
}

SF_API void
_add_native_list_pop (Vec<Statement *> &ast)
{
  NativeFunction *nv_pop
      = new NativeFunction (_native_list_pop, { "self", "idx" });
  nv_pop->set_self_arg (true);

  I (nv_pop);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].pop")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_pop))))));
}
} // namespace sf
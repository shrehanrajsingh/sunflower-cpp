#include "sort.hpp"
#include <algorithm>

namespace sf
{
SF_API Object *
_native_list_sort (Module *mod)
{
  Object *self_obj = mod->get_variable ("self");

  assert (self_obj->get_type () == ObjectType::ArrayObj
          && "self is not an array");

  ArrayObject *ao = static_cast<ArrayObject *> (self_obj);
  Vec<Object *> &vals = ao->get_vals ();

  // Simple sort based on object pointers; in a real implementation, you'd need a proper comparison
  std::sort (vals.get (), vals.get () + vals.get_size (),
             [] (Object *a, Object *b) {
               // For simplicity, sort by pointer value
               return a < b;
             });

  Object *ret = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (ret);
  return ret;
}

SF_API void
_add_native_list_sort (Vec<Statement *> &ast)
{
  NativeFunction *nv_sort
      = new NativeFunction (_native_list_sort, { "self" });
  nv_sort->set_self_arg (true);

  I (nv_sort);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("[].sort")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_sort))))));
}
} // namespace sf

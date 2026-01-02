#include "index.hpp"

namespace sf
{
SF_API Object *
_native_index (Module *mod)
{
  Object *self = mod->get_variable ("self");
  Object *sub_obj = mod->get_variable ("sub");
  assert (OBJ_IS_STR (self) && "self must be a string");

  Str &val = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (self)->get_c ().get ())
                 ->get_value ();
  Str sub = static_cast<StringConstant *> (
                static_cast<ConstantObject *> (sub_obj)->get_c ().get ())
                ->get_value ();

  size_t pos = val.find (sub);
  long result = (pos == std::string::npos) ? -1 : static_cast<long> (pos);

  return static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new IntConstant (result))));
}

SF_API void
_add_native_index (Vec<Statement *> &ast)
{
  NativeFunction *nv = new NativeFunction (_native_index, { "self", "sub" });
  nv->set_self_arg (true);
  I (nv);
  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("''.index")),
                     static_cast<Expr *> (
                         new FunctionExpr (static_cast<Function *> (nv))))));
}
}
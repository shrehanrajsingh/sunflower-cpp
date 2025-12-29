#include "to_string.hpp"

namespace sf
{
SF_API Object *
_native_dict_to_string (Module *mod)
{
  Object *self = mod->get_variable ("self");
  assert (self->get_type () == ObjectType::DictObj
          && "self must be an integer");

  DictObject *dobj = static_cast<DictObject *> (self);
  //   for (auto &&i : dobj->get_vals ())
  //     {
  //       //   std::cout << i.first;
  //       i.second->print ();
  //       std::cout << '\n';
  //     }

  std::string r = dobj->get_stdout_repr ();

  Object *ret = new ConstantObject (new StringConstant (r.c_str ()));
  IR (ret);

  return ret;
}

SF_API void
_add_native_dict_to_string (Vec<Statement *> &ast)
{
  NativeFunction *nv_to_string
      = new NativeFunction (_native_dict_to_string, { "self" });
  nv_to_string->set_self_arg (true);

  I (nv_to_string);

  ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
                     static_cast<Expr *> (new VariableExpr ("{:}.to_string")),
                     static_cast<Expr *> (new FunctionExpr (
                         static_cast<Function *> (nv_to_string))))));
}
} // namespace sf

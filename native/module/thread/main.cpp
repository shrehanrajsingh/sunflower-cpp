#include "main.hpp"

namespace sf
{
namespace native_mod
{
namespace Thread
{
SF_API Module *
make ()
{
  Module *mod = new Module (ModuleType::File);

  NativeFunction *nf_create
      = new NativeFunction (create, { "fname", "fargs" });
  FunctionObject *fo_create
      = new FunctionObject (static_cast<Function *> (nf_create));

  mod->set_variable ("create", static_cast<Object *> (fo_create));

  NativeFunction *nf_join = new NativeFunction (join, { "id" });
  FunctionObject *fo_join
      = new FunctionObject (static_cast<Function *> (nf_join));

  mod->set_variable ("join", static_cast<Object *> (fo_join));

  NativeFunction *nf_run = new NativeFunction (run, { "id" });
  FunctionObject *fo_run
      = new FunctionObject (static_cast<Function *> (nf_run));

  mod->set_variable ("run", static_cast<Object *> (fo_run));

  NativeFunction *nf_join_all = new NativeFunction (join_all, {});
  FunctionObject *fo_join_all
      = new FunctionObject (static_cast<Function *> (nf_join_all));

  mod->set_variable ("join_all", static_cast<Object *> (fo_join_all));

  return mod;
}
} // namespace Thread
} // namespace native_mod
} // namespace sf

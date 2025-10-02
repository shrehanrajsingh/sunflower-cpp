#include "main.hpp"

namespace sf
{
namespace native_mod
{
namespace File
{
SF_API Module *
make ()
{
  Module *mod = new Module (ModuleType::File);

  NativeFunction *nf_open = new NativeFunction (open, { "filename", "perms" });
  FunctionObject *fo_open
      = new FunctionObject (static_cast<Function *> (nf_open));

  mod->set_variable ("open", static_cast<Object *> (fo_open));

  NativeFunction *nf_read = new NativeFunction (read, { "fileid" });
  FunctionObject *fo_read
      = new FunctionObject (static_cast<Function *> (nf_read));

  mod->set_variable ("read", static_cast<Object *> (fo_read));

  return mod;
}
} // namespace File
} // namespace native_mod
} // namespace sf

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

  NativeFunction *nf_close = new NativeFunction (close, { "fileid" });
  FunctionObject *fo_close
      = new FunctionObject (static_cast<Function *> (nf_close));

  mod->set_variable ("close", static_cast<Object *> (fo_close));

  NativeFunction *nf_write = new NativeFunction (write, { "fileid", "str" });
  FunctionObject *fo_write
      = new FunctionObject (static_cast<Function *> (nf_write));

  mod->set_variable ("write", static_cast<Object *> (fo_write));

  NativeFunction *nf_seek_read
      = new NativeFunction (seek_read, { "fileid", "offset", "whence" });
  FunctionObject *fo_seek_read
      = new FunctionObject (static_cast<Function *> (nf_seek_read));

  mod->set_variable ("seek_read", static_cast<Object *> (fo_seek_read));

  NativeFunction *nf_seek_write
      = new NativeFunction (seek_write, { "fileid", "offset", "whence" });
  FunctionObject *fo_seek_write
      = new FunctionObject (static_cast<Function *> (nf_seek_write));

  mod->set_variable ("seek_write", static_cast<Object *> (fo_seek_write));

  NativeFunction *nf_read_n = new NativeFunction (read_n, { "fileid", "nb" });
  FunctionObject *fo_read_n
      = new FunctionObject (static_cast<Function *> (nf_read_n));

  mod->set_variable ("readn", static_cast<Object *> (fo_read_n));

  NativeFunction *nf_tell_read = new NativeFunction (tell_read, { "fileid" });
  FunctionObject *fo_tell_read
      = new FunctionObject (static_cast<Function *> (nf_tell_read));

  mod->set_variable ("tell_read", static_cast<Object *> (fo_tell_read));

  NativeFunction *nf_lsf = new NativeFunction (lsf, { "path" });
  FunctionObject *fo_lsf
      = new FunctionObject (static_cast<Function *> (nf_lsf));

  mod->set_variable ("lsf", static_cast<Object *> (fo_lsf));

  return mod;
}
} // namespace File
} // namespace native_mod
} // namespace sf

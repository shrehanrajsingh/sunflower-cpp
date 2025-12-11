#include "main.hpp"

namespace sf
{
namespace native_mod
{
namespace Socket
{
SF_API Module *
make ()
{
  Module *mod = new Module (ModuleType::File);

  NativeFunction *nf_socket = new NativeFunction (socket, {});
  FunctionObject *fo_socket
      = new FunctionObject (static_cast<Function *> (nf_socket));

  mod->set_variable ("socket", static_cast<Object *> (fo_socket));

  NativeFunction *nf_bind
      = new NativeFunction (bind, { "sock", "host", "port" });
  FunctionObject *fo_bind
      = new FunctionObject (static_cast<Function *> (nf_bind));

  mod->set_variable ("bind", static_cast<Object *> (fo_bind));

  NativeFunction *nf_listen = new NativeFunction (listen, { "sock", "count" });
  FunctionObject *fo_listen
      = new FunctionObject (static_cast<Function *> (nf_listen));

  mod->set_variable ("listen", static_cast<Object *> (fo_listen));

  NativeFunction *nf_accept
      = new NativeFunction (accept, { "sock", "host", "port" });
  FunctionObject *fo_accept
      = new FunctionObject (static_cast<Function *> (nf_accept));

  mod->set_variable ("accept", static_cast<Object *> (fo_accept));

  NativeFunction *nf_read = new NativeFunction (read, { "sock" });
  FunctionObject *fo_read
      = new FunctionObject (static_cast<Function *> (nf_read));

  mod->set_variable ("read", static_cast<Object *> (fo_read));

  NativeFunction *nf_send = new NativeFunction (send, { "sock", "msg" });
  FunctionObject *fo_send
      = new FunctionObject (static_cast<Function *> (nf_send));

  mod->set_variable ("send", static_cast<Object *> (fo_send));

  NativeFunction *nf_close = new NativeFunction (close, { "sock" });
  FunctionObject *fo_close
      = new FunctionObject (static_cast<Function *> (nf_close));

  mod->set_variable ("close", static_cast<Object *> (fo_close));

  return mod;
}
} // namespace Socket
} // namespace native_mod
} // namespace sf

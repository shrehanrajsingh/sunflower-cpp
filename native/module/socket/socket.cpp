#include "socket.hpp"

namespace sf
{
namespace native_mod
{
namespace Socket
{

/**
 * By default a TCP socket
 * SOCK_STREAM
 * Requires a connection
 */
SF_API Object *
socket (Module *mod)
{
  Object *res;

#ifdef _WIN32
  /* TODO */
#else
  int fd = ::socket (AF_INET, SOCK_STREAM, 0);

  assert (fd >= 0 && "failed to create socket");

  res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new IntegerConstant (fd))));
#endif

  IR (res);
  return res;
}

SF_API Object *
bind (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");
  Object *o_host = mod->get_variable ("host");
  Object *o_port = mod->get_variable ("port");

  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");
  assert (OBJ_IS_INT (o_port) && "port is not an integer");

  int fd = static_cast<IntegerConstant *> (
               static_cast<ConstantObject *> (o_sock)->get_c ().get ())
               ->get_value ();

  int port = static_cast<IntegerConstant *> (
                 static_cast<ConstantObject *> (o_port)->get_c ().get ())
                 ->get_value ();

#ifdef _WIN32

#else
  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (port);
  address.sin_family = AF_INET;

  if (::bind (fd, (sockaddr *)&address, (socklen_t)sizeof (address)) < 0)
    {
      std::cerr << "failed to bind socket\n";
      ::close (fd);

      Object *r = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (r);
      return r;
    }
#endif

  Object *res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (res);
  return res;
}

SF_API Object *
listen (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");
  Object *o_count = mod->get_variable ("count");

  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");
  assert (OBJ_IS_INT (o_count) && "count is not an integer");

  return NULL;
}

SF_API Object *
accept (Module *mod)
{
  return NULL;
}

SF_API Object *
read (Module *mod)
{
  return NULL;
}

SF_API Object *
send (Module *mod)
{
  return NULL;
}

} // namespace Socket
} // namespace native_mod
} // namespace sf

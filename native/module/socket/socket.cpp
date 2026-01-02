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
  SOCKET fd = ::socket (AF_INET, SOCK_STREAM, 0);

  assert (fd != INVALID_SOCKET && "failed to create socket");

  res = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new IntegerConstant (static_cast<int> (fd)))));

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
  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (static_cast<u_short> (port));
  address.sin_family = AF_INET;

  const char opt = 1;
  ::setsockopt (static_cast<SOCKET> (fd), SOL_SOCKET, SO_REUSEADDR, &opt,
                sizeof (opt));

  if (::bind (static_cast<SOCKET> (fd), (sockaddr *)&address, sizeof (address))
      == SOCKET_ERROR)
    {
      std::cerr << "failed to bind socket\n";
      ::closesocket (static_cast<SOCKET> (fd));

      Object *r = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (r);
      return r;
    }
#else
  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (port);
  address.sin_family = AF_INET;

  int opt = 1;
  ::setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
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

  int sock = static_cast<IntegerConstant *> (
                 static_cast<ConstantObject *> (o_sock)->get_c ().get ())
                 ->get_value ();

  int count = static_cast<IntegerConstant *> (
                  static_cast<ConstantObject *> (o_count)->get_c ().get ())
                  ->get_value ();

#ifdef _WIN32
  assert (::listen (static_cast<SOCKET> (sock), count) != SOCKET_ERROR);
#else
  assert (::listen (sock, count) >= 0 && "listen failed");
#endif

  Object *res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));
  IR (res);

  return res;
}

SF_API Object *
accept (Module *mod)
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
  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (static_cast<u_short> (port));
  address.sin_family = AF_INET;
  int addr_len = sizeof (address);
  SOCKET sid;

  if ((sid
       = ::accept (static_cast<SOCKET> (fd), (sockaddr *)&address, &addr_len))
      == INVALID_SOCKET)
    {
      std::cerr << "failed to accept socket\n";
      ::closesocket (fd);

      Object *r = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (r);
      return r;
    }

  Object *res = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new IntegerConstant (static_cast<int> (sid)))));
#else
  sockaddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons (port);
  address.sin_family = AF_INET;
  int addrlen = sizeof (address);
  int sid;

  if ((sid = ::accept (fd, (sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
      std::cerr << "failed to accept socket\n";
      ::close (fd);

      Object *r = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (r);
      return r;
    }

  Object *res = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new IntegerConstant (sid))));
#endif

  IR (res);
  return res;
}

SF_API Object *
read (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");
  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");

  int fd = static_cast<IntegerConstant *> (
               static_cast<ConstantObject *> (o_sock)->get_c ().get ())
               ->get_value ();

  std::string req;
  char buf[1024];

  while (1)
    {
#ifdef _WIN32
      ssize_t n = ::recv (static_cast<SOCKET> (fd), buf, 1024, 0);

      if (n == SOCKET_ERROR)
        {
          int err = WSAGetLastError ();
          if (err == WSAEINTR)
            continue;

          throw std::runtime_error ("read failed");
        }
#else
      ssize_t n = ::read (fd, static_cast<void *> (buf), 1024);

      if (n < 0)
        {
          if (errno == EINTR)
            continue;

          throw std::runtime_error ("read failed");
        }
#endif

      if (n == 0)
        break; /* client closed */

      req.append (buf, n);

      if (req.find ("\r\n\r\n") != std::string::npos)
        break;
    }

  size_t header_end = req.find ("\r\n\r\n");

  if (header_end == std::string::npos)
    {
      /* no body */
      Object *res;
      res = static_cast<Object *> (new ConstantObject (
          static_cast<Constant *> (new StringConstant ((req).c_str ()))));

      IR (res);
      return res;
    }

  size_t body_start = header_end + 4;
  size_t bytes_alr_read = req.size () - body_start;

  int content_length = 0;

  {
    size_t pos = req.find ("Content-Length:");
    if (pos != std::string::npos)
      {
        pos += strlen ("Content-Length:");
        while (pos < req.size () && req[pos] == ' ')
          pos++;

        content_length = std::stoi (req.substr (pos));
      }
  }

  std::string body;
  if (bytes_alr_read > 0)
    {
      body = req.substr (body_start);
    }

  while (static_cast<int> (body.size ()) < content_length)
    {
#ifdef _WIN32
      int n = ::recv (static_cast<SOCKET> (fd), buf, sizeof (buf), 0);
#else
      ssize_t n = ::read (fd, buf, sizeof (buf));
#endif
      if (n <= 0)
        break;

      body.append (buf, n);
    }

  Object *res;
  res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new StringConstant (
          (req.substr (0, req.find ("\r\n\r\n")) + "\r\n\r\n" + body)
              .c_str ()))));

  IR (res);
  return res;
}

SF_API Object *
send (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");
  Object *o_msg = mod->get_variable ("msg");

  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");
  assert (OBJ_IS_STR (o_msg) && "message is not a string");

  int fd = static_cast<IntegerConstant *> (
               static_cast<ConstantObject *> (o_sock)->get_c ().get ())
               ->get_value ();

  Str &msg = static_cast<StringConstant *> (
                 static_cast<ConstantObject *> (o_msg)->get_c ().get ())
                 ->get_value ();

#ifdef _WIN32
  ::send (static_cast<SOCKET> (fd), msg.to_std_string ().c_str (),
          static_cast<int> (msg.size ()), 0);
#else
  ::send (fd, (void *)(msg.to_std_string ().c_str ()), msg.size (), 0);
#endif

  Object *res;
  res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (res);
  return res;
}

SF_API Object *
close (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");

  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");

  int fd = static_cast<IntegerConstant *> (
               static_cast<ConstantObject *> (o_sock)->get_c ().get ())
               ->get_value ();

#ifdef _WIN32
  ::closesocket (static_cast<SOCKET> (fd));
#else
  ::close (fd);
#endif

  Object *res;
  res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (res);
  return res;
}

SF_API Object *
shutdown (Module *mod)
{
  Object *o_sock = mod->get_variable ("sock");
  Object *o_mode = mod->get_variable ("mode");

  assert (OBJ_IS_INT (o_sock) && "socket id is not an integer");
  assert (OBJ_IS_INT (o_mode) && "mode is not an integer");

  int fd = static_cast<IntegerConstant *> (
               static_cast<ConstantObject *> (o_sock)->get_c ().get ())
               ->get_value ();

  int mode = static_cast<IntegerConstant *> (
                 static_cast<ConstantObject *> (o_mode)->get_c ().get ())
                 ->get_value ();

#ifdef _WIN32
  ::shutdown (static_cast<SOCKET> (fd), mode);
#else
  ::shutdown (fd, mode);
#endif

  Object *res;
  res = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (res);
  return res;
}

} // namespace Socket
} // namespace native_mod
} // namespace sf

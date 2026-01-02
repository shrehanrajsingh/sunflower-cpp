#pragma once

#include "../../../header.hpp"
#include "../../../module.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace sf
{
namespace native_mod
{
namespace Socket
{
SF_API Object *socket (Module *);
SF_API Object *bind (Module *);
SF_API Object *listen (Module *);
SF_API Object *accept (Module *);
SF_API Object *read (Module *);
SF_API Object *send (Module *);
SF_API Object *close (Module *);
SF_API Object *shutdown (Module *);
} // namespace Socket
} // namespace native_mod
} // namespace sf

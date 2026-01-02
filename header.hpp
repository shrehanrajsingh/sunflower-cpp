#pragma once

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <variant>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#endif

#define SF_API
#define ERRMSG(X)                                                             \
  {                                                                           \
    std::cerr << __FILE__ << '(' << __LINE__ << "): " << X << std::endl;      \
    exit (EXIT_FAILURE);                                                      \
  }

#define here std::cout << __FILE__ << '(' << __LINE__ << ")" << std::endl;

#define SF_STANDARD_TABSIZE (4)

namespace sf
{
class GlobalEnv
{
private:
  std::map<std::string, std::string> gmap;

public:
  GlobalEnv () {}
  ~GlobalEnv () {}

  inline void
  add (std::string k, std::string v)
  {
    gmap[k] = v;
  }

  inline std::string &
  get (std::string k)
  {
    return gmap[k];
  }
};

using global_env_t = GlobalEnv;
SF_API global_env_t &__sf_get_global_env ();

#if !defined(SF_ENV)
#define SF_ENV(X) (__sf_get_global_env ().get ((X)))
#endif

} // namespace sf

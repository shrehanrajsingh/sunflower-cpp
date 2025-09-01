#include "sf-win.hpp"

#if defined(_WIN32)
#define _WIN32

double
pow (double a, double b)
{
  if (b == 0)
    return 1;

  double r = 1;
  while (b--)
    r *= a;

  return r;
}

#endif // _WIN32

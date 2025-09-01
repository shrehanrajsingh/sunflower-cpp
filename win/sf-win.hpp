#pragma once

#if defined(_WIN32)

/* implementation of POSIX specific functions
for windows */

#if defined(__cplusplus)
extern "C"
{
#endif // __cplusplus

  double pow (double, double);

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // _WIN32

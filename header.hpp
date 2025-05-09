#pragma once

#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SF_API
#define ERRMSG(X)                                                             \
  std::cerr << __FILE__ << '(' << __LINE__ << "): " << X << std::endl;        \
  exit (EXIT_FAILURE);

#define here std::cout << __FILE__ << '(' << __LINE__ << ")" << std::endl;
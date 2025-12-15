#pragma once

#include <algorithm>
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
#include <sstream>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SF_API
#define ERRMSG(X)                                                             \
  {                                                                           \
    std::cerr << __FILE__ << '(' << __LINE__ << "): " << X << std::endl;      \
    exit (EXIT_FAILURE);                                                      \
  }

#define here std::cout << __FILE__ << '(' << __LINE__ << ")" << std::endl;

#define SF_STANDARD_TABSIZE (4)
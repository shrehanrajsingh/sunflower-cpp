#pragma once

#include "../../header.hpp"
#include "../../vec.hpp"

/* file module */
#include "file/main.hpp"

namespace sf
{
namespace native_mod
{
using ModMap_ValType = std::function<Module *(void)>;
using ModMap_Type = std::map<std::string, ModMap_ValType>;

SF_API ModMap_Type &get_modmap ();
SF_API Module *get_mod (std::string);
SF_API void add_mod (std::string, ModMap_ValType);
SF_API void nmod_init ();
SF_API void nmod_destroy ();
} // namespace mod
} // namespace sf

#pragma once

#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
SF_API Object *_native_list_sort (Module *);
SF_API void _add_native_list_sort (Vec<Statement *> &);
} // namespace sf

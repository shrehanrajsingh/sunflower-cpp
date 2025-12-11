#pragma once

#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
SF_API Object *_native_int_to_string (Module *);
SF_API void _add_native_int_to_string (Vec<Statement *> &);
} // namespace sf

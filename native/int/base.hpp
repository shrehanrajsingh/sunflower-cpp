#pragma once

#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
SF_API Object *_native_int_base (Module *);
SF_API void _add_native_int_base (Vec<Statement *> &);
} // namespace sf

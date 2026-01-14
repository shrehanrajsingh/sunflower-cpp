#pragma once

#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
SF_API Object *_native_strip (Module *);
SF_API void _add_native_strip (Vec<Statement *> &);
} // namespace sf

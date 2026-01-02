#pragma once

#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
SF_API Object *_native_isalnum (Module *);
SF_API void _add_native_isalnum (Vec<Statement *> &);
} // namespace sf
#pragma once

#include "../../header.hpp"
#include "../../object.hpp"
#include "../../module.hpp"

namespace sf
{
SF_API Object *_native_list_clear (Module *);
SF_API void _add_native_list_clear (Vec<Statement *> &);
} // namespace sf

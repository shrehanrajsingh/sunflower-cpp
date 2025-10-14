#pragma once

#include "../header.hpp"
#include "../module.hpp"
#include "../object.hpp"
#include "../sfarray.hpp"

namespace sf
{
namespace native
{
SF_API Object *_native_input (Module *);
SF_API void _add_native_input (Vec<Statement *> &);
} // namespace native
} // namespace sf
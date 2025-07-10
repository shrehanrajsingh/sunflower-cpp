#pragma once

#include "../header.hpp"
#include "../module.hpp"
#include "../object.hpp"
#include "../sfarray.hpp"

namespace sf
{
namespace native
{
SF_API Object *_native_write (Module *);
SF_API void _add_native_write (Vec<Statement *> &);
} // namespace native
} // namespace sf

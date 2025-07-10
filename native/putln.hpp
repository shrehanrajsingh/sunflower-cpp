#pragma once

#include "../header.hpp"
#include "../module.hpp"
#include "../object.hpp"

namespace sf
{
namespace native
{
SF_API Object *_native_putln (Module *);
SF_API void _add_native_putln (Vec<Statement *> &);
} // namespace native
} // namespace sf
#pragma once
#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
// Function that converts string to lowercase
SF_API Object *_native_lower (Module *);

// Function to register 'lower' in the Sunflower engine
SF_API void _add_native_lower (Vec<Statement *> &);
}
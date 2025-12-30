#pragma once
#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
// Function that converts string to uppercase
SF_API Object *_native_upper (Module *);

// Function to register 'upper' in the Sunflower engine
SF_API void _add_native_upper (Vec<Statement *> &);
}
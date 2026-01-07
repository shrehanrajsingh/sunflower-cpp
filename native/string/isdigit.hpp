#pragma once
#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"
namespace sf
{
SF_API Object *
_native_isdigit (Module *); // Change to _native_isalpha for the other file
SF_API void _add_native_isdigit (Vec<Statement *> &);
}
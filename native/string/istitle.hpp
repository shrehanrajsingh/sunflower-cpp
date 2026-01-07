#pragma once
#include "../../header.hpp"
#include "../../module.hpp"
#include "../../object.hpp"

namespace sf
{
// native method function declaration
SF_API Object *_native_istitle (Module *);

// registration function declaration
SF_API void _add_native_istitle (Vec<Statement *> &);
}
#pragma once

#include "../header.hpp"
#include "../module.hpp"
#include "../object.hpp"

/* native functions */
#include "put.hpp"
#include "putln.hpp"
#include "write.hpp"

/* string methods */
#include "string/count.hpp"
#include "string/find.hpp"
#include "string/replace.hpp"

/* integer methods */
#include "int/base.hpp"

namespace sf
{
namespace native
{
/* native functions go here */
SF_API void add_natives (Vec<Statement *> &);
} // namespace native
} // namespace sf

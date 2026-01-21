#pragma once

#include "../header.hpp"
#include "../module.hpp"
#include "../object.hpp"

/* native functions */
#include "exit.hpp"
#include "input.hpp"
#include "len.hpp"
#include "put.hpp"
#include "putln.hpp"
#include "sleep.hpp"
#include "write.hpp"

/* string methods */
#include "string/count.hpp"
#include "string/find.hpp"
#include "string/replace.hpp"
#include "string/split.hpp"
#include "string/strip.hpp"

/* integer methods */
#include "int/base.hpp"
#include "int/to_string.hpp"

/* list methods */
#include "list/pop.hpp"
#include "list/push.hpp"

/* dict methods */
#include "dict/to_string.hpp"

namespace sf
{
namespace native
{
/* native functions go here */
SF_API void add_natives (Vec<Statement *> &);
} // namespace native
} // namespace sf

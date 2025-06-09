#pragma once

#include "arithmetic.hpp"
#include "expr.hpp"
#include "header.hpp"
#include "stmt.hpp"
#include "tokenize.hpp"

namespace sf
{
Expr *expr_gen (Vec<Token *> &_Tokens, size_t _StartIdx, size_t _EndIdx);
Vec<Statement *> stmt_gen (Vec<Token *> &_Tokens);
} // namespace sf

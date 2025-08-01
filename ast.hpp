#pragma once

#include "arithmetic.hpp"
#include "expr.hpp"
#include "header.hpp"
#include "stmt.hpp"
#include "tokenize.hpp"

#define SET_LINE_NUMBER(X, Y)                                                 \
  (X).back ()->get_line_number () = (Y)->get_line_number ()

namespace sf
{
Expr *expr_gen (Vec<Token *> &_Tokens, size_t _StartIdx, size_t _EndIdx);
Vec<Statement *> stmt_gen (Vec<Token *> &_Tokens);
} // namespace sf

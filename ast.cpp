#include "ast.hpp"
#include "expr.hpp"
#include "sfarray.hpp"
#include "sfclass.hpp"
#include "sfdict.hpp"

namespace sf
{
void _sf_ast_getline (Vec<Token *> &_Data, size_t _StartIdx,
                      Vec<Token *> &_Buf, bool _Reverse = false);

int _sf_ast_getline_idx (Vec<Token *> &_Data, size_t _StartIdx,
                         bool _Reverse = false);

/* returns the next ',' idx */
size_t _sf_ast_getarg_idx (Vec<Token *> &_Data, size_t _StartIdx, int _GB = 0);

size_t _sf_ast_gettabspace (Vec<Token *> &_Data, int _StartIdx);

size_t _sf_ast_getblock_idx (Vec<Token *> &_Data, size_t _StartIdx,
                             size_t _Tabspace = 0);

Expr *
expr_gen (Vec<Token *> &toks, size_t st, size_t ed)
{
  Expr *res = nullptr;

  size_t i = st;
  int gb = 0;

  /* try catch */
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "try" && !gb)
              {
                size_t catch_kwidx = i + 1;
                int gb = 0;

                for (size_t j = catch_kwidx; j < ed; j++)
                  {
                    Token *d = toks[j];

                    switch (c->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          Str &dop
                              = static_cast<OperatorToken *> (d)->get_val ();

                          if (dop == '(' || dop == '{' || dop == '[')
                            gb++;
                          if (dop == ')' || dop == '}' || dop == ']')
                            gb--;
                        }
                        break;

                      case SFTokenType::Keyword:
                        {
                          Str &kw
                              = static_cast<KeywordToken *> (d)->get_val ();

                          if (kw == "catch" && !gb)
                            {
                              catch_kwidx = j;
                              goto tce_fl1;
                              break;
                            }
                        }
                        break;

                      default:
                        break;
                      }
                  }

              tce_fl1:;

                TryCatchExpr *tce = new TryCatchExpr ();

                if (catch_kwidx == i + 1)
                  {
                    /* no catch */
                    tce->get_tryexpr () = expr_gen (toks, i + 1, ed);
                  }
                else
                  {
                    tce->get_tryexpr () = expr_gen (toks, i + 1, catch_kwidx);

                    if (catch_kwidx + 2 >= ed)
                      {
                        /* no var mentioned */
                        tce->get_catchexpr ()
                            = expr_gen (toks, catch_kwidx + 1, ed);
                      }
                    else
                      {
                        Token *c2 = toks[catch_kwidx + 2];

                        if (c2->get_type () == SFTokenType::Operator
                            && static_cast<OperatorToken *> (c2)->get_val ()
                                   == "->")
                          {
                            tce->get_catchcvar () = expr_gen (
                                toks, catch_kwidx + 1, catch_kwidx + 3);

                            tce->get_catchexpr ()
                                = expr_gen (toks, catch_kwidx + 3, ed);
                          }
                      }
                  }

                res = static_cast<Expr *> (tce);
                goto ret;
              }
          }
          break;

        default:
          break;
        }

    end4:
      i++;
    }

  i = ed - 1;
  gb = 0;

  /* inline for */
  while (i > st)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "for" && !gb)
              {
                int gb = 0;
                Vec<Expr *> vars;
                Expr *iterable;
                Expr *body;
                size_t last_v_idx = i + 1;

                for (size_t j = i + 1; j < ed; j++)
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          Str &dop
                              = static_cast<OperatorToken *> (d)->get_val ();

                          if (dop == ',' && !gb)
                            {
                              vars.push_back (expr_gen (toks, last_v_idx, j));

                              last_v_idx = j;
                              continue;
                            }

                          if (dop == '(' || dop == '{' || dop == '[')
                            gb++;
                          if (dop == ')' || dop == '}' || dop == ']')
                            gb--;
                        }
                        break;

                      case SFTokenType::Keyword:
                        {
                          Str &kw
                              = static_cast<KeywordToken *> (d)->get_val ();

                          if (kw == "in" && !gb)
                            {
                              vars.push_back (expr_gen (toks, last_v_idx, j));
                              last_v_idx = j;

                              goto out_kwin_scan;
                            }
                        }
                        break;

                      default:
                        break;
                      }
                  }

              out_kwin_scan:;

                iterable = expr_gen (toks, last_v_idx + 1, ed);
                body = expr_gen (toks, st, i);

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (
                    new InlineForExpr (vars, iterable, body));
                goto ret;
              }
          }
          break;

        default:
          break;
        }

    end3:
      i--;
    }

  /* for repeat expression we start from the end */
  i = ed - 1;
  gb = 0;

  /* repeat expression */
  while (i > st)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "repeat" && !gb)
              {
                Expr *times = expr_gen (toks, i + 1, ed);
                Expr *body = expr_gen (toks, st, i);

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (new RepeatExpr (times, body));
                goto ret;
              }
          }
          break;

        default:
          break;
        }

    end2:
      i--;
    }

  i = st;
  gb = 0;

  // to..step has more precedence than ==, != etc.
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "to" && !gb)
              {
                Expr *lval = expr_gen (toks, st, i);
                Expr *rval = nullptr;
                Expr *step = nullptr;
                int gb = 0;
                size_t step_idx = i;

                while (step_idx < ed)
                  {
                    Token *d = toks[step_idx];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *t = static_cast<OperatorToken *> (d);
                          Str &tv = t->get_val ();

                          if (tv == "(" || tv == "[" || tv == "{")
                            gb++;
                          else if (tv == ")" || tv == "]" || tv == "}")
                            gb--;
                        }
                        break;

                      case SFTokenType::Keyword:
                        {
                          KeywordToken *kt = static_cast<KeywordToken *> (d);
                          Str &ktv = kt->get_val ();

                          if (ktv == "step" && !gb)
                            {
                              rval = expr_gen (toks, i + 1, step_idx);
                              step = expr_gen (toks, step_idx + 1, ed);
                              goto l2;
                            }
                        }
                        break;

                      default:
                        break;
                      }

                    step_idx++;
                  }
              l2:;

                if (rval == nullptr)
                  {
                    /* no step in to..step clause */
                    rval = expr_gen (toks, i + 1, ed);
                  }

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (
                    new ToStepClause (lval, rval, step));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

    end1:
      i++;
    }

  /* or clause */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "or" && !gb)
              {
                Expr *lval = expr_gen (toks, st, i);
                Expr *rval = expr_gen (toks, i + 1, ed);
                int gb = 0;

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (new LogicalOrExpr (lval, rval));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* and clause */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "and" && !gb)
              {
                Expr *lval = expr_gen (toks, st, i);
                Expr *rval = expr_gen (toks, i + 1, ed);
                int gb = 0;

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (new LogicalAndExpr (lval, rval));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }
  i = st;
  gb = 0;

  /* not clause is after comparison operators */

  /* for comparison operators */
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *opt = static_cast<OperatorToken *> (c);
            Str &opv = opt->get_val ();

            if (opv == '(' || opv == '{' || opv == '[')
              gb++;
            if (opv == ')' || opv == '}' || opv == ']')
              gb--;

            if (opv == "==" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::EqEq, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
            else if (opv == "!=" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::NEq, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
            else if (opv == "<=" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::LEq, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
            else if (opv == ">=" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::GEq, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
            else if (opv == "<" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::Le, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
            else if (opv == ">" && !gb)
              {
                res = static_cast<Expr *> (new ConditionalExpr (
                    ConditionalType::Ge, expr_gen (toks, st, i),
                    expr_gen (toks, i + 1, ed)));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* not clause */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "not" && !gb)
              {
                Expr *val = expr_gen (toks, i + 1, ed);
                int gb = 0;

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (new LogicalNotExpr (val));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* in clause */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;
          }
          break;

        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &ktv = kt->get_val ();

            if (ktv == "in" && !gb)
              {
                Expr *lhs = expr_gen (toks, st, i);
                Expr *rhs = expr_gen (toks, i + 1, ed);

                if (res != nullptr)
                  delete res;

                res = static_cast<Expr *> (new InExpr (lhs, rhs));
                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* | */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;

            if (cop == "|" && !gb)
              {
                Expr *l = expr_gen (toks, st, i);
                Expr *r = expr_gen (toks, i + 1, ed);

                res = static_cast<Expr *> (new BitOrExpr (l, r));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* & */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;

            if (cop == "&" && !gb)
              {
                Expr *l = expr_gen (toks, st, i);
                Expr *r = expr_gen (toks, i + 1, ed);

                res = static_cast<Expr *> (new BitAndExpr (l, r));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* <<, >> */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;

            if (cop == "<<" && !gb)
              {
                Expr *l = expr_gen (toks, st, i);
                Expr *r = expr_gen (toks, i + 1, ed);

                res = static_cast<Expr *> (new BitLeftShiftExpr (l, r));

                goto ret;
              }

            if (cop == ">>" && !gb)
              {
                Expr *l = expr_gen (toks, st, i);
                Expr *r = expr_gen (toks, i + 1, ed);

                res = static_cast<Expr *> (new BitRightShiftExpr (l, r));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  /* ~ */
  i = st;
  gb = 0;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            Str &cop = static_cast<OperatorToken *> (c)->get_val ();

            if (cop == '(' || cop == '{' || cop == '[')
              gb++;
            if (cop == ')' || cop == '}' || cop == ']')
              gb--;

            if (cop == "~" && !gb)
              {
                Expr *v = expr_gen (toks, i + 1, ed);
                res = static_cast<Expr *> (new BitNegateExpr (v));

                goto ret;
              }
          }
          break;

        default:
          break;
        }

      i++;
    }

  i = st;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *opt = static_cast<OperatorToken *> (c);
            Str &opv = opt->get_val ();

            if (opv == "=")
              {
                int vv_idx;
                vv_idx = _sf_ast_getline_idx (toks, i + 1);

                Expr *e_name = res;
                Expr *e_val = expr_gen (toks, i + 1, vv_idx);

                res = static_cast<Expr *> (new VarDeclExpr (e_name, e_val));

                i = vv_idx;
              }
            else if (opv == "[")
              {
                if (res != nullptr)
                  {
                    /**
                     * Index access
                     * Example,
                     * a [21]
                     * ^
                     * |
                     * `a` is a VariableExpr, stored in res by the time
                     * interpreter reaches '['
                     */
                    int gb = 0;
                    size_t j = i + 1;
                    size_t end_idx = i;

                    while (j < ed)
                      {
                        Token *d = toks[j];

                        switch (d->get_type ())
                          {
                          case SFTokenType::Operator:
                            {
                              OperatorToken *t
                                  = static_cast<OperatorToken *> (d);
                              Str &tv = t->get_val ();
                              if (tv == "]" && !gb)
                                {
                                  end_idx = j;
                                  goto l1;
                                }

                              if (tv == "(" || tv == "[" || tv == "{")
                                gb++;
                              else if (tv == ")" || tv == "]" || tv == "}")
                                gb--;
                            }
                            break;

                          default:
                            break;
                          }

                        j++;
                      }

                  l1:
                    Expr *arr = res;
                    Expr *idx = expr_gen (toks, i + 1, end_idx);

                    res = static_cast<Expr *> (new ArrayAccess (arr, idx));
                    i = end_idx;

                    goto end;
                  }
                else
                  {
                    int gb = 0;
                    Vec<Expr *> arr_args;
                    size_t j = i + 1;
                    size_t last_arg_idx = i + 1;

                    while (j < ed)
                      {
                        Token *d = toks[j];

                        switch (d->get_type ())
                          {
                          case SFTokenType::Operator:
                            {
                              OperatorToken *t
                                  = static_cast<OperatorToken *> (d);
                              Str &tv = t->get_val ();

                              if (tv == "," && !gb)
                                {
                                  arr_args.push_back (
                                      expr_gen (toks, last_arg_idx, j));

                                  last_arg_idx = j;
                                }
                              else if (tv == "]" && !gb)
                                {
                                  if (last_arg_idx != j)
                                    {
                                      /* non-empty array */
                                      arr_args.push_back (
                                          expr_gen (toks, last_arg_idx, j));

                                      last_arg_idx = j;
                                      break;
                                    }
                                }

                              if (tv == "(" || tv == "[" || tv == "{")
                                gb++;
                              else if (tv == ")" || tv == "]" || tv == "}")
                                gb--;
                            }
                            break;

                          default:
                            break;
                          }

                        j++;
                      }

                    /* do not add if (res != nullptr) clause, check the
                     * condition of the if statement */
                    res = static_cast<Expr *> (new ArrayExpr (arr_args));
                    i = last_arg_idx;
                  }
              }
            else if (opv == "{")
              {
                int gb = 0;
                Vec<Expr *> dict_keys;
                Vec<Expr *> dict_values;
                size_t j = i + 1;
                size_t key_idx = i + 1;
                size_t val_idx = 0;
                bool parsing_key = true;

                while (j < ed)
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *t = static_cast<OperatorToken *> (d);
                          Str &tv = t->get_val ();

                          if (tv == ":" && !gb && parsing_key)
                            {
                              dict_keys.push_back (
                                  expr_gen (toks, key_idx, j));
                              val_idx = j + 1;
                              parsing_key = false;
                            }
                          else if (tv == "," && !gb && !parsing_key)
                            {
                              dict_values.push_back (
                                  expr_gen (toks, val_idx, j));
                              key_idx = j + 1;
                              parsing_key = true;
                            }
                          else if (tv == "}" && !gb)
                            {
                              if (!parsing_key && val_idx < j)
                                {
                                  dict_values.push_back (
                                      expr_gen (toks, val_idx, j));
                                }
                              goto dict_done;
                            }

                          if (tv == "(" || tv == "[" || tv == "{")
                            gb++;
                          else if (tv == ")" || tv == "]" || tv == "}")
                            gb--;
                        }
                        break;

                      default:
                        break;
                      }

                    j++;
                  }

              dict_done:
                std::map<Expr *, Expr *> r;

                for (size_t k = 0; k < dict_keys.get_size (); k++)
                  r[dict_keys[k]] = dict_values[k];

                res = static_cast<Expr *> (new DictExpr (r));
                i = j;
              }
            else if (opv == "(")
              {
                if (res != nullptr)
                  {
                    /* function call */
                    int gb = 0;
                    size_t last_arg_idx = i + 1;
                    Vec<Expr *> args;

                    size_t j;
                    for (j = last_arg_idx; j < ed; j++)
                      {
                        Token *d = toks[j];

                        if (d->get_type () == SFTokenType::Operator)
                          {
                            Str &dop
                                = static_cast<OperatorToken *> (d)->get_val ();

                            if (dop == ',' && !gb)
                              {
                                args.push_back (
                                    expr_gen (toks, last_arg_idx, j));
                                last_arg_idx = j + 1;
                              }

                            if (dop == ')' && !gb)
                              {
                                if (j == i + 1)
                                  {
                                    /* no args */
                                  }
                                else
                                  {
                                    args.push_back (
                                        expr_gen (toks, last_arg_idx, j));
                                  }

                                break;
                              }

                            if (dop == "(" || dop == "[" || dop == "{")
                              gb++;
                            else if (dop == ")" || dop == "]" || dop == "}")
                              gb--;
                          }
                      }

                    res = static_cast<Expr *> (new FuncCallExpr (res, args));
                    i = j;
                  }
                else
                  {
                    /* normal brackets / tuple */
                    int gb = 0;
                    size_t br_end_idx = i + 1;

                    while (br_end_idx < ed)
                      {
                        Token *&d = toks[br_end_idx];

                        if (d->get_type () == SFTokenType::Operator)
                          {
                            Str &dop
                                = static_cast<OperatorToken *> (d)->get_val ();

                            if (dop == ')' && !gb)
                              break;

                            if (dop == '(' || dop == '{' || dop == '[')
                              gb++;
                            if (dop == '}' || dop == ']')
                              gb--;
                          }

                        br_end_idx++;
                      }

                    res = expr_gen (toks, i + 1, br_end_idx);
                    i = br_end_idx;
                    goto end;
                  }
              }
            else if (opv == '+' || opv == '-' || opv == '*' || opv == '/')
              {
                Vec<AVBase *> a;

                if (res == nullptr)
                  {
                    /**
                     * TODO
                     * Could be a unary operator (like +, -)
                     */
                    a.push_back (new AVOperand (static_cast<Expr *> (
                        new ConstantExpr (static_cast<Constant *> (
                            new IntegerConstant (0))))));
                  }
                else
                  {
                    a.push_back (new AVOperand (res));
                  }

                size_t last_op_idx = i + 1;
                a.push_back (new AVOperator (opv.to_std_string ().c_str ()));
                int gb = 0;
                bool all_const = static_cast<AVOperand *> (a.front ())
                                     ->get_val ()
                                     ->get_type ()
                                 == ExprType::Constant;

                for (size_t j = last_op_idx; j < ed; j++)
                  {
                    Token *d = toks[j];

                    if (d->get_type () == SFTokenType::Operator)
                      {
                        Str &dop
                            = static_cast<OperatorToken *> (d)->get_val ();

                        if (dop == "(" || dop == "[" || dop == "{")
                          gb++;
                        else if (dop == ")" || dop == "]" || dop == "}")
                          gb--;

                        if ((dop == '+' || dop == '-' || dop == '*'
                             || dop == '/')
                            && !gb)
                          {
                            Expr *e = expr_gen (toks, last_op_idx, j);
                            all_const &= e->get_type () == ExprType::Constant;

                            // e->print ();
                            a.push_back (
                                static_cast<AVBase *> (new AVOperand (e)));
                            a.push_back (
                                static_cast<AVBase *> (new AVOperator (
                                    dop.to_std_string ().c_str ())));

                            last_op_idx = j + 1;
                          }
                      }

                    if (j == ed - 1)
                      {
                        Expr *e = expr_gen (toks, last_op_idx, j + 1);
                        all_const &= e->get_type () == ExprType::Constant;

                        // e->print ();
                        a.push_back (
                            static_cast<AVBase *> (new AVOperand (e)));
                      }
                  }

                // if (all_const)
                //   {
                //     /* evaluate in place */
                //   }
                // else
                //   {
                Arithmetic *arp = new Arithmetic (Arithmetic::from_infix (a));
                res = static_cast<Expr *> (arp);
                // }

                // res->print ();

                goto ret;
              }
            else if (opv == '.')
              {
                /**
                 * For now, we assume dot accesses are simple and of
                 * the form
                 * <some_huge_expression>.<single_variable_name>
                 */

                assert (i + 1 < ed);
                Token *n = toks[i + 1];

                if (n->get_type () != SFTokenType::Identifier)
                  {
                    here;
                    n->print ();
                    throw std::runtime_error ("Syntax Error: DotAccess should "
                                              "be of type expr.variable");
                  }

                res = static_cast<Expr *> (new DotAccess (
                    res,
                    static_cast<Expr *> (new VariableExpr (
                        static_cast<IdentifierToken *> (n)->get_val ()))));

                i++;
                goto end;
              }
            else if (opv == '?')
              {
                /**
                 * Ambig is defined as an operator while
                 * token scanning and used as a constant
                 * in practice
                 */

                if (i + 1 == ed)
                  {
                    res = static_cast<Expr *> (new ConstantExpr (
                        static_cast<Constant *> (new AmbigConstant ())));
                  }
                else
                  {
                    res = static_cast<Expr *> (
                        new ConstantExpr (static_cast<Constant *> (
                            new AmbigConstant (expr_gen (toks, i + 1, ed)))));
                    goto ret;
                  }
              }
          }
          break;

        case SFTokenType::Identifier:
          {
            IdentifierToken *idt = static_cast<IdentifierToken *> (c);

            if (res != nullptr)
              delete res;

            res = static_cast<Expr *> (new VariableExpr (idt->get_val ()));
          }
          break;

        case SFTokenType::Float:
          {
            if (res != nullptr)
              delete res;

            FloatToken *ft = static_cast<FloatToken *> (c);
            res = static_cast<Expr *> (new ConstantExpr (
                static_cast<Constant *> (new FloatConstant (ft->get_val ()))));
          }
          break;

        case SFTokenType::Integer:
          {
            if (res != nullptr)
              delete res;

            IntegerToken *it = static_cast<IntegerToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new IntegerConstant (it->get_val ()))));
          }
          break;

        case SFTokenType::String:
          {
            if (res != nullptr)
              delete res;

            StringToken *st = static_cast<StringToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new StringConstant (st->get_val ()))));
          }
          break;

        case SFTokenType::Boolean:
          {
            if (res != nullptr)
              delete res;

            BooleanToken *bt = static_cast<BooleanToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new BooleanConstant (bt->get_val ()))));
          }
          break;

        case SFTokenType::NoneType:
          {
            if (res != nullptr)
              delete res;

            res = static_cast<Expr *> (new ConstantExpr (
                static_cast<Constant *> (new NoneConstant ())));
          }
          break;

        default:
          break;
        }

    end:
      i++;
    }

ret:
  return res;
}

Vec<Statement *>
stmt_gen (Vec<Token *> &toks)
{
  Vec<Statement *> res;

  size_t i = 0;
  while (i < toks.get_size ())
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *opt = static_cast<OperatorToken *> (c);
            Str &opv = opt->get_val ();

            if (opv == "=")
              {
                int nv_idx, vv_idx;
                nv_idx = _sf_ast_getline_idx (toks, i - 1, true);
                vv_idx = _sf_ast_getline_idx (toks, i + 1);

                // std::cout << "name_tokens\n";
                // for (size_t j = nv_idx; j < i; j++)
                //   {
                //     toks[j]->print ();
                //   }
                // std::cout << "\nval_tokens\n";
                // for (size_t j = i + 1; j < vv_idx; j++)
                //   {
                //     toks[j]->print ();
                //   }
                // std::cout << "\n---------\n";

                Expr *e_name = expr_gen (toks, nv_idx, i);
                Expr *e_val = expr_gen (toks, i + 1, vv_idx);

                assert (e_name != nullptr);
                assert (e_val != nullptr);

                res.push_back (static_cast<Statement *> (
                    new VarDeclStatement (e_name, e_val)));

                SET_LINE_NUMBER (res, c);
                i = vv_idx;
              }

            else if (opv == "(")
              {
                int nv_idx = _sf_ast_getline_idx (toks, i - 1, true);

                size_t last_arg_idx = i + 1;
                size_t j = _sf_ast_getarg_idx (toks, last_arg_idx);

                Vec<Expr *> args;

                if (j != last_arg_idx)
                  {
                    while (j < toks.get_size ())
                      {
                        if (toks[j]->get_type () == SFTokenType::Operator
                            && static_cast<OperatorToken *> (toks[j])
                                       ->get_val ()
                                   == ')')
                          {
                            args.push_back (expr_gen (toks, last_arg_idx, j));
                            break;
                          }

                        args.push_back (expr_gen (toks, last_arg_idx, j));
                        last_arg_idx = j + 1;
                        j = _sf_ast_getarg_idx (toks, last_arg_idx);
                      }
                  }

                res.push_back (static_cast<Statement *> (
                    new FuncCallStatement (expr_gen (toks, nv_idx, i), args)));

                SET_LINE_NUMBER (res, c);
                i = j;
              }
          }
          break;
        case SFTokenType::Keyword:
          {
            KeywordToken *kt = static_cast<KeywordToken *> (c);
            Str &kw = kt->get_val ();

            if (kw == "if")
              {
                int gb = 0;
                size_t cond_end_idx = i;

                for (size_t j = i + 1; j < toks.get_size (); j++)
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *jop
                              = static_cast<OperatorToken *> (d);

                          Str &js = jop->get_val ();

                          if (js == '(' || js == '[' || js == '{')
                            gb++;

                          if (js == ')' || js == ']' || js == '}')
                            gb--;
                        }
                        break;

                      case SFTokenType::Newline:
                        {
                          if (!gb)
                            {
                              cond_end_idx = j;
                              goto l1;
                            }
                        }
                        break;

                      default:
                        break;
                      }
                  }
              l1:;

                Expr *cond = expr_gen (toks, i + 1, cond_end_idx);
                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                // std::cout << "-----\n";
                for (size_t j = cond_end_idx + 1; j < block_end_idx; j++)
                  {
                    // toks[j]->print ();
                    body_toks.push_back (toks[j]);
                  }
                // std::cout << "-----end\n";

                Vec<Statement *> body = stmt_gen (body_toks);

                IfConstruct *ifst = new IfConstruct (cond, body, {}, {});

                // std::cout << block_end_idx << '\t' << toks.get_size () <<
                // '\n';
              l5:
                while (block_end_idx < toks.get_size ()
                       && (toks[block_end_idx]->get_type ()
                               == SFTokenType::Newline
                           || toks[block_end_idx]->get_type ()
                                  == SFTokenType::Tabspace))
                  block_end_idx++;

                if (block_end_idx < toks.get_size ()
                    && toks[block_end_idx]->get_type () == SFTokenType::Keyword
                    && static_cast<KeywordToken *> (toks[block_end_idx])
                               ->get_val ()
                           == "else")
                  {
                    /**
                     * TODO: check for `else if`
                     */
                    if (block_end_idx < toks.get_size ()
                        && toks[block_end_idx + 1]->get_type ()
                               == SFTokenType::Keyword
                        && static_cast<KeywordToken *> (
                               toks[block_end_idx + 1])
                                   ->get_val ()
                               == "if")
                      {
                        int gb = 0;
                        size_t elseif_cond_start = block_end_idx + 2;
                        size_t elseif_cond_end = elseif_cond_start;

                        for (size_t j = elseif_cond_start;
                             j < toks.get_size (); j++)
                          {
                            Token *d = toks[j];

                            switch (d->get_type ())
                              {
                              case SFTokenType::Operator:
                                {
                                  OperatorToken *jop
                                      = static_cast<OperatorToken *> (d);
                                  Str &js = jop->get_val ();

                                  if (js == '(' || js == '[' || js == '{')
                                    gb++;
                                  if (js == ')' || js == ']' || js == '}')
                                    gb--;
                                }
                                break;

                              case SFTokenType::Newline:
                                {
                                  if (!gb)
                                    {
                                      elseif_cond_end = j;
                                      goto l4;
                                    }
                                }
                                break;

                              default:
                                break;
                              }
                          }
                      l4:

                        Expr *elseif_cond = expr_gen (toks, elseif_cond_start,
                                                      elseif_cond_end);

                        size_t elseif_block_end = _sf_ast_getblock_idx (
                            toks, block_end_idx + 2,
                            _sf_ast_gettabspace (toks, block_end_idx + 2));

                        Vec<Token *> elseif_body_toks;
                        for (size_t j = elseif_cond_end + 1;
                             j < elseif_block_end; j++)
                          {
                            elseif_body_toks.push_back (toks[j]);
                          }

                        Vec<Statement *> elseif_body
                            = stmt_gen (elseif_body_toks);

                        ifst->get_elifconstructs ().push_back (
                            new IfConstruct (elseif_cond, elseif_body, {},
                                             {}));

                        block_end_idx = elseif_block_end;

                        goto l5;
                      }
                    else
                      {
                        size_t else_bei = _sf_ast_getblock_idx (
                            toks, block_end_idx,
                            _sf_ast_gettabspace (toks, block_end_idx));

                        Vec<Token *> else_body_toks;
                        for (size_t j = block_end_idx + 1; j < else_bei; j++)
                          else_body_toks.push_back (toks[j]);

                        Vec<Statement *> else_body = stmt_gen (else_body_toks);
                        ifst->get_else_body () = else_body;

                        block_end_idx = else_bei;
                      }
                  }

                res.push_back (ifst);
                SET_LINE_NUMBER (res, c);
                i = block_end_idx - 1;
              }
            else if (kw == "for")
              {
                int gb = 0;
                size_t last_arg_idx = i + 1;
                size_t j = i + 1;
                size_t in_idx = i;
                size_t body_start_idx = i;
                Vec<Expr *> arg_list;
                Expr *iterable = nullptr;

                while (j < toks.get_size ())
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *jop
                              = static_cast<OperatorToken *> (d);

                          Str &js = jop->get_val ();

                          if (js == ',' && !gb)
                            {
                              arg_list.push_back (
                                  expr_gen (toks, last_arg_idx, j));
                              last_arg_idx = j;
                            }

                          if (js == '(' || js == '[' || js == '{')
                            gb++;

                          if (js == ')' || js == ']' || js == '}')
                            gb--;
                        }
                        break;

                      case SFTokenType::Keyword:
                        {
                          KeywordToken *kwt = static_cast<KeywordToken *> (d);

                          Str &kw = kwt->get_val ();

                          if (kw == "in")
                            {
                              arg_list.push_back (
                                  expr_gen (toks, last_arg_idx, j));
                              in_idx = j;
                              goto l2;
                            }
                        }
                        break;

                      default:
                        break;
                      }

                    j++;
                  }
              l2:;

                gb = 0;
                while (body_start_idx < toks.get_size ())
                  {
                    Token *d = toks[body_start_idx];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *jop
                              = static_cast<OperatorToken *> (d);

                          Str &js = jop->get_val ();

                          if (js == '(' || js == '[' || js == '{')
                            gb++;

                          if (js == ')' || js == ']' || js == '}')
                            gb--;
                        }
                        break;

                      case SFTokenType::Newline:
                        {
                          if (!gb)
                            goto l3;
                        }
                        break;

                      default:
                        break;
                      }

                    body_start_idx++;
                  }
              l3:;
                iterable = expr_gen (toks, in_idx + 1, body_start_idx);

                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, body_start_idx, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                for (size_t j = body_start_idx + 1; j < block_end_idx; j++)
                  {
                    body_toks.push_back (toks[j]);
                  }

                ForConstruct *fc = new ForConstruct (arg_list, iterable,
                                                     stmt_gen (body_toks));

                res.push_back (fc);
                SET_LINE_NUMBER (res, c);
                i = block_end_idx - 1;
              }
            else if (kw == "fun")
              {
                bool is_simple = false;
                /**
                 * There can be two types of function definitions
                 * Simple: functions with a name
                 * Example:
                 * fun name (args)
                 *    ...body
                 * The other type is Overloads
                 * Example:
                 * fun ''.replace(self, other)
                 *    ...body
                 */

                if (i + 2 < toks.get_size ()
                    && (toks[i + 2]->get_type () == SFTokenType::Operator)
                    && static_cast<OperatorToken *> (toks[i + 2])->get_val ()
                           == '(')
                  is_simple = true;

                // other logic (none thought of yet)

                if (is_simple)
                  {
                    Token *name_tok = toks[i + 1];
                    assert (name_tok->get_type () == SFTokenType::Identifier);

                    Str &name = static_cast<IdentifierToken *> (name_tok)
                                    ->get_val ();

                    int gb = 0;
                    size_t last_arg_idx = i + 3;
                    size_t j = i + 3;

                    Vec<Expr *> arg_list;

                    while (j < toks.get_size ())
                      {
                        Token *d = toks[j];

                        switch (d->get_type ())
                          {
                          case SFTokenType::Operator:
                            {
                              OperatorToken *t
                                  = static_cast<OperatorToken *> (d);
                              Str &tv = t->get_val ();

                              if (tv == ',' && !gb)
                                {
                                  arg_list.push_back (
                                      expr_gen (toks, last_arg_idx, j));
                                  last_arg_idx = j;
                                }

                              if (tv == ')' && !gb)
                                {
                                  if (j != last_arg_idx)
                                    {
                                      arg_list.push_back (
                                          expr_gen (toks, last_arg_idx, j));
                                    }
                                  last_arg_idx = j;
                                  goto l6;
                                }

                              if (tv == "(" || tv == "[" || tv == "{")
                                gb++;
                              else if (tv == ")" || tv == "]" || tv == "}")
                                gb--;
                            }
                            break;

                          default:
                            break;
                          }

                        j++;
                      }
                  l6:;

                    size_t body_start_idx = last_arg_idx + 1;
                    size_t body_block_end = _sf_ast_getblock_idx (
                        toks, i, _sf_ast_gettabspace (toks, i));

                    Vec<Token *> body_block;

                    for (size_t j = body_start_idx; j < body_block_end; j++)
                      body_block.push_back (toks[j]);

                    Vec<Statement *> body = stmt_gen (body_block);

                    FuncDeclStatement *fds
                        = new FuncDeclStatement (arg_list, body, name);

                    res.push_back (static_cast<Statement *> (fds));
                    SET_LINE_NUMBER (res, c);
                    i = body_block_end - 1;
                  }
                else
                  {
                    /**
                     * TODO: implement this sh*t
                     */
                  }
              }
            else if (kw == "return")
              {
                int gb = 0;
                size_t j = i + 1;

                while (j < toks.get_size ())
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *t = static_cast<OperatorToken *> (d);
                          Str &tv = t->get_val ();

                          if (tv == "(" || tv == "[" || tv == "{")
                            gb++;
                          else if (tv == ")" || tv == "]" || tv == "}")
                            gb--;
                        }
                        break;
                      case SFTokenType::Newline:
                        {
                          if (!gb)
                            {
                              goto l7;
                            }
                        }
                        break;

                      default:
                        break;
                      }

                    j++;
                  }
              l7:;

                Expr *r = nullptr;
                if (j != i + 1)
                  {
                    r = expr_gen (toks, i + 1, j);
                  }
                else /* nothing to return, so return None */
                  {
                    r = static_cast<Expr *> (new ConstantExpr (
                        static_cast<Constant *> (new NoneConstant ())));
                  }

                res.push_back (new ReturnStatement (r));
                SET_LINE_NUMBER (res, c);
                i = j;
              }
            else if (kw == "while")
              {
                /* similar logic to 'if' */
                int gb = 0;
                size_t cond_end_idx = i;

                for (size_t j = i + 1; j < toks.get_size (); j++)
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *jop
                              = static_cast<OperatorToken *> (d);

                          Str &js = jop->get_val ();

                          if (js == '(' || js == '[' || js == '{')
                            gb++;

                          if (js == ')' || js == ']' || js == '}')
                            gb--;
                        }
                        break;

                      case SFTokenType::Newline:
                        {
                          if (!gb)
                            {
                              cond_end_idx = j;
                              goto l8;
                            }
                        }
                        break;

                      default:
                        break;
                      }
                  }
              l8:;

                Expr *cond = expr_gen (toks, i + 1, cond_end_idx);
                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                // std::cout << "-----\n";
                for (size_t j = cond_end_idx + 1; j < block_end_idx; j++)
                  {
                    // toks[j]->print ();
                    body_toks.push_back (toks[j]);
                  }
                // std::cout << "-----end\n";

                Vec<Statement *> body = stmt_gen (body_toks);

                res.push_back (static_cast<Statement *> (
                    new WhileStatement (cond, body)));
                SET_LINE_NUMBER (res, c);

                i = block_end_idx - 1;
              }
            else if (kw == "repeat")
              {
                /* similar logic to 'if' and 'while' */
                int gb = 0;
                size_t cond_end_idx = i;

                for (size_t j = i + 1; j < toks.get_size (); j++)
                  {
                    Token *d = toks[j];

                    switch (d->get_type ())
                      {
                      case SFTokenType::Operator:
                        {
                          OperatorToken *jop
                              = static_cast<OperatorToken *> (d);

                          Str &js = jop->get_val ();

                          if (js == '(' || js == '[' || js == '{')
                            gb++;

                          if (js == ')' || js == ']' || js == '}')
                            gb--;
                        }
                        break;

                      case SFTokenType::Newline:
                        {
                          if (!gb)
                            {
                              cond_end_idx = j;
                              goto l9;
                            }
                        }
                        break;

                      default:
                        break;
                      }
                  }
              l9:;

                Expr *cond = expr_gen (toks, i + 1, cond_end_idx);
                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                // std::cout << "-----\n";
                for (size_t j = cond_end_idx + 1; j < block_end_idx; j++)
                  {
                    // toks[j]->print ();
                    body_toks.push_back (toks[j]);
                  }
                // std::cout << "-----end\n";

                Vec<Statement *> body = stmt_gen (body_toks);

                res.push_back (static_cast<Statement *> (
                    new RepeatStatement (cond, body)));
                SET_LINE_NUMBER (res, c);

                i = block_end_idx - 1;
              }
            else if (kw == "spawn")
              {
                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                // std::cout << "-----\n";
                for (size_t j = i + 1; j < block_end_idx; j++)
                  {
                    // toks[j]->print ();
                    body_toks.push_back (toks[j]);
                  }
                // std::cout << "-----end\n";

                Vec<Statement *> body = stmt_gen (body_toks);

                res.push_back (
                    static_cast<Statement *> (new SpawnBlock (body)));
                SET_LINE_NUMBER (res, c);

                i = block_end_idx - 1;
              }
            else if (kw == "class")
              {
                assert (i + 1 < toks.get_size ());
                Token *name = toks[i + 1];

                assert (name->get_type () == SFTokenType::Identifier);
                Str name_str
                    = static_cast<IdentifierToken *> (name)->get_val ();

                // assert (i + 2 < toks.get_size ()
                //         && toks[i + 2]->get_type () ==
                //         SFTokenType::Newline);

                Token *next_name = toks[i + 2];
                size_t block_st_idx;
                Vec<Expr *> inhs;

                if (next_name->get_type () == SFTokenType::Newline)
                  block_st_idx = i + 2;
                else if (next_name->get_type () == SFTokenType::Keyword
                         && static_cast<KeywordToken *> (next_name)->get_val ()
                                == "extends")
                  {
                    size_t l = i + 3, r = i + 4;
                    int gb = 0;

                    while (r < toks.get_size ())
                      {
                        Token *cr = toks[r];

                        if (cr->get_type () == SFTokenType::Newline && !gb)
                          {
                            inhs.push_back (expr_gen (toks, l, r));
                            break;
                          }

                        if (cr->get_type () == SFTokenType::Operator)
                          {
                            OperatorToken *ot
                                = static_cast<OperatorToken *> (cr);
                            Str &op = ot->get_val ();

                            if ((op == "(" || op == "{" || op == "[") && !gb)
                              gb++;

                            if ((op == ")" || op == "}" || op == "]") && !gb)
                              gb--;

                            if (op == "," && !gb)
                              {
                                inhs.push_back (expr_gen (toks, l, r));
                                l = r + 1;
                                r++;
                              }
                          }

                        r++;
                      }

                    block_st_idx = r;
                  }

                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> body_toks;
                for (size_t j = block_st_idx; j < block_end_idx; j++)
                  {
                    body_toks.push_back (toks[j]);
                  }

                Vec<Statement *> body = stmt_gen (body_toks);

                res.push_back (static_cast<Statement *> (
                    new ClassDeclStatement (name_str, body, inhs)));
                SET_LINE_NUMBER (res, c);

                i = block_end_idx - 1;
              }
            else if (kw == "import")
              {
                assert (i + 3 < toks.get_size ());
                Token *path = toks[i + 1];
                assert (path->get_type () == SFTokenType::String);

                Token *as_tok = toks[i + 2];
                assert (as_tok->get_type () == SFTokenType::Keyword
                        && static_cast<KeywordToken *> (as_tok)->get_val ()
                               == "as");

                Token *alias = toks[i + 3];
                assert (alias->get_type () == SFTokenType::Identifier);

                Str alias_str
                    = static_cast<IdentifierToken *> (alias)->get_val ();

                Str path_str = static_cast<StringToken *> (path)->get_val ();

                res.push_back (static_cast<Statement *> (
                    new ImportStatement (path_str, alias_str)));
                SET_LINE_NUMBER (res, c);

                i += 3;
              }
            else if (kw == "try")
              {
                TryCatchStmt *st = new TryCatchStmt ();

                size_t block_st_idx = i + 2;
                size_t block_end_idx = _sf_ast_getblock_idx (
                    toks, i, _sf_ast_gettabspace (toks, i));

                Vec<Token *> trybody_toks;
                for (size_t j = block_st_idx; j < block_end_idx; j++)
                  {
                    trybody_toks.push_back (toks[j]);
                  }

                st->get_try_body () = stmt_gen (trybody_toks);

                while (block_end_idx < toks.get_size ()
                       && (toks[block_end_idx]->get_type ()
                               == SFTokenType::Newline
                           || toks[block_end_idx]->get_type ()
                                  == SFTokenType::Tabspace))
                  block_end_idx++;

                if (block_end_idx < toks.get_size ()
                    && toks[block_end_idx]->get_type () == SFTokenType::Keyword
                    && static_cast<KeywordToken *> (toks[block_end_idx])
                               ->get_val ()
                           == "catch")
                  {
                    block_end_idx++; /* eat 'catch' */

                    size_t cc_end_idx = block_end_idx;
                    for (size_t j = block_end_idx; j < toks.get_size (); j++)
                      {
                        if (toks[j]->get_type () == SFTokenType::Newline)
                          {
                            cc_end_idx = j;
                            break;
                          }
                      }

                    if (cc_end_idx != block_end_idx)
                      {
                        st->get_cclause ()
                            = expr_gen (toks, block_end_idx, cc_end_idx);
                      }

                    size_t bei2 = _sf_ast_getblock_idx (
                        toks, cc_end_idx, _sf_ast_gettabspace (toks, i));

                    Vec<Token *> catchbody_toks;
                    for (size_t j = cc_end_idx; j < bei2; j++)
                      {
                        catchbody_toks.push_back (toks[j]);
                      }

                    st->get_catch_body () = stmt_gen (catchbody_toks);
                    i = bei2 - 1;
                  }
                else
                  i = block_end_idx - 1;

                res.push_back (st);
                SET_LINE_NUMBER (res, c);
              }
            else if (kw == "continue")
              {
                res.push_back (static_cast<Statement *> (new ContinueStmt ()));
                SET_LINE_NUMBER (res, c);
              }
            else if (kw == "break")
              {
                res.push_back (static_cast<Statement *> (new BreakStmt ()));
                SET_LINE_NUMBER (res, c);
              }
          }
          break;

        default:
          break;
        }

    end:
      i++;
    }

  return res;
}

void
_sf_ast_getline (Vec<Token *> &data, size_t st, Vec<Token *> &buf, bool rev)
{
  int gb = 0;

  if (rev)
    {
      for (int j = int (st) - 1; j >= 0; j--)
        {
          switch (data[j]->get_type ())
            {
            case SFTokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case SFTokenType::Newline:
            case SFTokenType::Tabspace:
              {
                if (!gb)
                  {
                    /* break out of loop */
                    j = -1;
                    continue;
                  }
              }
              break;

            default:
              break;
            }

          buf.push_back (data[j]);
        }
    }
  else
    {
      for (size_t j = st; j < data.get_size (); j++)
        {
          switch (data[j]->get_type ())
            {
            case SFTokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case SFTokenType::Newline:
            case SFTokenType::Tabspace:
              {
                if (!gb)
                  {
                    /* break out of loop */
                    j = -1;
                    continue;
                  }
              }
              break;

            default:
              break;
            }

          buf.push_back (data[j]);
        }
    }
}

int
_sf_ast_getline_idx (Vec<Token *> &data, size_t st, bool rev)
{
  int gb = 0;

  if (rev)
    {
      for (int j = int (st); j >= 0; j--)
        {
          switch (data[j]->get_type ())
            {
            case SFTokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case SFTokenType::Newline:
            case SFTokenType::Tabspace:
              {
                if (!gb)
                  {
                    return j;
                  }
              }
              break;

            default:
              break;
            }
        }

      return 0;
    }
  else
    {
      for (size_t j = st; j < data.get_size (); j++)
        {
          switch (data[j]->get_type ())
            {
            case SFTokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case SFTokenType::Newline:
            case SFTokenType::Tabspace:
              {
                if (!gb)
                  {
                    return int (j);
                  }
              }
              break;

            default:
              break;
            }
        }

      return data.get_size ();
    }

  return -1; /* unreachable */
}

size_t
_sf_ast_getarg_idx (Vec<Token *> &data, size_t st, int gb)
{
  size_t i = st;

  while (i < data.get_size ())
    {
      Token *c = data[i];

      switch (c->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *jop = static_cast<OperatorToken *> (data[i]);

            Str &js = jop->get_val ();

            if (js == ')' && !gb)
              return i;

            if (js == '(' || js == '[' || js == '{')
              gb++;

            if (js == ')' || js == ']' || js == '}')
              gb--;

            if (js == ',' && !gb)
              return i;
          }
          break;

        default:
          break;
        }

    end:
      i++;
    }

  return 0;
}

size_t
_sf_ast_gettabspace (Vec<Token *> &data, int st)
{
  int gb = 0;
  for (int i = st - 1; i >= 0; i--)
    {
      Token *d = data[i];

      switch (d->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *jop = static_cast<OperatorToken *> (d);

            Str &js = jop->get_val ();

            if (js == '(' || js == '[' || js == '{')
              gb++;

            if (js == ')' || js == ']' || js == '}')
              gb--;
          }
          break;

        case SFTokenType::Tabspace:
          {
            if (!gb)
              return static_cast<TabspaceToken *> (d)->get_val ();
          }
          break;
        case SFTokenType::Newline:
          {
            if (!gb)
              return 0;
          }
          break;

        default:
          break;
        }
    }

  return 0;
}

size_t
_sf_ast_getblock_idx (Vec<Token *> &data, size_t st, size_t tb)
{
  int gb = 0;
  for (size_t i = st; i < data.get_size (); i++)
    {
      Token *d = data[i];

      switch (d->get_type ())
        {
        case SFTokenType::Operator:
          {
            OperatorToken *jop = static_cast<OperatorToken *> (d);

            Str &js = jop->get_val ();

            if (js == '(' || js == '[' || js == '{')
              gb++;

            if (js == ')' || js == ']' || js == '}')
              gb--;
          }
          break;

        case SFTokenType::Newline:
          {
            if (!gb)
              {
                if (i + 1 < data.get_size ())
                  {
                    Token *nd = data[i + 1];

                    if (nd->get_type () == SFTokenType::Tabspace)
                      {
                        if (i + 2 < data.get_size ()
                            && data[i + 2]->get_type ()
                                   == SFTokenType::Newline)
                          continue;

                        if (static_cast<TabspaceToken *> (nd)->get_val ()
                            <= tb)
                          {
                            return i;
                          }
                      }
                    else if (nd->get_type () == SFTokenType::Newline)
                      continue;
                    else
                      {
                        return i;
                      }
                  }
              }
          }
          break;

        default:
          break;
        }
    }

  return data.get_size ();
}
} // namespace sf

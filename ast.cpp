#include "ast.hpp"

namespace sf
{
void _sf_ast_getline (Vec<Token *> &_Data, size_t _StartIdx,
                      Vec<Token *> &_Buf, bool _Reverse = false);

int _sf_ast_getline_idx (Vec<Token *> &_Data, size_t _StartIdx,
                         bool _Reverse = false);

/* returns the next ',' idx */
size_t _sf_ast_getarg_idx (Vec<Token *> &_Data, size_t _StartIdx, int _GB = 0);

Expr *
expr_gen (Vec<Token *> &toks, size_t st, size_t ed)
{
  Expr *res = nullptr;

  size_t i = st;
  while (i < ed)
    {
      Token *c = toks[i];

      switch (c->get_type ())
        {
        case TokenType::Operator:
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
          }
          break;

        case TokenType::Identifier:
          {
            IdentifierToken *idt = static_cast<IdentifierToken *> (c);

            if (res != nullptr)
              delete res;

            res = static_cast<Expr *> (new VariableExpr (idt->get_val ()));
          }
          break;

        case TokenType::Float:
          {
            if (res != nullptr)
              delete res;

            FloatToken *ft = static_cast<FloatToken *> (c);
            res = static_cast<Expr *> (new ConstantExpr (
                static_cast<Constant *> (new FloatConstant (ft->get_val ()))));
          }
          break;

        case TokenType::Integer:
          {
            if (res != nullptr)
              delete res;

            IntegerToken *it = static_cast<IntegerToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new IntegerConstant (it->get_val ()))));
          }
          break;

        case TokenType::String:
          {
            if (res != nullptr)
              delete res;

            StringToken *st = static_cast<StringToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new StringConstant (st->get_val ()))));
          }
          break;

        case TokenType::Boolean:
          {
            if (res != nullptr)
              delete res;

            BooleanToken *bt = static_cast<BooleanToken *> (c);
            res = static_cast<Expr *> (
                new ConstantExpr (static_cast<Constant *> (
                    new BooleanConstant (bt->get_val ()))));
          }
          break;

        case TokenType::NoneType:
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
        case TokenType::Operator:
          {
            OperatorToken *opt = static_cast<OperatorToken *> (c);
            Str &opv = opt->get_val ();

            if (opv == "=")
              {
                int nv_idx, vv_idx;
                nv_idx = _sf_ast_getline_idx (toks, i - 1, true);
                vv_idx = _sf_ast_getline_idx (toks, i + 1);

                Expr *e_name = expr_gen (toks, nv_idx, i);
                Expr *e_val = expr_gen (toks, i + 1, vv_idx);

                assert (e_name != nullptr);
                assert (e_val != nullptr);

                res.push_back (static_cast<Statement *> (
                    new VarDeclStatement (e_name, e_val)));

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
                        if (toks[j]->get_type () == TokenType::Operator
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

                i = j;
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
            case TokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case TokenType::Newline:
            case TokenType::Tabspace:
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
            case TokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case TokenType::Newline:
            case TokenType::Tabspace:
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
      for (int j = int (st) - 1; j >= 0; j--)
        {
          switch (data[j]->get_type ())
            {
            case TokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case TokenType::Newline:
            case TokenType::Tabspace:
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
            case TokenType::Operator:
              {
                OperatorToken *jop = static_cast<OperatorToken *> (data[j]);

                Str &js = jop->get_val ();

                if (js == '(' || js == '[' || js == '{')
                  gb++;

                if (js == ')' || js == ']' || js == '}')
                  gb--;
              }
              break;

            case TokenType::Newline:
            case TokenType::Tabspace:
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
        case TokenType::Operator:
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
} // namespace sf

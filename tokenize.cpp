#include "tokenize.hpp"

namespace sf
{
bool
_is_keyword (const char *k)
{
  static const char *KEYWORDS[]
      = { "if", "else", "for", "while", "in", "to", "step", "class", NULL };

  for (size_t i = 0; KEYWORDS[i] != NULL; i++)
    if (!strcmp (KEYWORDS[i], k))
      return true;

  return false;
}

Vec<Token *>
tokenize (char *p)
{
  Vec<Token *> res;

  while (*p != '\0')
    {
      char d = *p;

      if (isalpha (d) || d == '_')
        {
          Str v;

          while (isalnum (d) || d == '_')
            {
              v.push_back (d);
              d = *++p;
            }

          if (_is_keyword (v.get_internal_buffer ()))
            res.push_back (new KeywordToken (v));
          else if (v == "true" || v == "false")
            res.push_back (new BooleanToken (v == "true"));
          else if (v == "none")
            res.push_back (new NoneToken ());
          else
            res.push_back (new IdentifierToken (v));

          continue;
        }

      if (isnumber (d))
        {
          Str v;
          bool saw_dot = false;

          while (d)
            {
              if (d == '.')
                {
                  if (saw_dot)
                    break;
                  else
                    saw_dot = true;
                }

              if (!isnumber (d) && d != '.')
                break;

              v.push_back (d);
              d = *++p;
            }

          if (saw_dot)
            res.push_back (new FloatToken (atof (v.get_internal_buffer ())));
          else
            res.push_back (new IntegerToken (atoi (v.get_internal_buffer ())));

          continue;
        }

      switch (d)
        {
        case '+':
          res.push_back (new OperatorToken ("+"));
          break;
        case '-':
          res.push_back (new OperatorToken ("-"));
          break;
        case '*':
          res.push_back (new OperatorToken ("*"));
          break;
        case '/':
          res.push_back (new OperatorToken ("/"));
          break;
        case '=':
          if (*(p + 1) == '=')
            {
              res.push_back (new OperatorToken ("=="));
              p++;
            }
          else
            {
              res.push_back (new OperatorToken ("="));
            }
          break;
        case '<':
          if (*(p + 1) == '=')
            {
              res.push_back (new OperatorToken ("<="));
              p++;
            }
          else if (*(p + 1) == '<')
            {
              if (*(p + 2) == '=')
                {
                  res.push_back (new OperatorToken ("<<="));
                  p++;
                }
              else
                {
                  res.push_back (new OperatorToken ("<<"));
                }
              p++;
            }
          else
            {
              res.push_back (new OperatorToken ("<"));
            }
          break;
        case '>':
          if (*(p + 1) == '=')
            {
              res.push_back (new OperatorToken (">="));
              p++;
            }
          else if (*(p + 1) == '>')
            {
              if (*(p + 2) == '=')
                {
                  res.push_back (new OperatorToken (">>="));
                  p++;
                }
              else
                {
                  res.push_back (new OperatorToken (">>"));
                }
              p++;
            }
          else
            {
              res.push_back (new OperatorToken (">"));
            }
          break;
        case '!':
          if (*(p + 1) == '=')
            {
              res.push_back (new OperatorToken ("!="));
              p++;
            }
          else
            {
              res.push_back (new OperatorToken ("!"));
            }
          break;
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ';':
        case ',':
        case '.':
          res.push_back (new OperatorToken (d));
          break;
          // case ' ':
          // case '\t':
          // case '\n':
          // case '\r':
          //   goto end;

        case ' ':
          {
            if (res.get_size ())
              {
                if (res.back ()->get_type () == TokenType::Tabspace)
                  static_cast<TabspaceToken *> (res.back ())->get_val () += 1;

                else if (*(p - 1) == ' '
                         && res.back ()->get_type () == TokenType::Newline)
                  res.push_back (new TabspaceToken (2));
              }
          }
          break;

        case '\t':
          {
            if (res.get_size ())
              {
                if (res.back ()->get_type () == TokenType::Tabspace)
                  static_cast<TabspaceToken *> (res.back ())->get_val ()
                      += SF_STANDARD_TABSIZE;

                else if (res.back ()->get_type () == TokenType::Newline)
                  res.push_back (new TabspaceToken (SF_STANDARD_TABSIZE));
              }
          }
          break;
        case '"':
        case '\'':
          {
            char delim = d;
            Str v;
            bool esc = false;

            d = *++p;
            while (d && (d != delim || esc))
              {
                if (esc)
                  {
                    switch (d)
                      {
                      case 'n':
                        v.push_back ('\n');
                        break;
                      case 't':
                        v.push_back ('\t');
                        break;
                      case 'r':
                        v.push_back ('\r');
                        break;
                      case '\\':
                        v.push_back ('\\');
                        break;
                      case '\"':
                        v.push_back ('\"');
                        break;
                      case '\'':
                        v.push_back ('\'');
                        break;
                      case 'b':
                        v.push_back ('\b');
                        break;
                      case 'f':
                        v.push_back ('\f');
                        break;
                      default:
                        v.push_back (d);
                        break;
                      }
                    esc = false;
                  }
                else if (d == '\\')
                  {
                    esc = true;
                  }
                else
                  {
                    v.push_back (d);
                  }

                d = *++p;
              }

            if (d == delim)
              p++;

            res.push_back (new StringToken (v));
            continue;
          }

        case '\n':
          res.push_back (new NewlineToken ());
          break;

        default:
          break;
        }

    end:
      p++;
    }

  return res;
}
} // namespace sf

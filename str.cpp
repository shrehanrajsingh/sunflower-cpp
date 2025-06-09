#include "str.hpp"

namespace sf
{
Str
operator+ (const Str &lhs, const char rhs)
{
  Str res (lhs);
  res.push_back (rhs);

  return res;
}

bool
operator== (const Str &lhs, const Str &rhs)
{
  if (lhs.size () != rhs.size ())
    return false;

  for (size_t i = 0; i < lhs.size (); i++)
    if (lhs[i] != rhs[i])
      return false;

  return true;
}

bool
operator== (const Str &lhs, char *rhs)
{
  size_t rl = strlen (rhs);
  if (lhs.size () != rl)
    return false;

  size_t i = 0;
  while (*rhs)
    {
      if (i >= lhs.size ())
        return false;

      if (lhs[i] != *rhs)
        return false;

      i++;
      rhs++;
    }

  if (*rhs == '\0' && i == rl)
    return true;

  return false;
}

bool
operator== (const Str &lhs, char rhs)
{
  if (lhs.size () != 1)
    return false;

  return lhs[0] == rhs;
}

bool
operator< (const Str &lhs, const Str &rhs)
{
  size_t min_size = lhs.size () < rhs.size () ? lhs.size () : rhs.size ();

  for (size_t i = 0; i < min_size; i++)
    if (lhs[i] != rhs[i])
      return lhs[i] < rhs[i];

  return lhs.size () < rhs.size ();
}

} // namespace sf

#pragma once

#include "header.hpp"
#include "vec.hpp"
#include <cstring>
#include <ostream>
#include <utility>

namespace sf
{
class Str
{
private:
  char *v;
  size_t len;
  size_t cap;

public:
  Str () : v (new char[DEFAULT_VEC_CAP]), len (0), cap (DEFAULT_VEC_CAP)
  {
    v[0] = '\0';
  }

  ~Str () { delete[] v; };

  Str (const Str &rhs) : len (rhs.len), cap (rhs.cap)
  {
    v = new char[cap];
    memcpy (v, rhs.v, len + 1);
  }

  Str (Str &&rhs) noexcept : v (rhs.v), len (rhs.len), cap (rhs.cap)
  {
    rhs.v = nullptr;
    rhs.len = 0;
    rhs.cap = 0;
  }

  Str (char rhs)
      : v (new char[DEFAULT_VEC_CAP]), len (1), cap (DEFAULT_VEC_CAP)
  {
    v[0] = rhs;
    v[1] = '\0';
  }

  Str (const char *s) : len (0), cap (DEFAULT_VEC_CAP)
  {
    size_t s_len = strlen (s);
    if (s_len + 1 > cap)
      {
        cap = (s_len / DEFAULT_VEC_CAP + 1) * DEFAULT_VEC_CAP;
      }
    v = new char[cap];
    memcpy (v, s, s_len);
    v[s_len] = '\0';
    len = s_len;
  }

  Str &
  operator= (const Str &rhs)
  {
    if (this != &rhs)
      {
        delete[] v;
        len = rhs.len;
        cap = rhs.cap;
        v = new char[cap];
        memcpy (v, rhs.v, len + 1);
      }
    return *this;
  }

  Str &
  operator= (Str &&rhs) noexcept
  {
    if (this != &rhs)
      {
        delete[] v;
        v = rhs.v;
        len = rhs.len;
        cap = rhs.cap;
        rhs.v = nullptr;
        rhs.len = 0;
        rhs.cap = 0;
      }
    return *this;
  }

  Str &
  operator= (const char *s)
  {
    if (s == v)
      return *this;

    size_t n = strlen (s);
    size_t new_cap = (n / DEFAULT_VEC_CAP + 1) * DEFAULT_VEC_CAP;
    char *nv = new char[new_cap];
    memcpy (nv, s, n + 1);

    delete[] v;
    v = nv;
    len = n;
    cap = new_cap;
    return *this;
  }

  size_t
  size () const
  {
    return len;
  }

  void
  resize (size_t ncap)
  {
    if (ncap <= cap)
      return;

    char *vp = new char[ncap];
    memcpy (vp, v, len + 1);
    delete[] v;
    v = vp;
    cap = ncap;
  }

  void
  push_back (char c)
  {
    if (len + 1 >= cap)
      resize (2 * cap + 1);

    v[len++] = c;
    v[len] = '\0';
  }

  char
  pop_back (void)
  {
    if (!len)
      throw "Str.pop_back(): String is empty";

    char c = v[--len];
    v[len] = '\0';
    return c;
  }

  char &
  operator[] (size_t i)
  {
    return v[i];
  }

  const char &
  operator[] (size_t i) const
  {
    return v[i];
  }

  char *
  c_str () const
  {
    return strdup (v);
  }

  char *
  get_internal_buffer ()
  {
    return v;
  }

  friend std::ostream &
  operator<< (std::ostream &_Out, const Str &rhs)
  {
    _Out << rhs.v;
    return _Out;
  }

  Str
  operator+ (const Str &rhs) const
  {
    Str res;
    res.resize (len + rhs.len + 1);
    memcpy (res.v, v, len);
    memcpy (res.v + len, rhs.v, rhs.len);
    res.len = len + rhs.len;
    res.v[res.len] = '\0';
    return res;
  }

  int
  find (char c, int start = 0) const
  {
    for (int i = start; i < size (); i++)
      {
        if (v[i] == c)
          return i;
      }

    return -1;
  }

  int
  find (const Str &rhs, int start = 0) const
  {
    if (rhs.size () + start > size ())
      return -1;

    for (int i = start; i <= (int)size () - (int)rhs.size (); i++)
      {
        if (strncmp (v + i, rhs.v, rhs.size ()) == 0)
          return i;
      }

    return -1;
  }

  void
  replace (const Str &a, const Str &b)
  {
    if (a.size () == 0)
      return;

    Str res;
    for (size_t i = 0; i < size ();)
      {
        if (i + a.size () <= size () && strncmp (v + i, a.v, a.size ()) == 0)
          {
            res = res + b;
            i += a.size ();
          }
        else
          {
            res.push_back (v[i]);
            i++;
          }
      }

    *this = std::move (res);
  }

  friend Str operator+ (const Str &, const char);
  friend bool operator== (const Str &, const Str &);
  friend bool operator== (const Str &, char *);
  friend bool operator== (const Str &, char);
  friend bool operator< (const Str &, const Str &);
};

// std::ostream &
// operator<< (std::ostream &_Out, Str &rhs)
// {
//   for (auto c : rhs.v)
//     _Out << c;

//   return _Out;
// }

} // namespace sf

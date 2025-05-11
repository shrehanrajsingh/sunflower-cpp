#pragma once

#include "header.hpp"
#include "vec.hpp"

namespace sf
{
class Str
{
private:
  Vec<char> v;

public:
  Str () : v (Vec<char> ()) {};
  ~Str () {};

  Str (const Str &rhs) { v = rhs.v; }
  Str (Str &&rhs) noexcept : v (std::move (rhs.v)) {}
  Str (char &rhs) { v.push_back (rhs); }
  //   Str (const Str &rhs) { v = std::move (rhs.v); }

  Str &
  operator= (const Str &rhs)
  {
    v = rhs.v;
    return *this;
  }

  Str &
  operator= (const Str &&rhs)
  {
    v = std::move (rhs.v);
    return *this;
  }

  Str (const char *_s)
  {
    char *s = (char *)_s;
    while (*s)
      push_back (*s++);
  }

  Str &
  operator= (char *_S)
  {
    v.clear ();
    while (*_S)
      v.push_back (*_S++);

    return *this;
  }

  Str &
  operator= (const char *_S)
  {
    char *s = (char *)_S;
    v.clear ();
    while (*s)
      v.push_back (*s++);

    return *this;
  }

  size_t
  size () const
  {
    return v.get_size ();
  }

  void
  push_back (char c)
  {
    v.push_back (c);
  }

  char
  pop_back (void)
  {
    return v.pop_back ();
  }

  char &
  operator[] (size_t i)
  {
    return v[i];
  }

  const char
  operator[] (size_t i) const
  {
    return v[i];
  }

  char *
  c_str ()
  {
    char *p;
    char *s = p = new char[v.get_size () + 1];

    for (char c : v)
      *p++ = c;
    *p++ = '\0';

    return s;
  }

  char *
  get_internal_buffer ()
  {
    return v.get ();
  }

  friend std::ostream &
  operator<< (std::ostream &_Out, Str &rhs)
  {
    for (auto c : rhs.v)
      _Out << c;

    return _Out;
  }

  Str
  operator+ (const Str &rhs)
  {
    Str res (*this);

    for (size_t i = 0; i < rhs.size (); i++)
      res.push_back (rhs[i]);

    return res;
  }

  friend Str operator+ (const Str &, const char);
  friend bool operator== (const Str &, const Str &);
  friend bool operator== (const Str &, char *);
  friend bool operator== (const Str &, char);
};

// std::ostream &
// operator<< (std::ostream &_Out, Str &rhs)
// {
//   for (auto c : rhs.v)
//     _Out << c;

//   return _Out;
// }

} // namespace sf

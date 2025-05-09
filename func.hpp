#pragma once

/**
 * NOTE: Every function (native and coded) must return
 * an object with ref_count at-least 1.
 * For native functions, the returning object, if an r-value,
 * must have ref_count as >=1
 * For coded function, the return statement will constitute an
 * increment in the ref_count of the object to be returned.
 */

#include "header.hpp"
#include "str.hpp"
#include "vec.hpp"

enum class FuncType
{
  Native = 0,
  Coded = 1,
};

namespace sf
{
class Object;
class Module;
class Expr;
class Statement;

class Function
{
private:
  FuncType type;

public:
  Function () : type (FuncType::Native) {}
  Function (FuncType t) : type (t) {}

  inline FuncType
  get_type () const
  {
    return type;
  }

  virtual ~Function () {};
};

class NativeFunction : public Function
{
private:
  Vec<Str> args;
  std::function<Object *(Module *)> f;

public:
  NativeFunction () : Function (FuncType::Native) {}
  NativeFunction (std::function<Object *(Module *)> F) : f (F) {}
  NativeFunction (std::function<Object *(Module *)> F, Vec<Str> a)
      : f (F), args (a)
  {
  }
  ~NativeFunction () {};

  Object *
  call (Module *m)
  {
    assert (f != nullptr);
    return f (m);
  }

  inline Vec<Str> &
  get_args ()
  {
    return args;
  }

  inline Vec<Str>
  get_args () const
  {
    return args;
  }
};

class CodedFunction : public Function
{
private:
  Vec<Statement *> body;
  Vec<Str> args;

public:
  CodedFunction () : Function (FuncType::Coded) {}
  CodedFunction (Vec<Statement *> B, Vec<Str> a) : body (B), args (a) {}
  ~CodedFunction () {}

  inline Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  inline Vec<Statement *>
  get_body () const
  {
    return body;
  }

  inline Vec<Str> &
  get_args ()
  {
    return args;
  }

  inline Vec<Str>
  get_args () const
  {
    return args;
  }
};
} // namespace sf

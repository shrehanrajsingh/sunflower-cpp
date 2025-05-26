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
#include "memutil.hpp"
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

class Function : public memnode_t
{
private:
  FuncType type;

  bool use_va_args = false;

public:
  Function () : type (FuncType::Native) {}
  Function (FuncType t) : type (t) {}

  void
  set_va_args (bool _T)
  {
    use_va_args = _T;
  }

  inline bool
  get_va_args ()
  {
    return use_va_args;
  }

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
  NativeFunction (std::function<Object *(Module *)> F)
      : Function (FuncType::Native), f (F)
  {
  }
  NativeFunction (std::function<Object *(Module *)> F, Vec<Str> a)
      : Function (FuncType::Native), f (F), args (a)
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

  inline std::function<Object *(Module *)>
  get_f ()
  {
    return f;
  }
};

class CodedFunction : public Function
{
private:
  Vec<Statement *> body;
  Vec<Expr *> args;

public:
  CodedFunction () : Function (FuncType::Coded) {}
  CodedFunction (Vec<Statement *> B, Vec<Expr *> a)
      : Function (FuncType::Coded), body (B), args (a)
  {
  }
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

  inline Vec<Expr *> &
  get_args ()
  {
    return args;
  }

  inline Vec<Expr *>
  get_args () const
  {
    return args;
  }
};

void _sffunc_refcheck (Function *&);
} // namespace sf

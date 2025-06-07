#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "vec.hpp"

enum class ArithValTypeEnum
{
  Operator = 0,
  Val = 1,
};

namespace sf
{
class ArithValBase
{
protected:
  ArithValTypeEnum type;

public:
  ArithValBase () : type (ArithValTypeEnum::Val) {}
  ArithValBase (ArithValTypeEnum _Type) : type (_Type) {}

  ArithValTypeEnum
  get_type ()
  {
    return type;
  }

  ~ArithValBase () {}
};

class ArithValOp : public ArithValBase
{
  char op[4];

public:
  ArithValOp () : ArithValBase (ArithValTypeEnum::Operator) {}
  ArithValOp (char *_Op)
  {
    strncpy (op, _Op, sizeof (op) - 1);
    op[sizeof (op) - 1] = '\0';
  }

  inline char *
  get_op ()
  {
    return (char *)op;
  }

  ~ArithValOp () {}
};

class ArithValOperand : public ArithValBase
{
  Expr *val;

public:
  ArithValOperand () : ArithValBase (ArithValTypeEnum::Val) {}
  ArithValOperand (Expr *_Val)
      : ArithValBase (ArithValTypeEnum::Val), val (_Val)
  {
  }

  inline Expr *&
  get_val ()
  {
    return val;
  }

  inline const Expr *
  get_val () const
  {
    return val;
  }

  ~ArithValOperand () {}
};

class Arithmetic : public Expr
{
  Vec<ArithValBase *> vals;

public:
  Arithmetic () : Expr (ExprType::ExprArith) {}

  /**
   * _Vals must be in postfix form
   */
  Arithmetic (Vec<ArithValBase *> _Vals)
      : Expr (ExprType::ExprArith), vals (_Vals)
  {
  }

  inline Vec<ArithValBase *> &
  get_vals ()
  {
    return vals;
  }

  void
  print () override
  {
    std::cout << "Arithmetic Expression: " << std::endl;

    for (auto &&i : vals)
      {
        switch (i->get_type ())
          {
          case ArithValTypeEnum::Operator:
            {
              std::cout << "Operator: "
                        << static_cast<ArithValOp *> (i)->get_op ()
                        << std::endl;
            }
            break;

          case ArithValTypeEnum::Val:
            {
              std::cout << "Value: ";
              static_cast<ArithValOperand *> (i)->get_val ()->print ();
            }
            break;

          default:
            break;
          }
      }
  }

  ~Arithmetic () {}
};

} // namespace sf

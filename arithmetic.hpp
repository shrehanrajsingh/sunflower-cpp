#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "vec.hpp"

enum class AVTypeEnum
{
  Operator = 0,
  Val = 1,
};

namespace sf
{
class AVBase
{
protected:
  AVTypeEnum type;

public:
  AVBase () : type (AVTypeEnum::Val) {}
  AVBase (AVTypeEnum _Type) : type (_Type) {}

  AVTypeEnum
  get_type ()
  {
    return type;
  }

  ~AVBase () {}
};

class AVOperator : public AVBase
{
  char op[4];

public:
  AVOperator () : AVBase (AVTypeEnum::Operator) {}
  AVOperator (char *_Op)
  {
    strncpy (op, _Op, sizeof (op) - 1);
    op[sizeof (op) - 1] = '\0';
  }

  inline char *
  get_op ()
  {
    return (char *)op;
  }

  ~AVOperator () {}
};

class AVOperand : public AVBase
{
  Expr *val;

public:
  AVOperand () : AVBase (AVTypeEnum::Val) {}
  AVOperand (Expr *_Val) : AVBase (AVTypeEnum::Val), val (_Val) {}

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

  ~AVOperand () {}
};

class Arithmetic : public Expr
{
  Vec<AVBase *> vals;

public:
  Arithmetic () : Expr (ExprType::ExprArith) {}

  /**
   * _Vals must be in postfix form
   */
  Arithmetic (Vec<AVBase *> _Vals) : Expr (ExprType::ExprArith), vals (_Vals)
  {
  }

  inline Vec<AVBase *> &
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
          case AVTypeEnum::Operator:
            {
              std::cout << "Operator: "
                        << static_cast<AVOperator *> (i)->get_op ()
                        << std::endl;
            }
            break;

          case AVTypeEnum::Val:
            {
              std::cout << "Value: ";
              static_cast<AVOperand *> (i)->get_val ()->print ();
            }
            break;

          default:
            break;
          }
      }
  }

  static Arithmetic from_infix (Vec<AVBase *> &);

  ~Arithmetic () {}
};

} // namespace sf

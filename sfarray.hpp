#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "object.hpp"
#include "vec.hpp"

namespace sf
{
class ArrayObject : public Object
{
private:
  Vec<Object *> vals;

public:
  ArrayObject ();
  ArrayObject (Vec<Object *> &);
  ArrayObject (Vec<Object *> &&);

  std::string get_stdout_repr () override;

  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "Array Object: ";

    size_t d = 0;
    for (Object *&i : vals)
      {
        std::cout << "\n\t[" << d++ << "]\t";
        i->print ();
      }
  }

  inline Vec<Object *> &
  get_vals ()
  {
    return vals;
  };

  ~ArrayObject ();
};

class ArrayExpr : public Expr
{
private:
  Vec<Expr *> vals;

public:
  ArrayExpr ();
  ArrayExpr (Vec<Expr *> &);
  ArrayExpr (Vec<Expr *> &&);

  inline Vec<Expr *> &
  get_vals ()
  {
    return vals;
  };
  ~ArrayExpr ();
};
} // namespace sf

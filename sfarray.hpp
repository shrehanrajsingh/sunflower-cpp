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
    // return get_stdout_repr ();
    // std::stringstream ss;
    // ss << "<array " << this << ">";
    // return ss.str ();
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "Array Object: " << this;

    size_t d = 0;
    for (Object *&i : vals)
      {
        std::cout << "\n\t[" << d++ << "]\t";

        if (i->get_type () == ObjectType::ArrayObj)
          std::cout << "<array " << i << ">";
        else
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
  }

  void
  print () override
  {
    std::cout << "ArrayExpr: " << this << std::endl;

    for (Expr *&i : vals)
      {
        i->print ();
      }
  }

  ~ArrayExpr ();
};
} // namespace sf

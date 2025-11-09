#include "sfarray.hpp"

namespace sf
{
ArrayObject::ArrayObject () : Object (ObjectType::ArrayObj) {}
ArrayObject::ArrayObject (Vec<Object *> &v)
    : Object (ObjectType::ArrayObj), vals (v)
{
}
ArrayObject::ArrayObject (Vec<Object *> &&v)
    : Object (ObjectType::ArrayObj), vals (std::move (v))
{
}

std::string
ArrayObject::get_stdout_repr ()
{
  std::stringstream s;

  s << "[";

  if (!vals.get_size ())
    s << "]";
  for (size_t i = 0; i < vals.get_size (); i++)
    {
      s << vals[i]->get_stdout_repr_in_container ();
      if (i != vals.get_size () - 1)
        s << ", ";
      else
        s << ']';
    }

  return s.str ();
}

ArrayObject::~ArrayObject ()
{
  for (Object *&i : vals)
    {
      DR (i);
    }
}

ArrayExpr::ArrayExpr () : Expr (ExprType::ArrayExp) {}
ArrayExpr::ArrayExpr (Vec<Expr *> &v) : Expr (ExprType::ArrayExp), vals (v) {}
ArrayExpr::ArrayExpr (Vec<Expr *> &&v)
    : Expr (ExprType::ArrayExp), vals (std::move (v))
{
}

ArrayExpr::~ArrayExpr () {}
} // namespace sf

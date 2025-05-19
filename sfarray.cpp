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

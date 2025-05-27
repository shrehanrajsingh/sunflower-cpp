#include "sfdict.hpp"

namespace sf
{
DictObject::DictObject () : Object (ObjectType::DictObj) {}

DictObject::DictObject (std::map<std::string, Object *> &rhs)
    : Object (ObjectType::DictObj), vals (rhs)
{
}

DictObject::DictObject (std::map<std::string, Object *> &&rhs)
    : Object (ObjectType::DictObj), vals (std::move (rhs))
{
}

std::string
DictObject::get_stdout_repr ()
{
  std::stringstream s;
  size_t j = 0;

  s << "{";
  for (auto &&i : vals)
    s << '\'' << i.first << "': " << i.second->get_stdout_repr () << ", ";

  std::string res = s.str ();
  res.pop_back ();
  res.pop_back ();

  return res + "}";
}

DictObject::~DictObject ()
{
  for (auto &&i : vals)
    {
      DR (i.second);
    }
}

DictExpr::DictExpr () : Expr (ExprType::ExprDict) {}

DictExpr::DictExpr (std::map<Expr *, Expr *> &rhs)
    : Expr (ExprType::ExprDict), vals (rhs)
{
}

DictExpr::DictExpr (std::map<Expr *, Expr *> &&rhs)
    : Expr (ExprType::ExprDict), vals (std::move (rhs))
{
}

DictExpr::~DictExpr () {}
} // namespace sf

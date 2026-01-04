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

std::mutex dict_mutex;

std::string
DictObject::get_stdout_repr ()
{
  // std::lock_guard<std::mutex> lock (dict_mutex);
  static thread_local int recur_count = 0;

  recur_count++;

  if (recur_count > 100)
    {
      recur_count--;
      return "[...]";
    }

  std::string s = "";

  s += "{";
  for (auto &&i : vals)
    {
      auto rep = i.second->get_stdout_repr_in_container ();
      // std::cout << i.first << '\t' << rep << '\n';
      s += '\"' + i.first + "\": " + rep + ", ";
    }

  if (vals.size ())
    {
      s.pop_back (); // eat comma
      s.pop_back (); // eat space
    }

  s += "}";
  recur_count--;
  return s;
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

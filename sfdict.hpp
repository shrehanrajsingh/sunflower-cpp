#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "object.hpp"

namespace sf
{
class DictObject : public Object
{
private:
  std::map<std::string, Object *> vals;

public:
  DictObject ();
  DictObject (std::map<std::string, Object *> &);
  DictObject (std::map<std::string, Object *> &&);

  std::string get_stdout_repr () override;
  void
  print () override
  {
    std::cout << "Dict Object: " << std::endl;

    for (auto &&i : vals)
      {
        std::cout << i.first << ": ";
        i.second->print ();
        std::cout << std::endl;
      }
  }

  inline std::map<std::string, Object *> &
  get_vals ()
  {
    return vals;
  }

  ~DictObject ();
};

class DictExpr : public Expr
{
private:
  std::map<Expr *, Expr *> vals;

public:
  DictExpr ();
  DictExpr (std::map<Expr *, Expr *> &);
  DictExpr (std::map<Expr *, Expr *> &&);

  inline std::map<Expr *, Expr *> &
  get_vals ()
  {
    return vals;
  }

  void
  print () override
  {
    std::cout << "DictExpr" << std::endl;

    for (auto &&i : vals)
      {
        i.first->print ();
        i.second->print ();
      }
  }

  ~DictExpr ();
};
} // namespace sf

#pragma once

#include "const.hpp"
#include "func.hpp"
#include "header.hpp"

enum class ExprType
{
  Constant = 0,
  Variable = 1,
  VarDecl = 2,
  Function = 3,
  NoExpr, /* fallback type for default expr inits */
};

namespace sf
{
class Expr
{
private:
  ExprType type;

public:
  Expr () : type (ExprType::NoExpr) {}
  Expr (ExprType t) : type (t) {}
  ~Expr () = default;

  inline ExprType
  get_type () const
  {
    return type;
  }

  virtual void print () {};
};

class ConstantExpr : public Expr
{
private:
  Constant *c;

public:
  ConstantExpr () : Expr (ExprType::Constant) { c = nullptr; };

  ConstantExpr (Constant *v) : Expr (ExprType::Constant), c (v) {}

  ~ConstantExpr () = default;

  void
  print () override
  {
    std::cout << "ConstantExpr: ";
    (*c).print ();
    std::cout << '\n';
  }

  Constant *
  get_c ()
  {
    return c;
  }

  void
  set_c (Constant *p)
  {
    c = p;
  }
};

class VariableExpr : public Expr
{
private:
  Str name;

public:
  VariableExpr () : Expr (ExprType::Variable) {}
  VariableExpr (Str n) : Expr (ExprType::Variable), name (n) {}
  ~VariableExpr () = default;

  void
  print () override
  {
    std::cout << "VariableExpr: " << name << std::endl;
  }

  inline Str &
  get_name ()
  {
    return name;
  }
};

class VarDeclExpr : public Expr
{
private:
  Expr *name, *val;

public:
  VarDeclExpr () : Expr (ExprType::VarDecl), name (nullptr), val (nullptr) {}
  VarDeclExpr (Expr *n, Expr *i) : Expr (ExprType::VarDecl), name (n), val (i)
  {
    // std::cout << (int)name->get_type () << '\t' << (int)val->get_type ()
    //           << '\n';
  }
  ~VarDeclExpr () = default;

  void
  print () override
  {
    std::cout << "VarDeclExpr: " << name;
    if (val)
      {
        std::cout << " = ";
        val->print ();
      }
    else
      {
        std::cout << std::endl;
      }
  }

  Expr *
  get_name ()
  {
    return name;
  }

  Expr *
  get_val ()
  {
    return val;
  }

  void
  set_name (Expr *p)
  {
    name = p;
  }

  void
  set_val (Expr *p)
  {
    val = p;
  }
};

class NoExpr : public Expr
{
public:
  NoExpr () : Expr (ExprType::NoExpr) {}
  ~NoExpr () = default;

  void
  print () override
  {
    std::cout << "NoExpr" << std::endl;
  }
};

class FunctionExpr : public Expr
{
private:
  std::shared_ptr<Function> v;

public:
  FunctionExpr () : Expr (ExprType::Function) {}
  FunctionExpr (Function *V) : Expr (ExprType::Function), v (V) {}
  ~FunctionExpr () {}

  void
  print () override
  {
    std::cout << "FunctionExpr" << std::endl;
  }

  std::shared_ptr<Function>
  get_v ()
  {
    return v;
  }
};
} // namespace sf

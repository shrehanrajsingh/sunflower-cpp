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
  Conditional = 4,
  ArrayExp = 5,
  ArrayAccess = 6,
  ExprToStep = 7,
  NoExpr, /* fallback type for default expr inits */
};

enum class ConditionalType
{
  EqEq = 0,
  NEq = 1,
  LEq = 2,
  GEq = 3,
  Le = 4,
  Ge = 5,
  NCond
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

class ConditionalExpr : public Expr
{
private:
  ConditionalType cndtype;
  Expr *lval, *rval;

public:
  ConditionalExpr ()
      : Expr (ExprType::Conditional), cndtype (ConditionalType::NCond),
        lval (nullptr), rval (nullptr)
  {
  }
  ConditionalExpr (ConditionalType _Type)
      : Expr (ExprType::Conditional), cndtype (_Type), lval (nullptr),
        rval (nullptr)
  {
  }
  ConditionalExpr (ConditionalType _Type, Expr *_Lval, Expr *_Rval)
      : Expr (ExprType::Conditional), cndtype (_Type), lval (_Lval),
        rval (_Rval)
  {
  }

  inline ConditionalType
  get_cond_type () const
  {
    return cndtype;
  }

  inline Expr *&
  get_lval ()
  {
    return lval;
  }

  inline Expr *&
  get_rval ()
  {
    return rval;
  }

  void
  print () override
  {
    std::cout << "ConditionalExpr: " << std::endl;
    std::cout << "Type: ";
    switch (cndtype)
      {
      case ConditionalType::EqEq:
        std::cout << "==" << std::endl;
        break;
      case ConditionalType::NEq:
        std::cout << "!=" << std::endl;
        break;
      case ConditionalType::LEq:
        std::cout << "<=" << std::endl;
        break;
      case ConditionalType::GEq:
        std::cout << ">=" << std::endl;
        break;
      case ConditionalType::Le:
        std::cout << "<" << std::endl;
        break;
      case ConditionalType::Ge:
        std::cout << ">" << std::endl;
        break;
      case ConditionalType::NCond:
        std::cout << "No Condition" << std::endl;
        break;
      default:
        std::cout << "Unknown" << std::endl;
        break;
      }
    lval->print ();
    rval->print ();
  }

  ~ConditionalExpr () {}
};

class ArrayAccess : public Expr
{
private:
  /* type arr[idx] */
  Expr *arr;
  Expr *idx;

public:
  ArrayAccess () : Expr (ExprType::ArrayAccess) { arr = idx = nullptr; }
  ArrayAccess (Expr *_Arr, Expr *_Idx)
      : Expr (ExprType::ArrayAccess), arr (_Arr), idx (_Idx)
  {
  }

  inline Expr *&
  get_arr ()
  {
    return arr;
  }

  inline Expr *&
  get_idx ()
  {
    return idx;
  }

  ~ArrayAccess () {}
};

class ToStepClause : public Expr
{
private:
  Expr *lval;
  Expr *rval;
  Expr *step;

public:
  ToStepClause ()
      : Expr (ExprType::ExprToStep), lval (nullptr), rval (nullptr),
        step (nullptr)
  {
  }

  ToStepClause (Expr *_Lval, Expr *_Rval, Expr *_Step)
      : Expr (ExprType::ExprToStep), lval (_Lval), rval (_Rval), step (_Step)
  {
  }

  inline Expr *&
  get_lval ()
  {
    return lval;
  }
  inline Expr *&
  get_rval ()
  {
    return rval;
  }
  inline Expr *&
  get_step ()
  {
    return step;
  }

  ~ToStepClause () {}
};
} // namespace sf

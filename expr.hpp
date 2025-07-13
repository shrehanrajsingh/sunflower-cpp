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
  ExprDict = 8,
  ExprArith = 9,
  FuncCall = 10,
  DotAccess = 11,
  LogicalOr = 12,
  LogicalAnd = 13,
  LogicalNot = 14,
  BitLeftShift = 15,
  BitRightShift = 16,
  BitAnd = 17,
  BitOr = 18,
  BitNegate = 19,
  Repeat = 20,
  InlineFor = 21,
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

class FuncCallExpr : public Expr
{
private:
  Expr *name;
  Vec<Expr *> args;

public:
  FuncCallExpr () : Expr (ExprType::FuncCall) { name = nullptr; }

  FuncCallExpr (Expr *n, Vec<Expr *> a)
      : Expr (ExprType::FuncCall), name (n), args (a)
  {
  }

  ~FuncCallExpr () = default;

  void
  print () override
  {
    std::cout << "FuncCallExpr:\nName: ";
    name->print ();
    std::cout << "Args (" << args.get_size () << ")\n";

    for (size_t i = 0; i < args.get_size (); i++)
      {
        std::cout << i << '\t';
        args[i]->print ();
      }

    std::cout << std::endl;
  }

  Expr *
  get_name ()
  {
    return name;
  }

  Vec<Expr *> &
  get_args ()
  {
    return args;
  }

  void
  set_name (Expr *p)
  {
    name = p;
  }
};

class DotAccess : public Expr
{
private:
  Expr *parent;
  Expr *child;

public:
  DotAccess () : Expr (ExprType::DotAccess), parent (nullptr), child (nullptr)
  {
  }
  DotAccess (Expr *_Parent, Expr *_Child)
      : Expr (ExprType::DotAccess), parent (_Parent), child (_Child)
  {
  }

  inline Expr *&
  get_parent ()
  {
    return parent;
  }
  inline const Expr *
  get_parent () const
  {
    return parent;
  }

  inline Expr *&
  get_child ()
  {
    return child;
  }
  inline const Expr *
  get_child () const
  {
    return child;
  }

  void
  print () override
  {
    std::cout << "DotAccess\nParent: ";

    if (parent == nullptr)
      std::cout << "nullptr\n";
    else
      parent->print ();

    std::cout << "Child: ";
    if (child == nullptr)
      std::cout << "nullptr\n";
    else
      child->print ();
  }

  ~DotAccess () {}
};

class LogicalAndExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  LogicalAndExpr () : Expr (ExprType::LogicalAnd) {}
  LogicalAndExpr (Expr *_L, Expr *_R)
      : Expr (ExprType::LogicalAnd), left{ _L }, right{ _R }
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  const Expr *const &
  get_left () const
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "LogicalAnd\nVal: ";
    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr" << '\n';

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr" << '\n';
  }

  ~LogicalAndExpr () {}
};

class LogicalOrExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  LogicalOrExpr () : Expr (ExprType::LogicalOr) {}
  LogicalOrExpr (Expr *_L, Expr *_R)
      : Expr (ExprType::LogicalOr), left{ _L }, right (_R)
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  const Expr *const &
  get_left () const
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "LogicalOr\nLeft: ";
    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr" << '\n';

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr" << '\n';
  }

  ~LogicalOrExpr () {}
};

class LogicalNotExpr : public Expr
{
  Expr *val;

public:
  LogicalNotExpr () : Expr (ExprType::LogicalNot) {}
  LogicalNotExpr (Expr *_V) : Expr (ExprType::LogicalNot), val{ _V } {}

  Expr *&
  get_val ()
  {
    return val;
  }

  const Expr *const &
  get_val () const
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "LogicalNot\nVal: ";
    val->print ();
  }

  ~LogicalNotExpr () {}
};

class BitLeftShiftExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  BitLeftShiftExpr ()
      : Expr (ExprType::BitLeftShift), left{ nullptr }, right{ nullptr }
  {
  }
  BitLeftShiftExpr (Expr *_l, Expr *_r)
      : Expr (ExprType::BitLeftShift), left{ _l }, right{ _r }
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "BitLeftShiftExpr\nLeft: ";

    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr\n";

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr\n";
  }

  ~BitLeftShiftExpr () {}
};

class BitRightShiftExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  BitRightShiftExpr ()
      : Expr (ExprType::BitRightShift), left{ nullptr }, right{ nullptr }
  {
  }
  BitRightShiftExpr (Expr *_l, Expr *_r)
      : Expr (ExprType::BitRightShift), left{ _l }, right{ _r }
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "BitRightShiftExpr\nLeft: ";

    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr\n";

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr\n";
  }

  ~BitRightShiftExpr () {}
};

class BitAndExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  BitAndExpr () : Expr (ExprType::BitAnd), left{ nullptr }, right{ nullptr } {}
  BitAndExpr (Expr *_l, Expr *_r)
      : Expr (ExprType::BitAnd), left{ _l }, right{ _r }
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "BitAndExpr\nLeft: ";

    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr\n";

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr\n";
  }

  ~BitAndExpr () {}
};

class BitOrExpr : public Expr
{
  Expr *left;
  Expr *right;

public:
  BitOrExpr () : Expr (ExprType::BitOr), left{ nullptr }, right{ nullptr } {}
  BitOrExpr (Expr *_l, Expr *_r)
      : Expr (ExprType::BitOr), left{ _l }, right{ _r }
  {
  }

  Expr *&
  get_left ()
  {
    return left;
  }

  Expr *&
  get_right ()
  {
    return right;
  }

  void
  print () override
  {
    std::cout << "BitOrExpr\nLeft: ";

    if (left != nullptr)
      left->print ();
    else
      std::cout << "nullptr\n";

    std::cout << "Right: ";
    if (right != nullptr)
      right->print ();
    else
      std::cout << "nullptr\n";
  }

  ~BitOrExpr () {}
};

class BitNegateExpr : public Expr
{
  Expr *val;

public:
  BitNegateExpr () : Expr (ExprType::BitNegate), val{ nullptr } {}
  BitNegateExpr (Expr *_v) : Expr (ExprType::BitNegate), val{ _v } {}

  Expr *&
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "BitNegateExprVal: ";

    if (val != nullptr)
      val->print ();
    else
      std::cout << "nullptr\n";
  }

  ~BitNegateExpr () {}
};

class RepeatExpr : public Expr
{
  Expr *times;
  Expr *body;

public:
  RepeatExpr () : Expr (ExprType::Repeat), times{ nullptr }, body{ nullptr } {}
  RepeatExpr (Expr *_Times, Expr *_Body)
      : Expr (ExprType::Repeat), times{ _Times }, body{ _Body }
  {
  }

  inline Expr *&
  get_times ()
  {
    return times;
  }
  inline const Expr *const &
  get_times () const
  {
    return times;
  }

  inline Expr *&
  get_body ()
  {
    return body;
  }

  inline const Expr *const &
  get_body () const
  {
    return body;
  }

  void
  print () override
  {
    std::cout << "RepeatExpr: \nTimes: ";
    if (times == nullptr)
      std::cout << "nullptr\n";
    else
      times->print ();

    std::cout << "Body: ";
    if (body == nullptr)
      std::cout << "nullptr\n";
    else
      body->print ();
  }

  ~RepeatExpr () {}
};

class InlineForExpr : public Expr
{
  Expr **vars;
  size_t var_count;
  Expr *iterable;
  Expr *body;

public:
  InlineForExpr ()
      : Expr (ExprType::InlineFor), vars{ nullptr }, var_count{ 0 },
        iterable{ nullptr }, body{ nullptr }
  {
  }

  InlineForExpr (Expr **_Vars, size_t _VarCount, Expr *_Iterable, Expr *_Body)
      : Expr (ExprType::InlineFor), vars{ _Vars }, var_count{ _VarCount },
        iterable{ _Iterable }, body{ _Body }
  {
  }

  void
  print () override
  {
    std::cout << "InlineForExpr\n";
    std::cout << "Variables (" << var_count << "): \n";
    for (size_t i = 0; i < var_count; i++)
      {
        std::cout << "  Var " << i << ": ";
        if (vars[i] == nullptr)
          std::cout << "nullptr\n";
        else
          vars[i]->print ();
      }

    std::cout << "Iterable: ";
    if (iterable == nullptr)
      std::cout << "nullptr\n";
    else
      iterable->print ();

    std::cout << "Body: ";
    if (body == nullptr)
      std::cout << "nullptr\n";
    else
      body->print ();
  }

  ~InlineForExpr () {}
};
} // namespace sf

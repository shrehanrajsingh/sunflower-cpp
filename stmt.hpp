#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "str.hpp"

enum class StatementType
{
  VarDecl = 0,
  FuncCall = 1,
  IfConstruct = 2,
  NoStmt,
};

namespace sf
{
class Statement
{
private:
  StatementType type;

public:
  Statement () : type (StatementType::NoStmt) {}
  Statement (StatementType t) : type (t) {}
  ~Statement () = default;

  inline StatementType
  get_type () const
  {
    return type;
  }

  virtual void print () {};
};

class VarDeclStatement : public Statement
{
private:
  Expr *name, *value;

public:
  VarDeclStatement () : Statement (StatementType::VarDecl)
  {
    name = nullptr;
    value = nullptr;
  }

  VarDeclStatement (Expr *n, Expr *v)
      : Statement (StatementType::VarDecl), name (n), value (v)
  {
  }

  ~VarDeclStatement () = default;

  void
  print () override
  {
    std::cout << "VarDeclStatement\nName: ";
    name->print ();
    std::cout << "Value: ";
    value->print ();
  }

  Expr *
  get_name ()
  {
    return name;
  }

  Expr *
  get_value ()
  {
    return value;
  }

  void
  set_name (Expr *p)
  {
    name = p;
  }

  void
  set_value (Expr *p)
  {
    value = p;
  }
};

class FuncCallStatement : public Statement
{
private:
  Expr *name;
  Vec<Expr *> args;

public:
  FuncCallStatement () : Statement (StatementType::FuncCall)
  {
    name = nullptr;
  }

  FuncCallStatement (Expr *n, Vec<Expr *> a)
      : Statement (StatementType::FuncCall), name (n), args (a)
  {
  }

  ~FuncCallStatement () = default;

  void
  print () override
  {
    std::cout << "FuncCallStatement:\nName: ";
    name->print ();
    std::cout << "Args (" << args.get_size () << "):\n";

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

class IfConstruct : public Statement
{
private:
  Expr *cond;
  Vec<Statement *> body;
  Vec<IfConstruct *> elif_constructs;
  Vec<Statement *> else_body;

public:
  IfConstruct () : Statement (StatementType::IfConstruct), cond (nullptr) {}
  IfConstruct (Expr *_Cond)
      : Statement (StatementType::IfConstruct), cond (_Cond)
  {
  }
  IfConstruct (Expr *_Cond, Vec<Statement *> _Body,
               Vec<IfConstruct *> _ElifConstructs, Vec<Statement *> _ElseBody)
      : Statement (StatementType::IfConstruct), cond (_Cond), body (_Body),
        elif_constructs (_ElifConstructs), else_body (_ElseBody)
  {
  }
  ~IfConstruct () {}

  inline Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  inline Vec<IfConstruct *> &
  get_elifconstructs ()
  {
    return elif_constructs;
  }

  inline Vec<Statement *> &
  get_else_body ()
  {
    return else_body;
  }

  inline Expr *&
  get_cond ()
  {
    return cond;
  }

  void
  print () override
  {
    std::cout << "IfConstruct" << std::endl;
    std::cout << "Condition: ";

    if (cond != nullptr)
      cond->print ();
    else
      std::cout << "(Null)" << std::endl;

    std::cout << "Elifs: " << std::endl;
    for (auto &&i : elif_constructs)
      i->print ();

    if (else_body.get_size ())
      std::cout << "Else: " << std::endl;

    for (auto &&i : else_body)
      i->print ();
  }
};
} // namespace sf

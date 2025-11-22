#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "str.hpp"

enum class StatementType
{
  VarDecl = 0,
  FuncCall = 1,
  IfConstruct = 2,
  ForConstruct = 3,
  FuncDecl = 4,
  ReturnStmt = 5,
  WhileStmt = 6,
  ClassDeclStmt = 7,
  RepeatStmt = 8,
  ImportStmt = 9,
  TryCatchStmt = 10,
  BreakStmt = 11,
  ContinueStmt = 12,
  NoStmt,
};

namespace sf
{
class Statement
{
private:
  StatementType type;
  int line_number;

public:
  Statement () : type (StatementType::NoStmt), line_number{ 0 } {}
  Statement (StatementType t) : type (t), line_number{ 0 } {}
  virtual ~Statement () = default;

  inline StatementType
  get_type () const
  {
    return type;
  }

  inline int &
  get_line_number ()
  {
    return line_number;
  }

  inline const int
  get_line_number () const
  {
    return line_number;
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

  ~VarDeclStatement ()
  {
    if (name != nullptr)
      delete name;

    if (value != nullptr)
      delete value;
  }

  void
  print () override
  {
    std::cout << "VarDeclStatement\nName: ";
    assert (name != nullptr);
    name->print ();

    assert (value != nullptr);
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

  ~FuncCallStatement ()
  {
    if (name != nullptr)
      delete name;

    for (Expr *&i : args)
      delete i;
  }

  void
  print () override
  {
    std::cout << "FuncCallStatement:\nName: ";
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

  ~IfConstruct ()
  {
    if (cond != nullptr)
      delete cond;

    for (Statement *&i : body)
      delete i;

    for (Statement *&i : else_body)
      delete i;

    for (IfConstruct *&i : elif_constructs)
      delete i;
  }

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

    std::cout << "Body: " << get_body ().get_size () << std::endl;
    for (auto &&i : body)
      i->print ();

    std::cout << "Elifs: (" << elif_constructs.get_size () << ')' << std::endl;
    for (auto &&i : elif_constructs)
      i->print ();

    if (else_body.get_size ())
      std::cout << "Else: " << std::endl;

    for (auto &&i : else_body)
      i->print ();
  }
};

class ForConstruct : public Statement
{
private:
  Vec<Expr *> var_list;
  Expr *iterable;
  Vec<Statement *> body;

public:
  ForConstruct () : Statement (StatementType::ForConstruct), iterable (nullptr)
  {
  }
  ForConstruct (Vec<Expr *> _VarList, Expr *_Iterable, Vec<Statement *> _Body)
      : Statement (StatementType::ForConstruct), var_list (_VarList),
        iterable (_Iterable), body (_Body)
  {
  }

  inline Vec<Expr *> &
  get_var_list ()
  {
    return var_list;
  }

  inline Expr *&
  get_iterable ()
  {
    return iterable;
  }

  inline Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  void
  print () override
  {
    std::cout << "ForConstruct\nVars (" << var_list.get_size () << ')'
              << std::endl;

    for (Expr *&i : var_list)
      if (i != nullptr)
        i->print ();
      else
        std::cout << "(nullptr)" << std::endl;

    std::cout << "Iterable: " << std::endl;

    if (iterable != nullptr)
      iterable->print ();
    else
      std::cout << "(nullptr)" << std::endl;

    std::cout << "Body (" << body.get_size () << ')' << std::endl;

    for (Statement *&i : body)
      i->print ();
  }

  ~ForConstruct ()
  {
    if (iterable != nullptr)
      delete iterable;

    for (Statement *&i : body)
      delete i;

    for (Expr *&i : var_list)
      delete i;
  }
};

class FuncDeclStatement : public Statement
{
private:
  Vec<Expr *> args;
  Vec<Statement *> body;
  Str name;

public:
  FuncDeclStatement () : Statement (StatementType::FuncDecl) {}
  FuncDeclStatement (Vec<Expr *> _Args, Vec<Statement *> _Body, Str _Name)
      : Statement (StatementType::FuncDecl), args (_Args), body (_Body),
        name (_Name)
  {
  }

  inline Vec<Expr *> &
  get_args ()
  {
    return args;
  }

  inline Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  inline Str &
  get_name ()
  {
    return name;
  }

  void
  print () override
  {
    std::cout << "FuncDeclStatement\nArgs (" << args.get_size () << ')'
              << std::endl;

    for (Expr *&i : args)
      if (i != nullptr)
        i->print ();
      else
        std::cout << "(nullptr)" << std::endl;

    std::cout << "Body (" << body.get_size () << ')' << std::endl;

    for (Statement *&i : body)
      if (i != nullptr)
        i->print ();
      else
        std::cout << "(nullptr)" << std::endl;
  }

  ~FuncDeclStatement ()
  {
    /**
     * CodedFunction takes a reference
     * to both body and args from FuncDeclStatement
     * So you cannot delete the pointers
     * in body and args in both the places
     */
    for (Statement *&i : body)
      {
        if (i != nullptr)
          {
            delete i;
            i = nullptr;
          }
      }

    for (Expr *&i : args)
      {
        if (i != nullptr)
          {
            delete i;
            i = nullptr;
          }
      }
  }
};

class ReturnStatement : public Statement
{
private:
  Expr *val;

public:
  ReturnStatement () : Statement (StatementType::ReturnStmt) {}
  ReturnStatement (Expr *_Val)
      : Statement (StatementType::ReturnStmt), val (_Val)
  {
  }

  inline Expr *&
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "ReturnStatement" << std::endl;

    if (val != nullptr)
      val->print ();
    else
      std::cout << "(nullptr)" << std::endl;
  }

  ~ReturnStatement ()
  {
    if (val != nullptr)
      delete val;
  }
};

class WhileStatement : public Statement
{
private:
  Expr *cond;
  Vec<Statement *> body;

public:
  WhileStatement () : Statement (StatementType::WhileStmt) {}
  WhileStatement (Expr *_Cond)
      : Statement (StatementType::WhileStmt), cond (_Cond)
  {
  }
  WhileStatement (Expr *_Cond, Vec<Statement *> _Body)
      : Statement (StatementType::WhileStmt), cond (_Cond), body (_Body)
  {
  }

  inline Expr *&
  get_cond ()
  {
    return cond;
  }

  inline Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  void
  print () override
  {
    std::cout << "While Statement:\nCondition: ";
    if (cond)
      cond->print ();
    else
      std::cout << "nullptr" << std::endl;

    std::cout << "Body (" << body.get_size () << ')' << std::endl;
    for (Statement *&i : body)
      i->print ();
  }

  ~WhileStatement ()
  {
    if (cond != nullptr)
      delete cond;

    for (Statement *&i : body)
      delete i;
  }
};

class RepeatStatement : public Statement
{
  Expr *cond;
  Vec<Statement *> body;

public:
  RepeatStatement () : Statement (StatementType::RepeatStmt), cond{ nullptr }
  {
  }
  RepeatStatement (Expr *_Cond, Vec<Statement *> _Body)
      : Statement (StatementType::RepeatStmt), cond{ _Cond }, body{ _Body }
  {
  }

  inline Expr *&
  get_cond ()
  {
    return cond;
  }

  Vec<Statement *> &
  get_body ()
  {
    return body;
  }

  void
  print () override
  {
    std::cout << "ReturnStatement\nCond: ";

    if (cond == nullptr)
      std::cout << "nullptr\n";
    else
      cond->print ();

    std::cout << "Body (" << body.get_size () << ")\n";
    for (Statement *&i : body)
      {
        i->print ();
      }
  }

  ~RepeatStatement ()
  {
    if (cond != nullptr)
      delete cond;

    for (Statement *&i : body)
      delete i;
  }
};

class ImportStatement : public Statement
{
private:
  Str filepath;
  Str alias;

public:
  ImportStatement ()
      : Statement (StatementType::ImportStmt), filepath{ "" }, alias{ "" }
  {
  }

  ImportStatement (Str _FilePath)
      : Statement (StatementType::ImportStmt), filepath{ _FilePath },
        alias{ "" }
  {
  }

  ImportStatement (Str _FilePath, Str _Alias)
      : Statement (StatementType::ImportStmt), filepath{ _FilePath },
        alias{ _Alias }
  {
  }

  inline Str &
  get_filepath ()
  {
    return filepath;
  }

  inline Str &
  get_alias ()
  {
    return alias;
  }

  void
  print () override
  {
    std::cout << "Import Statement: path=" << filepath << ", alias=" << alias
              << std::endl;
  }

  ~ImportStatement () {}
};

class TryCatchStmt : public Statement
{
private:
  Vec<Statement *> try_body;
  Vec<Statement *> catch_body;
  Expr *catch_clause;

public:
  TryCatchStmt ()
      : Statement (StatementType::TryCatchStmt), catch_clause{ nullptr }
  {
  }

  TryCatchStmt (Expr *_CClause)
      : Statement (StatementType::TryCatchStmt), catch_clause{ _CClause }
  {
  }

  TryCatchStmt (Expr *_CClause, Vec<Statement *> _TryBody,
                Vec<Statement *> _CatchBody)
      : Statement (StatementType::TryCatchStmt), catch_clause{ _CClause },
        try_body{ _TryBody }, catch_body{ _CatchBody }
  {
  }

  inline Vec<Statement *> &
  get_try_body ()
  {
    return try_body;
  }

  inline Vec<Statement *> &
  get_catch_body ()
  {
    return catch_body;
  }

  inline Expr *&
  get_cclause ()
  {
    return catch_clause;
  }

  void
  print () override
  {
    std::cout << "TryCatchStmt:\nTry Body (" << try_body.get_size () << ")\n";
    for (Statement *&i : try_body)
      i->print ();

    std::cout << "catch_clause: ";
    if (catch_clause)
      catch_clause->print ();
    else
      std::cout << "nullptr\n";

    std::cout << "Catch Body (" << catch_body.get_size () << ")\n";
    for (Statement *&i : catch_body)
      i->print ();
  }

  ~TryCatchStmt ()
  {
    if (catch_clause != nullptr)
      delete catch_clause;

    for (Statement *&i : try_body)
      delete i;

    for (Statement *&i : catch_body)
      delete i;
  }
};

class BreakStmt : public Statement
{
public:
  BreakStmt () : Statement (StatementType::BreakStmt) {}

  void
  print () override
  {
    std::cout << "break_stmt\n";
  }

  ~BreakStmt () {}
};

class ContinueStmt : public Statement
{
public:
  ContinueStmt () : Statement (StatementType::ContinueStmt) {}

  void
  print () override
  {
    std::cout << "continue_stmt\n";
  }

  ~ContinueStmt () {}
};
} // namespace sf

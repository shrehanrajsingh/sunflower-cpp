#pragma once

#include "arithmetic.hpp"
#include "expr.hpp"
#include "header.hpp"
#include "object.hpp"
#include "sfarray.hpp"
#include "sfclass.hpp"
#include "sfdict.hpp"
#include "stmt.hpp"
#include "str.hpp"
#include "tokenize.hpp"
#include "vec.hpp"

enum class ModuleType
{
  File = 0,
  Interactive = 1,
  Class = 2,
  Function = 3
};

namespace sf
{
class Module
{
private:
  ModuleType type;
  Vec<Statement *> stmts;
  Module *parent;
  std::map<std::string, Object *> vtable;
  Object *ret;
  bool continue_exec;

public:
  Module () : type (ModuleType::File)
  {
    parent = nullptr;
    ret = nullptr;
    continue_exec = true;
  }

  Module (ModuleType t) : type (t)
  {
    parent = nullptr;
    ret = nullptr;
    continue_exec = true;
  }

  Module (ModuleType t, Vec<Statement *> &st)
      : type (t), stmts (st), parent (nullptr), ret (nullptr),
        continue_exec (true)
  {
  }

  Module (ModuleType t, Vec<Statement *> &&st)
      : type (t), stmts (std::move (st)), parent (nullptr),
        continue_exec (true)
  {
  }

  ~Module ()
  {
    if (ret != nullptr)
      DR (ret);

    for (auto i : vtable)
      {
        // std::cout << i.first << '\t' << i.second->get_ref_count () << '\n';
        DR (i.second);
      }
  }

  inline ModuleType
  get_type () const
  {
    return type;
  }

  inline Vec<Statement *> &
  get_stmts ()
  {
    return stmts;
  }

  inline std::map<std::string, Object *>
  get_vtable ()
  {
    return vtable;
  }

  inline Object *&
  get_ret ()
  {
    return ret;
  }

  inline bool &
  get_continue_exec ()
  {
    return continue_exec;
  }

  Module *
  get_parent ()
  {
    return parent;
  }

  void
  set_parent (Module *p)
  {
    /* don't delete current parent */
    parent = p;
  }

  Object *get_variable (std::string);
  void set_variable (std::string, Object *);

  bool has_variable (std::string);

  friend void mod_exec (Module &);
};

void mod_exec (Module &);
Object *expr_eval (Module &, Expr *);
} // namespace sf

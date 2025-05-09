#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "object.hpp"
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

public:
  Module () : type (ModuleType::File) { parent = nullptr; }
  Module (ModuleType t) : type (t) { parent = nullptr; }
  Module (ModuleType t, Vec<Statement *> &st)
      : type (t), stmts (st), parent (nullptr)
  {
  }
  Module (ModuleType t, Vec<Statement *> &&st)
      : type (t), stmts (std::move (st)), parent (nullptr)
  {
  }

  ~Module ()
  {
    for (auto i : vtable)
      {
        // std::cout << i.second->get_ref_count () << '\n';
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
  void set_variable (std::string, Object);
  void set_variable (std::string, Object *);

  friend void mod_exec (Module &);
};

void mod_exec (Module &);
Object *expr_eval (Module &, Expr *);
} // namespace sf

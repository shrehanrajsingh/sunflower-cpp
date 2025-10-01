#pragma once

#include "arithmetic.hpp"
#include "environment.hpp"
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

  bool continue_exec;
  bool saw_ambig;
  Object *ret;
  Object *ambig;

  Environment *env;

  Vec<Str> code_lines;
  Vec<int> backtrace;

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
        continue_exec (true), ambig (nullptr), saw_ambig (false)
  {
  }

  Module (ModuleType t, Vec<Statement *> &st, Vec<Str> &lines)
      : type (t), stmts (st), parent (nullptr), ret (nullptr),
        continue_exec (true), ambig (nullptr), saw_ambig (false),
        code_lines (lines)
  {
  }

  Module (ModuleType t, Vec<Statement *> &&st)
      : type (t), stmts (std::move (st)), parent (nullptr),
        continue_exec (true), ambig (nullptr), ret (nullptr), saw_ambig (false)
  {
  }

  Module (ModuleType t, Vec<Statement *> &&st, Vec<Str> &lines)
      : type (t), stmts (st), parent (nullptr), ret (nullptr),
        continue_exec (true), ambig (nullptr), saw_ambig (false),
        code_lines (lines)
  {
  }

  Module (ModuleType t, Vec<Statement *> &st, Vec<Str> &lines,
          Environment *_Env)
      : type (t), stmts (st), parent (nullptr), ret (nullptr),
        continue_exec (true), ambig (nullptr), saw_ambig (false),
        code_lines (lines), env (_Env)
  {
  }

  Module (ModuleType t, Vec<Statement *> &&st, Vec<Str> &lines,
          Environment *_Env)
      : type (t), stmts (st), parent (nullptr), ret (nullptr),
        continue_exec (true), ambig (nullptr), saw_ambig (false),
        code_lines (lines), env (_Env)
  {
  }

  ~Module ()
  {
    if (ret != nullptr)
      {
        DR (ret);
      }

    if (ambig != nullptr)
      {
        Object *vo = static_cast<AmbigObject *> (ambig)->get_val ();

        if (vo != nullptr)
          {
            // std::cout << vo->get_ref_count () << '\n';
            // while (vo->get_ref_count () > 0)
            DR (vo);
          }
        DR (ambig);
      }

    if (env != nullptr)
      delete env;

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

  inline Object *&
  get_ambig ()
  {
    return ambig;
  }

  inline bool &
  get_saw_ambig ()
  {
    return saw_ambig;
  }

  inline bool &
  get_continue_exec ()
  {
    return continue_exec;
  }

  inline Environment *&
  get_env ()
  {
    return env;
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

  inline Vec<Str> &
  get_code_lines ()
  {
    return code_lines;
  }

  inline const Vec<Str> &
  get_code_lines () const
  {
    return code_lines;
  }

  inline Vec<int> &
  get_backtrace ()
  {
    return backtrace;
  }

  inline const Vec<int> &
  get_backtrace () const
  {
    return backtrace;
  }

  Object *get_variable (std::string);
  void set_variable (std::string, Object *);

  bool has_variable (std::string);

  friend void mod_exec (Module &);
};

void mod_exec (Module &);
Object *expr_eval (Module &, Expr *);
} // namespace sf

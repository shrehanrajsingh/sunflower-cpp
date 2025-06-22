#pragma once

#include "expr.hpp"
#include "header.hpp"
#include "object.hpp"
#include "stmt.hpp"
#include "vec.hpp"

namespace sf
{
class Module;
class SfClass;

class ClassObject : public Object
{
private:
  Module *mod;

public:
  ClassObject ();
  ClassObject (Module *);

  inline Module *&
  get_mod ()
  {
    return mod;
  }

  inline const Module *
  get_mod () const
  {
    return mod;
  }

  std::string get_stdout_repr () override;
  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "ClassObject: mod_ref = "
              << (mod == nullptr ? "nullptr" : (void *)mod) << std::endl;
  }

  ~ClassObject ();
};

class SfClass : public Object
{
private:
  Module *mod;
  Str name;

public:
  SfClass ();
  SfClass (Str, Module *);

  inline Module *&
  get_mod ()
  {
    return mod;
  }

  inline const Module *
  get_mod () const
  {
    return mod;
  }

  inline Str &
  get_name ()
  {
    return name;
  }

  inline const Str &
  get_name () const
  {
    return name;
  }

  std::string get_stdout_repr () override;

  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "SfClass \"" << name
              << "\": mod_ref = " << (mod == nullptr ? "nullptr" : (void *)mod)
              << std::endl;
  }

  ~SfClass ();
};

class ClassDeclStatement : public Statement
{
private:
  Vec<Statement *> body;
  Str name;

public:
  ClassDeclStatement ();
  ClassDeclStatement (Str, Vec<Statement *> &);
  ClassDeclStatement (Str, Vec<Statement *> &&);

  inline Vec<Statement *>
  get_body ()
  {
    return body;
  }

  inline const Vec<Statement *> &
  get_body () const
  {
    return body;
  }

  inline Str &
  get_name ()
  {
    return name;
  }

  inline const Str &
  get_name () const
  {
    return name;
  }

  void
  print () override
  {
    std::cout << "ClassDeclStatement \"" << name << "\"\n";
    for (Statement *&i : body)
      {
        i->print ();
      }
  }

  ~ClassDeclStatement ();
};
} // namespace sf

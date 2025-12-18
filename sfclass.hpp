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
  Vec<Object *> mro;

public:
  ClassObject ();
  ClassObject (Module *);

  inline Vec<Object *> &
  get_mro ()
  {
    return mro;
  }

  inline const Vec<Object *> &
  get_mro () const
  {
    return mro;
  }

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

class ClassDeclStatement : public Statement
{
private:
  Vec<Statement *> body;
  Str name;
  Vec<Expr *> inhs;

public:
  ClassDeclStatement ();
  ClassDeclStatement (Str, Vec<Statement *> &);
  ClassDeclStatement (Str, Vec<Statement *> &&);
  ClassDeclStatement (Str, Vec<Statement *> &, Vec<Expr *> &);
  ClassDeclStatement (Str, Vec<Statement *> &&, Vec<Expr *> &&);

  inline Vec<Statement *> &
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

  inline Vec<Expr *> &
  get_inhs ()
  {
    return inhs;
  }

  inline const Vec<Expr *> &
  get_inhs () const
  {
    return inhs;
  }

  void
  print () override
  {
    std::cout << "ClassDeclStatement \"" << name << "\"\n";
    for (Statement *&i : body)
      {
        i->print ();
      }

    if (get_inhs ().get_size ())
      {
        std::cout << "Inherits (" << get_inhs ().get_size () << ")\n";

        for (Expr *&i : get_inhs ())
          i->print ();
      }
  }

  ~ClassDeclStatement ();
};

class SfClass : public Object
{
private:
  Module *mod = nullptr;
  Str name;
  Vec<Object *> inhs;
  ClassDeclStatement *cds = nullptr;

public:
  SfClass ();
  SfClass (Str, Module *);
  SfClass (Str, Module *, ClassDeclStatement *);

  inline Module *&
  get_mod ()
  {
    return mod;
  }

  inline ClassDeclStatement *&
  get_cds ()
  {
    return cds;
  }

  inline const ClassDeclStatement *
  get_cds () const
  {
    return cds;
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

  inline Vec<Object *> &
  get_inhs ()
  {
    return inhs;
  }

  inline const Vec<Object *> &
  get_inhs () const
  {
    return inhs;
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
} // namespace sf

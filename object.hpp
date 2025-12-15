#pragma once

#include "const.hpp"
#include "expr.hpp"
#include "func.hpp"
#include "header.hpp"
#include "memutil.hpp"
#include "sr.hpp"
#include "str.hpp"
#include "vec.hpp"

/* check */
#define R(X) sf::_sfobj_refcheck ((X))
#define R__func(X) sf::_sffunc_refcheck ((X))

/* increase and check */
#define IR(X)                                                                 \
  do                                                                          \
    {                                                                         \
      _sfobj_passownership (X);                                               \
      I (X);                                                                  \
      R (X);                                                                  \
    }                                                                         \
  while (0);

/* decrease and check */
#define DR(X)                                                                 \
  do                                                                          \
    {                                                                         \
      _sfobj_removeownership (X);                                             \
      D (X);                                                                  \
      R (X);                                                                  \
    }                                                                         \
  while (0);

#define IR__func(X)                                                           \
  do                                                                          \
    {                                                                         \
      I (X);                                                                  \
      R__func (X);                                                            \
    }                                                                         \
  while (0);

#define DR__func(X)                                                           \
  do                                                                          \
    {                                                                         \
      D (X);                                                                  \
      R__func (X);                                                            \
    }                                                                         \
  while (0);

enum class ObjectType
{
  Constant = 0,
  FuncObject = 1,
  ArrayObj = 2,
  DictObj = 3,
  SfClass = 4,
  ClassObj = 5,
  AmbigObject = 6,
  ModuleObject = 7,
  NoObject,
};

namespace sf
{
void _sfobj_refcheck (Object *&);
bool _sfobj_isfalse (Module &, Object *&);
bool _sfobj_cmp (Object *&, Object *&, ConditionalType);
void _sfobj_passownership (Object *&);
void _sfobj_removeownership (Object *&);
bool _sfobj_isiterable (Object *&);
bool _sfobj_iscallable (Module &, Object *&);

class Object : public memnode_t, public StdoutRepr
{
private:
  ObjectType type;
  Object *self_arg;

public:
  Object () : type (ObjectType::NoObject), memnode_t (), self_arg (nullptr) {};
  Object (ObjectType t) : type (t), memnode_t (), self_arg (nullptr) {}
  virtual ~Object ()
  {
    if (self_arg != nullptr)
      {
        DR (self_arg);
        self_arg = nullptr;
      }
  }

  virtual std::string
  get_stdout_repr ()
  {
    return "<class internal::Object>";
  }

  virtual std::string
  get_stdout_repr_in_container ()
  {
    return get_stdout_repr ();
  }

  inline ObjectType
  get_type () const
  {
    return type;
  }

  inline Object *&
  get_self_arg ()
  {
    return self_arg;
  }

  inline const Object *
  get_self_arg () const
  {
    return self_arg;
  }

  virtual void print () {};
};

class ConstantObject : public Object
{
private:
  std::shared_ptr<Constant> c;

public:
  ConstantObject () : Object (ObjectType::Constant) { c = nullptr; }

  ConstantObject (Constant *d)
      : Object (ObjectType::Constant), c (std::shared_ptr<Constant> (d))
  {
  }

  ~ConstantObject () = default;

  std::string get_stdout_repr () override;
  std::string get_stdout_repr_in_container () override;

  void
  print () override
  {
    std::cout << "ConstantObject: ";
    c->print ();
  }

  std::shared_ptr<Constant>
  get_c ()
  {
    return c;
  }

  void
  set_c (std::shared_ptr<Constant> d)
  {
    c = d;
  }
};

class NoObj : public Object
{
public:
  NoObj () : Object (ObjectType::NoObject) {};
  ~NoObj () = default;

  std::string get_stdout_repr () override;
  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "NoObj" << std::endl;
  }
};

class FunctionObject : public Object
{
private:
  Function *v;

public:
  FunctionObject () : Object (ObjectType::FuncObject), v (nullptr) {}
  FunctionObject (Function *V) : Object (ObjectType::FuncObject), v (V) {}

  std::string get_stdout_repr () override;
  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  void
  print () override
  {
    std::cout << "FunctionObject" << std::endl;
  }

  Function *&
  get_v ()
  {
    return v;
  }

  ~FunctionObject ()
  {
    if (v != nullptr)
      delete v;
  }
};

class AmbigObject : public Object
{
private:
  Object *val;

public:
  AmbigObject () : Object (ObjectType::AmbigObject), val{ nullptr } {}
  AmbigObject (Object *_Val) : Object (ObjectType::AmbigObject), val{ _Val } {}

  std::string get_stdout_repr () override;
  std::string
  get_stdout_repr_in_container () override
  {
    return get_stdout_repr ();
  }

  inline Object *&
  get_val ()
  {
    return val;
  }

  inline Object *const &
  get_val () const
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "AmbigObject";
    if (val != nullptr)
      {
        std::cout << " containing: ";
        val->print ();
      }
    else
      {
        std::cout << " (empty)" << std::endl;
      }
  }

  ~AmbigObject () {}
};

class ModuleObject : public Object
{
private:
  Module *mod;

public:
  ModuleObject () : Object (ObjectType::ModuleObject) {}
  ModuleObject (Module *_Mod) : Object (ObjectType::ModuleObject), mod{ _Mod }
  {
  }

  inline Module *&
  get_mod ()
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
    std::cout << "ModuleObject\n";
  }

  ~ModuleObject ();
};

#define OBJ_IS_INT(X)                                                         \
  ((X)->get_type () == ObjectType::Constant                                   \
   && static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()       \
          == ConstantType::Integer)

#define OBJ_IS_FLOAT(X)                                                       \
  ((X)->get_type () == ObjectType::Constant                                   \
   && static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()       \
          == ConstantType::Float)

#define OBJ_IS_NUMBER(X)                                                      \
  ((X)->get_type () == ObjectType::Constant                                   \
   && (static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()      \
           == ConstantType::Float                                             \
       || static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()   \
              == ConstantType::Integer))

#define OBJ_IS_STR(X)                                                         \
  ((X)->get_type () == ObjectType::Constant                                   \
   && static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()       \
          == ConstantType::String)

#define OBJ_IS_BOOL(X)                                                        \
  ((X)->get_type () == ObjectType::Constant                                   \
   && static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()       \
          == ConstantType::Boolean)

#define OBJ_IS_AMBIG(X) ((X)->get_type () == ObjectType::AmbigObject)

#define AMBIG_CHECK(X, Y)                                                     \
  if (OBJ_IS_AMBIG ((X)))                                                     \
    {                                                                         \
      Y;                                                                      \
      goto ambig_test;                                                        \
    }
} // namespace sf

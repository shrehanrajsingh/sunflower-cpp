#pragma once

#include "const.hpp"
#include "expr.hpp"
#include "func.hpp"
#include "header.hpp"
#include "memutil.hpp"
#include "str.hpp"
#include "vec.hpp"

/* check */
#define R(X) sf::_sfobj_refcheck ((X))
#define R__func(X) sf::_sffunc_refcheck ((X))

/* increase and check */
#define IR(X)                                                                 \
  do                                                                          \
    {                                                                         \
      I (X);                                                                  \
      R (X);                                                                  \
    }                                                                         \
  while (0);

/* decrease and check */
#define DR(X)                                                                 \
  do                                                                          \
    {                                                                         \
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
  NoObject,
};

namespace sf
{
class Object : public memnode_t
{
private:
  ObjectType type;

public:
  Object () : type (ObjectType::NoObject), memnode_t () {};
  Object (ObjectType t) : type (t), memnode_t () {}
  virtual ~Object () {};

  inline ObjectType
  get_type () const
  {
    return type;
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

  void
  print () override
  {
    std::cout << "FunctionObject" << std::endl;
  }

  Function *
  get_v ()
  {
    return v;
  }

  ~FunctionObject () {}
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
       && static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()   \
              == ConstantType::Float                                          \
   || static_cast<ConstantObject *> ((X))->get_c ().get ()->get_type ()       \
          == ConstantType::Integer)

void _sfobj_refcheck (Object *&);
bool _sfobj_isfalse (Module &, Object *&);
bool _sfobj_cmp (Object *&, Object *&, ConditionalType);

} // namespace sf

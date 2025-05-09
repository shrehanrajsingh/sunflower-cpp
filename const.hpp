#pragma once

#include "header.hpp"
#include "str.hpp"

enum class ConstantType
{
  Integer = 0,
  Float = 1,
  String = 2,
  Boolean = 3,
  NoneType = 4, /* let this be the default constant type for no init classes */
};

namespace sf
{
class Constant
{
private:
  ConstantType type;

public:
  Constant () : type (ConstantType::NoneType) {}
  Constant (ConstantType t) : type (t) {}
  virtual ~Constant () {};

  inline ConstantType
  get_type () const
  {
    return type;
  }

  virtual void print () {};
};

class IntegerConstant : public Constant
{
private:
  int value;

public:
  IntegerConstant () : Constant (ConstantType::Integer), value (0) {}
  IntegerConstant (int val) : Constant (ConstantType::Integer), value (val) {}

  int
  get_value () const
  {
    return value;
  }
  void
  set_value (int val)
  {
    value = val;
  }

  void
  print () override
  {
    std::cout << value;
  }
};

class FloatConstant : public Constant
{
private:
  float value;

public:
  FloatConstant () : Constant (ConstantType::Float), value (0.0f) {}
  FloatConstant (float val) : Constant (ConstantType::Float), value (val) {}

  float
  get_value () const
  {
    return value;
  }
  void
  set_value (float val)
  {
    value = val;
  }

  void
  print () override
  {
    std::cout << value;
  }
};

class StringConstant : public Constant
{
private:
  Str value;

public:
  StringConstant () : Constant (ConstantType::String), value ("") {}
  StringConstant (Str val) : Constant (ConstantType::String), value (val) {}

  inline Str &
  get_value ()
  {
    return value;
  }

  void
  set_value (Str &val)
  {
    value = val;
  }

  void
  print () override
  {
    std::cout << "\"" << value << "\"";
  }
};

class BooleanConstant : public Constant
{
private:
  bool value;

public:
  BooleanConstant () : Constant (ConstantType::Boolean), value (false) {}
  BooleanConstant (bool val) : Constant (ConstantType::Boolean), value (val) {}

  bool
  get_value () const
  {
    return value;
  }
  void
  set_value (bool val)
  {
    value = val;
  }

  void
  print () override
  {
    std::cout << (value ? "true" : "false");
  }
};

class NoneConstant : public Constant
{
public:
  NoneConstant () : Constant (ConstantType::NoneType) {}

  void
  print () override
  {
    std::cout << "None";
  }
};
} // namespace sf

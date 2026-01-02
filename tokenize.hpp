#pragma once

#include "header.hpp"
#include "str.hpp"

enum class SFTokenType
{
  Identifier = 0,
  Operator = 1,
  Tabspace = 2,
  Newline = 3,
  String = 4,
  Integer = 5,
  Float = 6,
  Keyword = 7,
  Boolean = 8,
  NoneType = 9,
  EndOfFile
};

namespace sf
{
class Token
{
  SFTokenType type;
  int line_number;
  int line_col;

public:
  Token () : line_number{ 0 }, line_col{ 0 } { type = SFTokenType::EndOfFile; }
  Token (SFTokenType t) : type (t), line_number{ 0 }, line_col{ 0 } {}

  inline SFTokenType
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

  inline int &
  get_line_col ()
  {
    return line_col;
  }

  inline const int
  get_line_col () const
  {
    return line_col;
  }

  void
  print_pos ()
  {
    std::cout << "(ln: " << line_number << ", lc: " << line_col << ")\n";
  }

  virtual void print () {};

  virtual ~Token () {}
};

class IdentifierToken : public Token
{
private:
  Str val;

public:
  IdentifierToken (Str v) : Token (SFTokenType::Identifier), val (v) {};
  ~IdentifierToken () = default;

  inline Str &
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Identifier: " << val << std::endl;
  }
};

class OperatorToken : public Token
{
private:
  Str val;

public:
  OperatorToken (Str v) : Token (SFTokenType::Operator), val (v) {};
  ~OperatorToken () = default;

  inline Str &
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Operator: " << val << std::endl;
  }
};

class TabspaceToken : public Token
{
private:
  size_t val;

public:
  TabspaceToken (size_t v) : Token (SFTokenType::Tabspace), val (v) {};
  ~TabspaceToken () = default;

  inline size_t &
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Tabspace: " << val << std::endl;
  }
};

class NewlineToken : public Token
{
public:
  NewlineToken () : Token (SFTokenType::Newline) {};
  ~NewlineToken () = default;

  void
  print () override
  {
    std::cout << "Newline" << std::endl;
  }
};

class StringToken : public Token
{
private:
  Str val;

public:
  StringToken (Str v) : Token (SFTokenType::String), val (v) {};
  ~StringToken () = default;

  inline Str &
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "String: " << val << std::endl;
  }
};

class IntegerToken : public Token
{
private:
  int val;

public:
  IntegerToken (int v) : Token (SFTokenType::Integer), val (v) {};
  ~IntegerToken () = default;

  inline int
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Integer: " << val << std::endl;
  }
};

class FloatToken : public Token
{
private:
  float val;

public:
  FloatToken (float v) : Token (SFTokenType::Float), val (v) {};
  ~FloatToken () = default;

  inline float
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Float: " << val << std::endl;
  }
};

class KeywordToken : public Token
{
private:
  Str val;

public:
  KeywordToken (Str v) : Token (SFTokenType::Keyword), val (v) {};
  ~KeywordToken () = default;

  inline Str &
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Keyword: " << val << std::endl;
  }
};

class BooleanToken : public Token
{
private:
  bool val;

public:
  BooleanToken (bool v) : Token (SFTokenType::Boolean), val (v) {};
  ~BooleanToken () = default;

  inline bool
  get_val ()
  {
    return val;
  }

  void
  print () override
  {
    std::cout << "Boolean: " << val << std::endl;
  }
};

class NoneToken : public Token
{
public:
  NoneToken () : Token (SFTokenType::NoneType) {}
  ~NoneToken () = default;
};

class EndOfFileToken : public Token
{
public:
  EndOfFileToken () : Token (SFTokenType::EndOfFile) {}
  ~EndOfFileToken () = default;
};

Vec<Token *> tokenize (char *);
} // namespace sf

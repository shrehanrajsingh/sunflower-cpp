#pragma once

#include "header.hpp"
#include "str.hpp"

enum class TokenType
{
  Identifier = 0,
  Operator = 1,
  Tabspace = 2,
  Newline = 3,
  String = 4,
  Integer = 5,
  Float = 6,
  Keyword = 7,
  EndOfFile
};

namespace sf
{
class Token
{
  TokenType type;

public:
  Token () { type = TokenType::EndOfFile; }
  Token (TokenType t) : type (t) {}

  inline TokenType
  get_type () const
  {
    return type;
  }

  virtual void print () {};

  ~Token () {}
};

class IdentifierToken : public Token
{
private:
  Str val;

public:
  IdentifierToken (Str v) : Token (TokenType::Identifier), val (v) {};
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
  OperatorToken (Str v) : Token (TokenType::Operator), val (v) {};
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
  Str val;

public:
  TabspaceToken (Str v) : Token (TokenType::Tabspace), val (v) {};
  ~TabspaceToken () = default;

  inline Str &
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
  NewlineToken () : Token (TokenType::Newline) {};
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
  StringToken (Str v) : Token (TokenType::String), val (v) {};
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
  IntegerToken (int v) : Token (TokenType::Integer), val (v) {};
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
  FloatToken (float v) : Token (TokenType::Float), val (v) {};
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
  KeywordToken (Str v) : Token (TokenType::Keyword), val (v) {};
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

class EndOfFileToken : public Token
{
public:
  EndOfFileToken () : Token (TokenType::EndOfFile) {}
  ~EndOfFileToken () = default;
};
} // namespace sf

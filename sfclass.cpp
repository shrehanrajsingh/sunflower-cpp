#include "sfclass.hpp"
#include "module.hpp"

namespace sf
{
ClassDeclStatement::ClassDeclStatement ()
    : Statement (StatementType::ClassDeclStmt)
{
}

ClassDeclStatement::ClassDeclStatement (Str n, Vec<Statement *> &b)
    : Statement (StatementType::ClassDeclStmt), body (b), name (n)
{
}

ClassDeclStatement::ClassDeclStatement (Str n, Vec<Statement *> &&b)
    : Statement (StatementType::ClassDeclStmt), body (std::move (b)), name (n)
{
}

ClassDeclStatement::~ClassDeclStatement () {}

ClassObject::ClassObject () : Object (ObjectType::ClassObj), mod (nullptr) {}

ClassObject::ClassObject (Module *m) : Object (ObjectType::ClassObj), mod (m)
{
}

std::string
ClassObject::get_stdout_repr ()
{
  return (std::stringstream{} << "<class object " << this << ">").str ();
}

ClassObject::~ClassObject ()
{
  if (mod != nullptr)
    delete mod;
}

SfClass::SfClass () : Object (ObjectType::SfClass), mod (nullptr) {}

std::string
SfClass::get_stdout_repr ()
{
  return (std::stringstream{} << "<class " << this << ">").str ();
}

SfClass::SfClass (Str n, Module *m)
    : Object (ObjectType::SfClass), mod (m), name (n)
{
}

SfClass::~SfClass ()
{
  if (mod != nullptr)
    delete mod;
}
} // namespace sf

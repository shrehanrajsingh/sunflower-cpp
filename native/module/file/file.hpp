#pragma once

#include "../../../header.hpp"
#include "../../../module.hpp"

namespace sf
{
namespace native_mod
{
namespace File
{
class FileHandle
{
private:
  size_t id;
  std::fstream fs;
  Str perms;

public:
  FileHandle () : id{ 0 } {}
  FileHandle (size_t _Id) : id{ _Id } {}
  FileHandle (size_t _Id, std::fstream _Fs, Str _Perms)
      : id{ _Id }, fs{ std::move (_Fs) }, perms{ _Perms }
  {
  }

  inline size_t &
  get_id ()
  {
    return id;
  }

  inline std::fstream &
  get_fs ()
  {
    return fs;
  }

  inline Str &
  get_perms ()
  {
    return perms;
  }

  ~FileHandle ()
  {
    if (fs.is_open ())
      fs.close ();
  }
};

SF_API Object *open (Module *);
SF_API Object *read (Module *);

SF_API void destroy ();
} // namespace File
} // namespace native_mod
} // namespace sf

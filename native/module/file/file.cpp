#include "file.hpp"

namespace sf
{
namespace native_mod
{
namespace File
{
static std::map<size_t, FileHandle *> filemap;
static size_t fmap_id = 0;

SF_API Object *
open (Module *mod)
{
  Object *o_fname = mod->get_variable ("filename");
  Object *o_perms = mod->get_variable ("perms");

  assert (OBJ_IS_STR (o_fname) && "file name must be a string");
  assert (OBJ_IS_STR (o_perms) && "permissions must be a string");

  Str &fname = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (o_fname)->get_c ().get ())
                   ->get_value ();

  Str &perms = static_cast<StringConstant *> (
                   static_cast<ConstantObject *> (o_perms)->get_c ().get ())
                   ->get_value ();

  size_t fmid = ++fmap_id;
  while (filemap.count (fmid))
    fmid++;

  std::ios_base::openmode mode = static_cast<std::ios_base::openmode> (0);

  if (perms.find ('r') != -1)
    mode |= std::ios::in;

  if (perms.find ('w') != -1)
    mode |= std::ios::out;

  if (perms.find ('b') != -1)
    mode |= std::ios::binary;

  if (perms.find ('a') != -1)
    mode |= std::ios::app;

  std::fstream fs (fname.get_internal_buffer (), mode);

  if (!fs)
    {
      /* 0 is returned when file cannot be opened */
      Object *r = static_cast<Object *> (new ConstantObject (
          static_cast<Constant *> (new IntegerConstant (0))));

      IR (r);
      return r;
    }

  FileHandle *fh = new FileHandle (fmid, std::move (fs), perms);
  filemap[fmid] = fh;

  Object *r
      = static_cast<Object *> (new ConstantObject (static_cast<Constant *> (
          new IntegerConstant (static_cast<int> (fmid)))));

  // r->print ();

  IR (r);
  return r;
}

SF_API Object *
read (Module *mod)
{
  Object *fileid = mod->get_variable ("fileid");

  assert (OBJ_IS_INT (fileid) && "File id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (fileid)->get_c ().get ())
          ->get_value ());

  if (!filemap.count (id))
    {
      std::cerr << "File with id " << id << " does not exist." << std::endl;
      exit (-1);
    }

  FileHandle *fh = filemap[id];
  std::fstream &fs = fh->get_fs ();

  assert (fs.is_open () && "File has been closed");
  std::string line;
  std::string full;

  while (std::getline (fs, line))
    full += line + '\n';

  Object *r = static_cast<Object *> (new ConstantObject (
      static_cast<Constant *> (new StringConstant (full.c_str ()))));

  IR (r);
  return r;
}

SF_API Object *
write (Module *mod)
{
  Object *fileid = mod->get_variable ("fileid");
  Object *fstr = mod->get_variable ("str");

  assert (OBJ_IS_INT (fileid) && "File id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (fileid)->get_c ().get ())
          ->get_value ());

  if (!filemap.count (id))
    {
      std::cerr << "File with id " << id << " does not exist." << std::endl;
      exit (-1);
    }

  FileHandle *fh = filemap[id];
  std::fstream &fs = fh->get_fs ();

  assert (fs.is_open () && "File has been closed");

  Str &vl = static_cast<StringConstant *> (
                static_cast<ConstantObject *> (fstr)->get_c ().get ())
                ->get_value ();

  fs << vl.get_internal_buffer ();

  Object *r = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (r);
  return r;
}

SF_API Object *
close (Module *mod)
{
  Object *fileid = mod->get_variable ("fileid");

  assert (OBJ_IS_INT (fileid) && "File id must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (fileid)->get_c ().get ())
          ->get_value ());

  if (!filemap.count (id))
    {
      std::cerr << "File with id " << id << " does not exist." << std::endl;
      exit (-1);
    }

  FileHandle *fh = filemap[id];
  std::fstream &fs = fh->get_fs ();

  assert (fs.is_open () && "File has already been closed");
  fs.close ();

  Object *r = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (r);
  return r;
}

SF_API Object *
seek_read (Module *mod)
{
  Object *fileid = mod->get_variable ("fileid");
  Object *offset = mod->get_variable ("offset");
  Object *whence = mod->get_variable ("whence");

  assert (OBJ_IS_INT (fileid) && "File descriptor must be an integer");
  assert (OBJ_IS_INT (offset) && "offset must be an integer");
  assert (OBJ_IS_INT (whence) && "whence descriptor must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (fileid)->get_c ().get ())
          ->get_value ());

  std::streamoff v_offset = static_cast<std::streamoff> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (offset)->get_c ().get ())
          ->get_value ());

  /**
   * 0: SEEK_BEG
   * 1: SEEK_CUR
   * 2: SEEK_END
   */
  int v_whence = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (whence)->get_c ().get ())
                     ->get_value ();

  std::ios::seekdir pos = v_whence == 0   ? std::ios::beg
                          : v_whence == 1 ? std::ios::cur
                          : v_whence == 2 ? std::ios::end
                                          : std::ios::beg /* default */;

  if (!filemap.count (id))
    {
      std::cerr << "File with id " << id << " does not exist." << std::endl;
      exit (-1);
    }

  FileHandle *fh = filemap[id];
  std::fstream &fs = fh->get_fs ();

  assert (fs.is_open () && "File has been closed");
  fs.clear ();
  fs.seekg (v_offset, pos);

  Object *r = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (r);
  return r;
}

SF_API Object *
seek_write (Module *mod)
{
  Object *fileid = mod->get_variable ("fileid");
  Object *offset = mod->get_variable ("offset");
  Object *whence = mod->get_variable ("whence");

  assert (OBJ_IS_INT (fileid) && "File descriptor must be an integer");
  assert (OBJ_IS_INT (offset) && "offset must be an integer");
  assert (OBJ_IS_INT (whence) && "whence descriptor must be an integer");

  size_t id = static_cast<size_t> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (fileid)->get_c ().get ())
          ->get_value ());

  std::streamoff v_offset = static_cast<std::streamoff> (
      static_cast<IntegerConstant *> (
          static_cast<ConstantObject *> (offset)->get_c ().get ())
          ->get_value ());

  /**
   * 0: SEEK_BEG
   * 1: SEEK_CUR
   * 2: SEEK_END
   */
  int v_whence = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (whence)->get_c ().get ())
                     ->get_value ();

  std::ios::seekdir pos = v_whence == 0   ? std::ios::beg
                          : v_whence == 1 ? std::ios::cur
                          : v_whence == 2 ? std::ios::end
                                          : std::ios::beg /* default */;

  if (!filemap.count (id))
    {
      std::cerr << "File with id " << id << " does not exist." << std::endl;
      exit (-1);
    }

  FileHandle *fh = filemap[id];
  std::fstream &fs = fh->get_fs ();

  assert (fs.is_open () && "File has been closed");
  fs.clear ();
  fs.seekp (v_offset, pos);

  Object *r = static_cast<Object *> (
      new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

  IR (r);
  return r;
}

SF_API void
destroy ()
{
  for (auto &&i : filemap)
    {
      if (i.second != nullptr)
        delete i.second;
    }
}
} // namespace File
} // namespace native_mod
} // namespace sf

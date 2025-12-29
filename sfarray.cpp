#include "sfarray.hpp"

namespace sf
{
ArrayObject::ArrayObject () : Object (ObjectType::ArrayObj) {}
ArrayObject::ArrayObject (Vec<Object *> &v)
    : Object (ObjectType::ArrayObj), vals (v)
{
}
ArrayObject::ArrayObject (Vec<Object *> &&v)
    : Object (ObjectType::ArrayObj), vals (std::move (v))
{
}

std::mutex array_mutex;

std::string
ArrayObject::get_stdout_repr ()
{
  // std::lock_guard<std::mutex> lock (array_mutex);
  std::string s = "";

  s += "[";

  if (!vals.get_size ())
    s += "]";
  for (size_t i = 0; i < vals.get_size (); i++)
    {
      // std::cout << "entry: " << vals[i]->get_stdout_repr_in_container ()
      //           << std::endl;
      s += vals[i]->get_stdout_repr_in_container ();
      // std::string p = vals[i]->get_stdout_repr_in_container ();

      // std::cout << p.size () << '\t' << p[0] << p[1] << '\n';

      if (i != vals.get_size () - 1)
        s += ", ";
      else
        s += ']';

      // std::cout << s << '\n';
    }

  return s;
}

ArrayObject::~ArrayObject ()
{
  for (Object *&i : vals)
    {
      DR (i);
    }
}

ArrayExpr::ArrayExpr () : Expr (ExprType::ArrayExp) {}
ArrayExpr::ArrayExpr (Vec<Expr *> &v) : Expr (ExprType::ArrayExp), vals (v) {}
ArrayExpr::ArrayExpr (Vec<Expr *> &&v)
    : Expr (ExprType::ArrayExp), vals (std::move (v))
{
}

ArrayExpr::~ArrayExpr () {}
} // namespace sf

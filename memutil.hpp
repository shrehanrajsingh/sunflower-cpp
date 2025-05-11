#pragma once

/**
 * utility class(es) for reference counting
 * of objects
 */

#include "header.hpp"

/* increase */
#define I(X) ++(*(X))

/* decrease */
#define D(X) --(*(X))

#define TC(X)                                                                 \
  try                                                                         \
    {                                                                         \
      X;                                                                      \
    }                                                                         \
  catch (const char *__E)                                                     \
    {                                                                         \
      std::cerr << __LINE__ << '\t' << __E << std::endl;                      \
    }

namespace sf
{
class _MemNode
{
private:
  int ref_count = 0;

public:
  _MemNode () : ref_count (0) {};
  ~_MemNode () = default;

  /* all these methods change ref_count */
  virtual _MemNode &operator-- ();
  virtual _MemNode operator-- (int);
  virtual _MemNode &operator++ ();
  virtual _MemNode operator++ (int);

  /* explicit method to change ref_count */
  void set_ref_count (int);

  inline int &
  get_ref_count ()
  {
    return ref_count;
  };
};

typedef _MemNode memnode_t;
} // namespace sf

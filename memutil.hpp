#pragma once

/**
 * utility class(es) for reference counting
 * of objects
 */

#include "header.hpp"

/**
 * increase ref_count
 * WARN: Do not use I(X) anywhere, use IR(X) as it provides checks.
 * I(X) is only meant to be used inside IR(X) and not anywhere else
 */
// #define I(X) ++(*(X))
#define I(X) ((X)->__mn_inc ())

/**
 * decrease ref_count
 * WARN: Do not use D(X) anywhere, use DR(X) as it provides checks.
 * D(X) is only meant to be used inside DR(X) and not anywhere else
 */
// #define D(X) --(*(X))
#define D(X) ((X)->__mn_dec ())

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
  std::thread::id creator_id;

public:
  _MemNode () : ref_count (0), creator_id (std::this_thread::get_id ()) {};
  ~_MemNode () = default;

  _MemNode &__mn_inc (std::thread::id _this_thread
                      = std::this_thread::get_id ());
  _MemNode &__mn_dec (std::thread::id _this_thread
                      = std::this_thread::get_id ());

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

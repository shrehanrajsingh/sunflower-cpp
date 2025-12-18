#include "memutil.hpp"

namespace sf
{
_MemNode &
_MemNode::__mn_inc (std::thread::id _this_thread)
{
  if (_this_thread != creator_id)
    return *this;

  return ++(*this);
}

_MemNode &
_MemNode::__mn_dec (std::thread::id _this_thread)
{
  if (_this_thread != creator_id)
    return *this;

  return --(*this);
}

_MemNode &
_MemNode::operator-- ()
{
  --ref_count;
  return *this;
}

_MemNode
_MemNode::operator-- (int)
{
  _MemNode tmp = *this;
  --ref_count;
  return tmp;
}

_MemNode &
_MemNode::operator++ ()
{
  ++ref_count;
  return *this;
}

_MemNode
_MemNode::operator++ (int)
{
  _MemNode tmp = *this;
  ++ref_count;
  return *this;
}

void
_MemNode::set_ref_count (int i)
{
  ref_count = i;
}
} // namespace sf

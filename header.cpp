#include "header.hpp"

namespace sf
{
static global_env_t global_env;

SF_API global_env_t &
__sf_get_global_env ()
{
  return global_env;
}
} // namespace sf

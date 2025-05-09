#include "object.hpp"

namespace sf
{
void
_sfobj_refcheck (Object *&obj)
{
  if (obj->get_ref_count () < 1)
    {
      delete obj;
      obj = nullptr;
    }
}

bool
_sfobj_isfalse (Module &mod, Object *&obj)
{
  switch (obj->get_type ())
    {
    case ObjectType::Constant:
      {
        ConstantObject *co = static_cast<ConstantObject *> (obj);
        Constant *cc = co->get_c ().get ();

        switch (cc->get_type ())
          {
          case ConstantType::Boolean:
            {
              return (static_cast<BooleanConstant *> (cc))->get_value ();
            }
            break;
          case ConstantType::Float:
            {
              return (static_cast<FloatConstant *> (cc))->get_value () == 0.0f;
            }
            break;
          case ConstantType::Integer:
            {
              return (static_cast<IntegerConstant *> (cc))->get_value () == 0;
            }
            break;
          case ConstantType::NoneType:
            {
              return true;
            }
            break;
          case ConstantType::String:
            {
              return (static_cast<StringConstant *> (cc))->get_value ().size ()
                     == 0;
            }
            break;

          default:
            break;
          }
      }
      break;

    default:
      break;
    }

  return false;
}
} // namespace sf

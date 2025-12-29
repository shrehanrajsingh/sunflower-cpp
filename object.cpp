#include "object.hpp"
#include "module.hpp"

namespace sf
{
void
_sfobj_refcheck (Object *&obj)
{
  // if (obj->get_type () == ObjectType::FuncObject)
  //   {
  //     FunctionObject *fo = static_cast<FunctionObject *> (obj);

  //     if (fo->get_v () != nullptr && fo->get_v ()->get_self_arg ())
  //       {
  //         if (obj->get_self_arg () != nullptr)
  //           {
  //             // std::cout << obj->get_self_arg ()->get_ref_count () <<
  //             '\n'; DR (obj->get_self_arg ()); obj->get_self_arg () =
  //             nullptr;
  //           }
  //       }
  //   }

  if (obj->get_ref_count () < 1)
    {
      if (obj->get_type () == ObjectType::FuncObject)
        {
          FunctionObject *fo = static_cast<FunctionObject *> (obj);

          if (fo->get_self_arg () != nullptr)
            DR (fo->get_self_arg ());
        }

      if (obj->get_type () == ObjectType::HalfFunction)
        {
          HalfFunction *hf = static_cast<HalfFunction *> (obj);

          if (hf->get_function_obj () != nullptr)
            DR (hf->get_function_obj ());

          for (Object *&i : hf->get_args ())
            DR (i);
        }
      // if (obj->get_type () == ObjectType::ModuleObject)
      //   {
      //     ModuleObject *mo = static_cast<ModuleObject *> (obj);
      //     Module *momod = mo->get_mod ();

      //     Vec<Statement *> &getstmts = momod->get_stmts ();

      //     for (Statement *&i : getstmts)
      //       delete i;
      //   }

      if (obj->get_type () == ObjectType::ClassObj)
        {
          ClassObject *co = static_cast<ClassObject *> (obj);
          Module *mco = co->get_mod ();

          IR (obj);

          if (mco->has_variable ("_kill"))
            {
              Object *kill_fun = mco->get_variable ("_kill");

              Vec<Object *> args;
              // args.push_back (obj);
              // IR (obj);
              kill_fun->get_self_arg () = obj;
              Object *res
                  = call_func (*mco->get_parent (), kill_fun, args, obj);

              DR (res);

              // if (kill_fun->get_type () == ObjectType::FuncObject)
              //   {
              //     FunctionObject *fo
              //         = static_cast<FunctionObject *> (kill_fun);
              //     Function *fv = fo->get_v ();

              //     switch (fv->get_type ())
              //       {
              //       case FuncType::Coded:
              //         {
              //           CodedFunction *cf = static_cast<CodedFunction *>
              //           (fv); Module *fmod
              //               = new Module (ModuleType::File, cf->get_body
              //               ());

              //           fmod->set_parent (mco->get_parent ());

              //           Expr *e = cf->get_args ()[0];

              //           if (e->get_type () == ExprType::Variable)
              //             {
              //               fmod->set_variable (static_cast<VariableExpr *>
              //               (e)
              //                                       ->get_name ()
              //                                       .get_internal_buffer (),
              //                                   obj);
              //             }
              //           else if (e->get_type () == ExprType::VarDecl)
              //             {
              //               /* TODO */
              //               here;
              //               std::cout << "TODO";
              //             }

              //           mod_exec (*fmod);

              //           delete fmod;
              //         }
              //         break;

              //       case FuncType::Native:
              //         {
              //           /* TODO */
              //         }
              //         break;

              //       default:
              //         break;
              //       }
              //   }
            }

          /* call destructor of inherits in reverse MRO order */
          for (int i = co->get_mro ().get_size () - 1; i > -1; i--)
            {
              // std::cout << co->get_mro ()[i]->get_ref_count () << '\n';
              DR (co->get_mro ()[i]);
            }

          // /**
          //  * remove self_arg from all variables
          //  * which have self_arg as this class (obj)
          //  */
          // for (auto &&i : mco->get_vtable ())
          //   {
          //     if (i.second->get_self_arg () != nullptr
          //         && i.second->get_self_arg () == obj)
          //       i.second->get_self_arg () = nullptr;
          //   }

          /**
           * set the ref count to a very
           * big value so even if
           * variables exist who have
           * this class in their reference
           * they will not trigger a seg fault
           *
           * This breaks if there are >INT32_MAX
           * references. Anyone who writes
           * programs involving this condition
           * deserves the seg fault.
           * Although in the future I might add
           * some flags to bless such cases
           */
          obj->get_ref_count () = INT32_MAX;
        }

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
              return (static_cast<BooleanConstant *> (cc))->get_value ()
                     == false;
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

bool
_sfobj_cmp (Object *&lval, Object *&rval, ConditionalType t)
{
  bool res = false;
  switch (t)
    {
    case ConditionalType::EqEq:
      {
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 == f2;
          }
        else if (lval->get_type () == rval->get_type ())
          {
            switch (lval->get_type ())
              {
              case ObjectType::Constant:
                {
                  ConstantObject *lco = static_cast<ConstantObject *> (lval);
                  ConstantObject *rco = static_cast<ConstantObject *> (rval);

                  Constant *lcoc = lco->get_c ().get ();
                  Constant *rcoc = rco->get_c ().get ();

                  if (lcoc->get_type () == rcoc->get_type ())
                    {
                      switch (lcoc->get_type ())
                        {
                        case ConstantType::Boolean:
                          {
                            res = static_cast<BooleanConstant *> (lcoc)
                                      ->get_value ()
                                  == static_cast<BooleanConstant *> (rcoc)
                                         ->get_value ();
                          }
                          break;

                        case ConstantType::NoneType:
                          {
                            res = true;
                          }
                          break;
                        case ConstantType::String:
                          {
                            res = static_cast<StringConstant *> (lcoc)
                                      ->get_value ()
                                  == static_cast<StringConstant *> (rcoc)
                                         ->get_value ();
                          }
                          break;

                        default:
                          break;
                        }
                    }
                }
                break;

              case ObjectType::FuncObject:
                {
                  res = static_cast<FunctionObject *> (lval)->get_v ()
                        == static_cast<FunctionObject *> (rval)->get_v ();
                }
                break;

              default:
                break;
              }
          }
      }
      break;

    case ConditionalType::NEq:
      {
        /* same logic for eqeq, just flip boolean at the end */
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 == f2;
          }
        else if (lval->get_type () == rval->get_type ())
          {
            switch (lval->get_type ())
              {
              case ObjectType::Constant:
                {
                  ConstantObject *lco = static_cast<ConstantObject *> (lval);
                  ConstantObject *rco = static_cast<ConstantObject *> (rval);

                  Constant *lcoc = lco->get_c ().get ();
                  Constant *rcoc = rco->get_c ().get ();

                  if (lcoc->get_type () == rcoc->get_type ())
                    {
                      switch (lcoc->get_type ())
                        {
                        case ConstantType::Boolean:
                          {
                            res = static_cast<BooleanConstant *> (lcoc)
                                      ->get_value ()
                                  == static_cast<BooleanConstant *> (rcoc)
                                         ->get_value ();
                          }
                          break;

                        case ConstantType::NoneType:
                          {
                            res = true;
                          }
                          break;
                        case ConstantType::String:
                          {
                            res = static_cast<StringConstant *> (lcoc)
                                      ->get_value ()
                                  == static_cast<StringConstant *> (rcoc)
                                         ->get_value ();
                          }
                          break;

                        default:
                          break;
                        }
                    }
                }
                break;

              case ObjectType::FuncObject:
                {
                  res = static_cast<FunctionObject *> (lval)->get_v ()
                        == static_cast<FunctionObject *> (rval)->get_v ();
                }
                break;

              default:
                break;
              }
          }

        res = !res;
      }
      break;

    case ConditionalType::Le:
      {
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 < f2;
          }
      }
      break;

    case ConditionalType::LEq:
      {
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 <= f2;
          }
      }
      break;

    case ConditionalType::Ge:
      {
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 > f2;
          }
      }
      break;

    case ConditionalType::GEq:
      {
        if (OBJ_IS_NUMBER (lval) && OBJ_IS_NUMBER (rval))
          {
            ConstantObject *lc = static_cast<ConstantObject *> (lval);
            ConstantObject *rc = static_cast<ConstantObject *> (rval);

            float f1 = 0.0, f2 = 0.0;

            Constant *lcc = lc->get_c ().get ();
            Constant *rcc = rc->get_c ().get ();

            switch (lcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f1 = float (
                      static_cast<IntegerConstant *> (lcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f1 = static_cast<FloatConstant *> (lcc)->get_value ();
                }
                break;

              default:
                break;
              }

            switch (rcc->get_type ())
              {
              case ConstantType::Integer:
                {
                  f2 = float (
                      static_cast<IntegerConstant *> (rcc)->get_value ());
                }
                break;
              case ConstantType::Float:
                {
                  f2 = static_cast<FloatConstant *> (rcc)->get_value ();
                }
                break;

              default:
                break;
              }

            res = f1 >= f2;
          }
      }
      break;

    default:
      break;
    }

  return res;
}

void
_sfobj_passownership (Object *&obj)
{
  switch (obj->get_type ())
    {
    case ObjectType::FuncObject:
      {
        FunctionObject *fobj = static_cast<FunctionObject *> (obj);
        Function *&f = fobj->get_v ();

        I (f);
        // std::cout << f->get_ref_count () << '\n';
      }
      break;

    case ObjectType::AmbigObject:
      {
        AmbigObject *ao = static_cast<AmbigObject *> (obj);

        if (ao->get_val () != nullptr)
          ao->get_val ()->get_ref_count () = obj->get_ref_count () + 1;
      }
      break;

    default:
      break;
    }
}

void
_sfobj_removeownership (Object *&obj)
{
  switch (obj->get_type ())
    {
    case ObjectType::FuncObject:
      {
        FunctionObject *fobj = static_cast<FunctionObject *> (obj);
        Function *&f = fobj->get_v ();

        // std::cout << f->get_ref_count () << '\n';

        DR__func (f);
      }
      break;

    case ObjectType::AmbigObject:
      {
        AmbigObject *ao = static_cast<AmbigObject *> (obj);

        if (ao->get_val () != nullptr)
          ao->get_val ()->get_ref_count () = obj->get_ref_count () - 1;
      }
      break;

    default:
      break;
    }
}

std::string
ConstantObject::get_stdout_repr ()
{
  Constant *cp = c.get ();
  std::string res;

  switch (cp->get_type ())
    {
    case ConstantType::Boolean:
      res = static_cast<BooleanConstant *> (cp)->get_value () ? "true"
                                                              : "false";
      break;

    case ConstantType::Float:
      res = std::to_string (static_cast<FloatConstant *> (cp)->get_value ());
      break;

    case ConstantType::Integer:
      res = std::to_string (static_cast<IntegerConstant *> (cp)->get_value ());
      break;

    case ConstantType::NoneType:
      res = "none";
      break;

    case ConstantType::AmbigType:
      res = "ambig";
      break;

    case ConstantType::String:
      res = static_cast<StringConstant *> (cp)->get_value ().to_std_string ();
      break;

    default:
      break;
    }

  return res;
}

std::string
ConstantObject::get_stdout_repr_in_container ()
{
  Constant *cp = c.get ();
  std::string res;

  switch (cp->get_type ())
    {
    case ConstantType::Boolean:
      res = static_cast<BooleanConstant *> (cp)->get_value () ? "true"
                                                              : "false";
      break;

    case ConstantType::Float:
      res = std::to_string (static_cast<FloatConstant *> (cp)->get_value ());
      break;

    case ConstantType::Integer:
      res = std::to_string (static_cast<IntegerConstant *> (cp)->get_value ());
      break;

    case ConstantType::NoneType:
      res = "none";
      break;

    case ConstantType::String:
      res = std::string{ "\"" }
            + static_cast<StringConstant *> (cp)
                  ->get_value ()
                  .get_internal_buffer ()
            + "\"";
      break;

    default:
      break;
    }

  return res;
}

std::string
NoObj::get_stdout_repr ()
{
  return "<class internal::NoObj>";
}

std::string
FunctionObject::get_stdout_repr ()
{
  std::stringstream s;
  s << "<function " << v << ">";

  return s.str ();
}

std::string
AmbigObject::get_stdout_repr ()
{
  return "ambig";
}

std::string
ModuleObject::get_stdout_repr ()
{
  return "<module>";
}

std::string
HalfFunction::get_stdout_repr ()
{
  std::stringstream s;
  s << "<hfunction " << function_obj << ">";

  return s.str ();
}

bool
_sfobj_isiterable (Object *&obj)
{
  /* add all iterables here */
  return (obj->get_type () == ObjectType::ArrayObj) || OBJ_IS_STR (obj);
}

bool
_sfobj_iscallable (Module &mod, Object *&obj)
{
  /* add all callables here */
  return obj->get_type () == ObjectType::FuncObject;
}

ModuleObject::~ModuleObject ()
{
  // here;
  Vec<Statement *> &getstmts = mod->get_stmts ();

  for (Statement *&i : getstmts)
    {
      delete i;
    }

  if (mod != nullptr)
    delete mod;
}
} // namespace sf

#include "module.hpp"

namespace sf
{
Object *
Module::get_variable (std::string s)
{
  if (!vtable.count (s))               /* not found */
    throw "module_var_does_not_exist"; /* temp fix */

  return vtable.at (s);
}

void
Module::set_variable (std::string n, Object v)
{
  vtable[n] = new Object (v);
}

void
Module::set_variable (std::string n, Object *v)
{
  I (v);
  vtable[n] = v;
}

void
mod_exec (Module &mod)
{
  Vec<Statement *> &stmts = mod.get_stmts ();
  size_t i = 0;

  while (i < stmts.get_size ())
    {
      Statement *st = stmts[i];

      switch (st->get_type ())
        {
        case StatementType::VarDecl:
          {
            VarDeclStatement *v = static_cast<VarDeclStatement *> (st);
            Expr *ne = v->get_name ();
            Expr *nv = v->get_value ();

            Object *val_eval;

            /*
              Safe eval with TC,
              expand later with safety checks
             */
            TC (val_eval = expr_eval (mod, nv));

            /*
              Unless val_eval is nullptr,
              (which will be checked in expr_eval itself)
              we always increase ref_count by 1 to count
              for r-values, i.e, every object returned from
              expr_eval will have a ref_count of at-least 1.
              Now that we have our object, we need not
              worry about deallocation, so we can safely
              decrease its ref_count.
              This practice will be used throughout the library.
             */
            D (val_eval);

            // std::cout << val_eval->get_ref_count () << '\n';

            switch (ne->get_type ())
              {
              case ExprType::Variable:
                {
                  VariableExpr *nv = static_cast<VariableExpr *> (ne);

                  char *p = (char *)nv->get_name ().c_str ();
                  mod.set_variable (p, val_eval); /* set_variable will handle
                                                     obj_count increment */

                  delete p;
                }
                break;

              default:
                std::cerr << "invalid expr type: " << (int)ne->get_type ()
                          << std::endl;
                break;
              }

            /*
              It's possible val_eval evaluated an r-value.
              In that case, ref_count of val_eval will be 0 at this stage.
              (Note: we called obj_eval which returns an object with
              a ref_count of at-least 1, but after calling it we decremented
              it's ref_count by 1, reducing the count by 1).
              If ref_count after these steps is 0, we can safely
              free the object as our work with it is done.
            */
            R (val_eval);
            // std::cout << val_eval->get_ref_count () << '\n';
          }
          break;
        case StatementType::FuncCall:
          {
            FuncCallStatement *fst = static_cast<FuncCallStatement *> (st);
            Object *name_eval;

            TC (name_eval = expr_eval (mod, fst->get_name ()));
            D (name_eval);

            Vec<Object *> args_eval;
            for (auto j : fst->get_args ())
              {
                Object *t = nullptr;
                TC (t = expr_eval (mod, j));
                D (t);

                args_eval.push_back (t);
              }

            switch (name_eval->get_type ())
              {
              case ObjectType::FuncObject:
                {
                  FunctionObject *fo
                      = static_cast<FunctionObject *> (name_eval);

                  Function *fv = fo->get_v ();

                  switch (fv->get_type ())
                    {
                    case FuncType::Native:
                      {
                        NativeFunction *nf
                            = static_cast<NativeFunction *> (fv);

                        Module *fmod = new Module (ModuleType::Function,
                                                   Vec<Statement *> ());
                        fmod->set_parent (&mod);

                        size_t j = 0;
                        for (Str &a : nf->get_args ())
                          {
                            char *p = a.c_str ();
                            fmod->set_variable (p, args_eval[j++]);

                            delete p;
                          }

                        Object *ret;

                        TC (ret = nf->call (fmod));
                        DR (ret);

                        delete fmod;
                      }
                      break;

                    case FuncType::Coded:
                      {
                        /**
                         * TODO: Implement this sh*t
                         */
                      }
                      break;

                    default:
                      /* unreachable */
                      exit (__LINE__);
                      break;
                    }
                }
                break;

              default:
                ERRMSG ("Entity with type" << (int)name_eval->get_type ()
                                           << "is not callable");
                break;
              }

            // for (auto o : args_eval)
            //   {
            //     R (o);
            //   }
            R (name_eval);
          }
          break;

        case StatementType::IfConstruct:
          {
            IfConstruct *ic = static_cast<IfConstruct *> (st);
            assert (ic->get_cond () != nullptr);

            Object *cond_eval = expr_eval (mod, ic->get_cond ());
            /*
              Since expr_eval already returns an object with an incremented
              reference count, we do not need to manually increase ourselves.
              We do note that we need to decrease the ref_count by 1 after
              we are done with the construct
             */

            Vec<Statement *> pres_st = mod.get_stmts ();
            bool eval_else = false;

            if (_sfobj_isfalse (mod, cond_eval))
              {
                if (ic->get_elifconstructs ().get_size ())
                  {
                    /* eval elifs */
                  }
                else
                  eval_else = true;
              }
            else
              {
                mod.get_stmts () = ic->get_body ();
              }

            if (eval_else)
              {
                mod.get_stmts () = ic->get_else_body ();
              }

            mod_exec (mod);
            mod.get_stmts () = pres_st;
            DR (cond_eval); /* reclaim object */
          }
          break;

        default:
          std::cerr << "invalid type: " << (int)st->get_type () << std::endl;
          break;
        }

    end:
      i++;
    }
}

Object *
expr_eval (Module &mod, Expr *e)
{
  Object *res = nullptr;

  switch (e->get_type ())
    {
    case ExprType::Constant:
      {
        ConstantExpr *ce = static_cast<ConstantExpr *> (e);

        Constant *cc = ce->get_c ();

        if (res != nullptr)
          DR (res);

        switch (cc->get_type ())
          {
          case ConstantType::Integer:
            {

              /* handle destruction */
              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new IntegerConstant (
                      ((IntegerConstant *)cc)->get_value ()))));
            }
            break;

          case ConstantType::Boolean:
            {
              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new BooleanConstant (
                      ((BooleanConstant *)cc)->get_value ()))));
            }
            break;
          case ConstantType::String:
            {
              char *p = (char *)((static_cast<StringConstant *> (cc))
                                     ->get_value ()
                                     .c_str ());

              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new StringConstant (p))));

              delete p;
            }
            break;

          case ConstantType::Float:
            {
              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new FloatConstant (
                      ((FloatConstant *)cc)->get_value ()))));
            }
            break;

          case ConstantType::NoneType:
            {
              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new NoneConstant ())));
            }
            break;

          default:
            break;
          }

        I (res);
      }
      break;

    case ExprType::Variable:
      {
        VariableExpr *ve = static_cast<VariableExpr *> (e);
        char *p = (char *)ve->get_name ().c_str ();
        try
          {
            Object *o = mod.get_variable (p);

            if (res != nullptr)
              DR (res);
            res = o;

            I (res);

            delete p;
          }
        catch (const char *&e)
          {
            std::cerr << e << '\n';

            delete p;
            exit (EXIT_FAILURE);
          }
      }
      break;

    case ExprType::Function:
      {
        FunctionExpr *fe = static_cast<FunctionExpr *> (e);

        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new FunctionObject (fe->get_v ().get ()));
        I (res);
      }
      break;

    default:
      std::cerr << "invalid expr type: " << (int)e->get_type () << std::endl;
      break;
    }

  if (res == nullptr)
    throw "res_expr_is_null";
  return res;
}
} // namespace sf

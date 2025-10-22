#include "module.hpp"
#include "ast.hpp"
#include "native/native.hpp"
#include "stmt.hpp"

#include "native/module/nmod.hpp"

namespace sf
{
Object *
Module::get_variable (std::string s)
{
  if (!vtable.count (s)) /* not found */
    {
      if (parent == nullptr)
        {
          std::cout << s << ": ";
          throw "module_var_does_not_exist"; /* temp fix */
        }
      else
        return parent->get_variable (s);
    }

  return vtable.at (s);
}

void
Module::set_variable (std::string n, Object *v)
{
  if (vtable.find (n) != vtable.end ())
    {
      DR (vtable[n]);
    }

  IR (v);

  vtable[n] = v;
  vhist[n] = var_priority_counter++;
}

void
mod_exec (Module &mod)
{
  Vec<Statement *> &stmts = mod.get_stmts ();
  size_t i = 0;

  while (mod.get_continue_exec () && i < stmts.get_size ())
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
            TC (val_eval = expr_eval (mod, nv));
            assert (val_eval != nullptr);

            AMBIG_CHECK (val_eval, {
              mod.get_backtrace ().push_back (st->get_line_number ());
              // std::cout << val_eval->get_ref_count () << '\n';
              // mod.get_continue_exec () = false;
              // mod.get_saw_ambig () = true;
              // mod.get_ambig () = val_eval;

              // IR (val_eval);
            });

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
            // D (val_eval); /* this is done after the switch () */
            // _sfobj_removeownership (val_eval);

            // std::cout << val_eval->get_ref_count () << '\n';

            switch (ne->get_type ())
              {
              case ExprType::Variable:
                {
                  VariableExpr *nv = static_cast<VariableExpr *> (ne);

                  char *p = (char *)nv->get_name ().get_internal_buffer ();
                  mod.set_variable (p, val_eval); /* set_variable takes care of
                                                     ref_count increment */
                }
                break;

              case ExprType::ArrayExp:
                {
                  /**
                   * [a, b] = iterable
                   * NOTE: For now, we assume size of
                   * LHS array = size of RHS iterable
                   */

                  ArrayExpr *av = static_cast<ArrayExpr *> (ne);

                  if (_sfobj_isiterable (val_eval))
                    {
                      switch (val_eval->get_type ())
                        {
                        case ObjectType::ArrayObj:
                          {
                            ArrayObject *vao
                                = static_cast<ArrayObject *> (val_eval);

                            /**
                             * NOTE: for now
                             */
                            assert (av->get_vals ().get_size ()
                                    == vao->get_vals ().get_size ());

                            for (size_t i = 0;
                                 i < vao->get_vals ().get_size (); i++)
                              {
                                Expr *&iv = av->get_vals ()[i];

                                switch (iv->get_type ())
                                  {
                                  case ExprType::Variable:
                                    {
                                      VariableExpr *ive
                                          = static_cast<VariableExpr *> (iv);

                                      Object *&v = vao->get_vals ()[i];

                                      char *p
                                          = (char *)ive->get_name ().c_str ();
                                      mod.set_variable (
                                          p, v); /* set_variable takes
                                                           care of ref_count
                                                           increment */

                                      delete[] p;
                                    }
                                    break;

                                  default:
                                    throw std::invalid_argument (
                                        "[] = ... only supports variable "
                                        "names in []");
                                    break;
                                  }
                              }
                          }
                          break;

                        default:
                          break;
                        }
                    }
                }
                break;
              case ExprType::ArrayAccess:
                {
                  ArrayAccess *ac = static_cast<ArrayAccess *> (ne);
                  Expr *arr = ac->get_arr ();
                  Expr *idx = ac->get_idx ();

                  Object *oarr;
                  Object *oidx;

                  TC (oarr = expr_eval (mod, arr));
                  AMBIG_CHECK (oarr, {
                    mod.get_backtrace ().push_back (st->get_line_number ());
                  });

                  TC (oidx = expr_eval (mod, idx));
                  AMBIG_CHECK (oidx, {
                    DR (oarr);
                    mod.get_backtrace ().push_back (st->get_line_number ());
                  });

                  switch (oarr->get_type ())
                    {
                    case ObjectType::ArrayObj:
                      {
                        ArrayObject *ao = static_cast<ArrayObject *> (oarr);

                        assert (OBJ_IS_INT (oidx)
                                && "Index must be an integer");

                        int iv = static_cast<IntegerConstant *> (
                                     static_cast<ConstantObject *> (oidx)
                                         ->get_c ()
                                         .get ())
                                     ->get_value ();

                        assert (iv > -1 && iv < ao->get_vals ().get_size ()
                                && "Index out of bounds");

                        Object *prev = ao->get_vals ()[iv];
                        DR (prev);

                        ao->get_vals ()[iv] = val_eval;
                        IR (val_eval);
                      }
                      break;
                    case ObjectType::DictObj:
                      {
                        DictObject *dobj = static_cast<DictObject *> (oarr);

                        assert (OBJ_IS_STR (oidx)
                                && "Object key must be a string");

                        Str &k = static_cast<StringConstant *> (
                                     static_cast<ConstantObject *> (oidx)
                                         ->get_c ()
                                         .get ())
                                     ->get_value ();

                        char *p = k.c_str ();
                        std::string sp = p;

                        if (dobj->get_vals ().find (sp)
                            != dobj->get_vals ().end ())
                          {
                            Object *prev = dobj->get_vals ()[sp];
                            DR (prev);
                          }

                        dobj->get_vals ()[sp] = val_eval;
                        IR (val_eval);

                        delete[] p;
                      }
                      break;

                    case ObjectType::Constant:
                      {
                        ConstantObject *co
                            = static_cast<ConstantObject *> (oarr);

                        Constant *co_c = co->get_c ().get ();

                        switch (co_c->get_type ())
                          {
                          case ConstantType::String:
                            {
                              assert (OBJ_IS_STR (val_eval)
                                      && "Value for string "
                                         "assignment must be "
                                         "a string");

                              Str &rhs_val
                                  = static_cast<StringConstant *> (
                                        static_cast<ConstantObject *> (
                                            val_eval)
                                            ->get_c ()
                                            .get ())
                                        ->get_value ();

                              StringConstant *sc
                                  = static_cast<StringConstant *> (co_c);

                              Str &sc_val = sc->get_value ();

                              switch (oidx->get_type ())
                                {
                                case ObjectType::Constant:
                                  {
                                    ConstantObject *idx_co
                                        = static_cast<ConstantObject *> (oidx);

                                    Constant *idx_c = idx_co->get_c ().get ();

                                    switch (idx_c->get_type ())
                                      {
                                      case ConstantType::Integer:
                                        {
                                          int iv = static_cast<
                                                       IntegerConstant *> (
                                                       static_cast<
                                                           ConstantObject *> (
                                                           oidx)
                                                           ->get_c ()
                                                           .get ())
                                                       ->get_value ();

                                          assert (iv > -1
                                                  && iv < sc_val.size ()
                                                  && "Index out of bounds");

                                          assert (rhs_val.size () == 1
                                                  && "Expected a single "
                                                     "character");

                                          sc_val[iv] = rhs_val[0];
                                        }
                                        break;

                                      default:
                                        break;
                                      }
                                  }
                                  break;

                                case ObjectType::ArrayObj:
                                  {
                                    /*
                                      a = "somelongstring"
                                      a[i to j] = "string of size (j-i)"
                                    */

                                    ArrayObject *ao
                                        = static_cast<ArrayObject *> (oidx);

                                    Vec<Object *> &ao_vals = ao->get_vals ();

                                    assert (ao_vals.get_size ()
                                                == rhs_val.size ()
                                            && "String assignment requires "
                                               "bounds to be equal to size of "
                                               "value string");

                                    for (int i = 0; i < ao_vals.get_size ();
                                         i++)
                                      {
                                        Object *ao_i = ao_vals[i];

                                        assert (OBJ_IS_INT (ao_i)
                                                && "Values in array index "
                                                   "must be integers");

                                        int iv
                                            = static_cast<IntegerConstant *> (
                                                  static_cast<
                                                      ConstantObject *> (ao_i)
                                                      ->get_c ()
                                                      .get ())
                                                  ->get_value ();

                                        sc_val[iv] = rhs_val[i];
                                      }
                                  }
                                  break;

                                default:
                                  {
                                    std::cerr << "Invalid index for string "
                                                 "assignment.";

                                    exit (-1);
                                  }
                                  break;
                                }
                            }
                            break;

                          default:
                            {
                              std::cerr << "Invalid constant assignment\n";
                              exit (-1);
                            }
                            break;
                          }
                      }
                      break;

                    default:
                      break;
                    }

                  DR (oarr);
                  DR (oidx);
                }
                break;
              case ExprType::DotAccess:
                {
                  DotAccess *da = static_cast<DotAccess *> (ne);

                  Expr *&e_parent = da->get_parent ();
                  Expr *&e_child = da->get_child ();

                  assert (e_parent != nullptr);
                  assert (e_child != nullptr
                          && e_child->get_type () == ExprType::Variable);

                  Str &member
                      = static_cast<VariableExpr *> (e_child)->get_name ();

                  Object *o_parent;
                  TC (o_parent = expr_eval (mod, e_parent));
                  AMBIG_CHECK (o_parent, {
                    mod.get_backtrace ().push_back (st->get_line_number ());
                  });

                  switch (o_parent->get_type ())
                    {
                    case ObjectType::ClassObj:
                      {
                        ClassObject *co
                            = static_cast<ClassObject *> (o_parent);
                        co->get_mod ()->set_variable (
                            member.get_internal_buffer (), val_eval);
                      }
                      break;

                    case ObjectType::SfClass:
                      {
                        SfClass *cl = static_cast<SfClass *> (o_parent);
                        cl->get_mod ()->set_variable (
                            member.get_internal_buffer (), val_eval);
                      }
                      break;

                    default:
                      break;
                    }

                  DR (o_parent);
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
            // R (val_eval);
            DR (val_eval);
            // std::cout << val_eval->get_ref_count () << '\n';
          }
          break;
        case StatementType::FuncCall:
          {
            FuncCallStatement *fst = static_cast<FuncCallStatement *> (st);
            Object *name_eval;

            TC (name_eval = expr_eval (mod, fst->get_name ()));
            AMBIG_CHECK (name_eval, {
              mod.get_backtrace ().push_back (st->get_line_number ());
            });

            Vec<Object *> args_eval;

            for (auto j : fst->get_args ())
              {
                Object *t = nullptr;
                TC (t = expr_eval (mod, j));
                assert (t != nullptr);

                AMBIG_CHECK (t, {
                  DR (name_eval);
                  mod.get_backtrace ().push_back (st->get_line_number ());
                });

                /**
                 * We could use DR here
                 * but we do not need
                 * to deallocate the object
                 * so we only do 2 out of 3
                 * steps of DR
                 */
                D (t);
                _sfobj_removeownership (t);

                args_eval.push_back (t);
              }

            switch (name_eval->get_type ())
              {
              case ObjectType::FuncObject:
                {
                  Object *ret = call_func (mod, name_eval, args_eval);
                  DR (ret);
                }
                break;
              case ObjectType::SfClass:
                {
                  Object *obj = call_func (mod, name_eval, args_eval);
                  DR (obj);
                }
                break;

              case ObjectType::ModuleObject:
                {
                  Object *obj = call_func (mod, name_eval, args_eval);
                  DR (obj);
                }
                break;

              default:
                ERRMSG ("Entity with type " << (int)name_eval->get_type ()
                                            << " is not callable");
                break;
              }

            // for (auto o : args_eval)
            //   {
            //     R (o);
            //   }
            DR (name_eval);
          }
          break;

        case StatementType::IfConstruct:
          {
            IfConstruct *ic = static_cast<IfConstruct *> (st);
            assert (ic->get_cond () != nullptr);

            Object *cond_eval;
            TC (cond_eval = expr_eval (mod, ic->get_cond ()));
            AMBIG_CHECK (cond_eval, {
              mod.get_backtrace ().push_back (st->get_line_number ());
            });
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
                    eval_else = true;
                    for (IfConstruct *&ifc : ic->get_elifconstructs ())
                      {
                        Object *iv;
                        TC (iv = expr_eval (mod, ifc->get_cond ()));
                        AMBIG_CHECK (iv, {
                          DR (cond_eval);
                          mod.get_backtrace ().push_back (
                              st->get_line_number ());
                        });

                        if (!_sfobj_isfalse (mod, iv))
                          {
                            mod.get_stmts () = ifc->get_body ();
                            mod_exec (mod);

                            eval_else = false;
                            DR (iv);
                            break;
                          }

                        DR (iv);
                      }
                  }
                else
                  eval_else = true;
              }
            else
              {
                mod.get_stmts () = ic->get_body ();
                mod_exec (mod);
              }

            if (eval_else)
              {
                mod.get_stmts () = ic->get_else_body ();
                mod_exec (mod);
              }

            mod.get_stmts () = pres_st;
            DR (cond_eval); /* reclaim object */
          }
          break;

        case StatementType::ForConstruct:
          {
            ForConstruct *fc = static_cast<ForConstruct *> (st);

            Vec<Expr *> &var_list = fc->get_var_list ();
            Vec<Statement *> &for_body = fc->get_body ();
            Vec<Statement *> mod_body_pres = mod.get_stmts ();

            mod.get_stmts () = fc->get_body ();

            Object *it_eval;
            TC (it_eval = expr_eval (mod, fc->get_iterable ()));
            AMBIG_CHECK (it_eval, {
              mod.get_backtrace ().push_back (st->get_line_number ());
            });

            switch (it_eval->get_type ())
              {
              case ObjectType::ArrayObj:
                {
                  ArrayObject *ao = static_cast<ArrayObject *> (it_eval);
                  Vec<Object *> ao_vals = ao->get_vals ();

                  for (Object *&av : ao_vals)
                    {
                      if (var_list.get_size () == 1)
                        {
                          Expr *v = var_list[0];

                          switch (v->get_type ())
                            {
                            case ExprType::Variable:
                              {
                                char *p = static_cast<VariableExpr *> (v)
                                              ->get_name ()
                                              .c_str ();
                                mod.set_variable (p, av);

                                delete[] p;
                              }
                              break;

                            default:
                              break;
                            }
                        }
                      else
                        {
                          switch (av->get_type ())
                            {
                            case ObjectType::ArrayObj:
                              {
                                ArrayObject *avo
                                    = static_cast<ArrayObject *> (av);

                                assert (avo->get_vals ().get_size ()
                                        >= var_list.get_size ());

                                if (avo->get_vals ().get_size ()
                                    == var_list.get_size ())
                                  {
                                    /* == */

                                    size_t vi = 0;
                                    for (Expr *&vv : var_list)
                                      {
                                        switch (vv->get_type ())
                                          {
                                          case ExprType::Variable:
                                            {
                                              char *p
                                                  = static_cast<
                                                        VariableExpr *> (vv)
                                                        ->get_name ()
                                                        .c_str ();

                                              mod.set_variable (
                                                  p, avo->get_vals ()[vi++]);

                                              delete[] p;
                                            }
                                            break;

                                          default:
                                            break;
                                          }
                                      }
                                  }
                                else
                                  {
                                    /* > */
                                  }
                              }
                              break;

                            default:
                              break;
                            }
                        }

                      mod_exec (mod);
                    }
                }
                break;
              case ObjectType::DictObj:
                {
                  DictObject *dobj = static_cast<DictObject *> (it_eval);
                  std::map<std::string, Object *> dobj_vals
                      = dobj->get_vals ();

                  for (auto &&i : dobj_vals)
                    {
                      Object *kobj = static_cast<Object *> (
                          new ConstantObject (static_cast<Constant *> (
                              new StringConstant (i.first.c_str ()))));

                      IR (kobj);

                      assert (var_list.get_size () == 1);
                      Expr *v = var_list[0];

                      switch (v->get_type ())
                        {
                        case ExprType::Variable:
                          {
                            char *p = static_cast<VariableExpr *> (v)
                                          ->get_name ()
                                          .c_str ();
                            mod.set_variable (p, kobj);

                            delete[] p;
                          }
                          break;

                        default:
                          break;
                        }

                      mod_exec (mod);
                      DR (kobj);
                    }
                }
                break;

              default:
                break;
              }

            mod.get_stmts () = mod_body_pres;
            DR (it_eval);
          }
          break;

        case StatementType::FuncDecl:
          {
            FuncDeclStatement *fds = static_cast<FuncDeclStatement *> (st);

            CodedFunction *cd
                = new CodedFunction (fds->get_body (), fds->get_args ());
            cd->set_parent (&mod);

            if (mod.get_type () == ModuleType::Class)
              cd->set_self_arg (true);

            FunctionObject *fo = new FunctionObject (cd);

            char *p = fds->get_name ().c_str ();
            mod.set_variable (p, fo);

            delete[] p;
          }
          break;

        case StatementType::ReturnStmt:
          {
            ReturnStatement *rt = static_cast<ReturnStatement *> (st);

            if (mod.get_type () == ModuleType::Function)
              {
                mod.get_continue_exec () = false;
                mod.get_ret () = expr_eval (mod, rt->get_val ());

                AMBIG_CHECK (mod.get_ret (), {
                  mod.get_continue_exec () = false;
                  mod.get_saw_ambig () = true;
                  // DR (mod.get_ret ());

                  mod.get_ambig () = mod.get_ret ();
                  IR (mod.get_ambig ());

                  DR (mod.get_ret ());
                  // here;
                  // std::cout << mod.get_ambig ()->get_ref_count () << '\n';
                  mod.get_ret () = nullptr;
                  mod.get_backtrace ().push_back (st->get_line_number ());
                });
              }
            else
              {
                Module *mref = mod.parent;

                while (mref != nullptr
                       && mref->get_type () != ModuleType::Function)
                  mref = mref->get_parent ();

                if (mref != nullptr)
                  {
                    mref->get_ret () = expr_eval (mod, rt->get_val ());
                    AMBIG_CHECK (mref->get_ret (), {
                      DR (mref->get_ret ());
                      mref->get_continue_exec () = false;

                      mref = mod.parent;

                      while (mref != nullptr
                             && mref->get_type () != ModuleType::Function)
                        {
                          mref->get_continue_exec () = false;
                          mref = mref->get_parent ();
                        }

                      mod.get_backtrace ().push_back (st->get_line_number ());
                    });
                    mref->get_continue_exec () = false;

                    mref = mod.parent;

                    while (mref != nullptr
                           && mref->get_type () != ModuleType::Function)
                      {
                        mref->get_continue_exec () = false;
                        mref = mref->get_parent ();
                      }
                  }
                else
                  {
                    std::cout << "return_used_outside_function\n";
                    throw "return_used_outside_function";
                  }

                goto ret;
              }
          }
          break;

        case StatementType::WhileStmt:
          {
            WhileStatement *ws = static_cast<WhileStatement *> (st);
            Vec<Statement *> &body = ws->get_body ();
            Expr *cond = ws->get_cond ();

            Vec<Statement *> st_pres = mod.get_stmts ();
            mod.get_stmts () = body;

            Object *cond_eval;
            TC (cond_eval = expr_eval (mod, cond));
            AMBIG_CHECK (cond_eval, {
              mod.get_stmts () = st_pres;
              mod.get_backtrace ().push_back (st->get_line_number ());
            });

            while (!_sfobj_isfalse (mod, cond_eval))
              {
                mod_exec (mod);
                DR (cond_eval);
                TC (cond_eval = expr_eval (mod, cond));
                AMBIG_CHECK (cond_eval, {
                  mod.get_stmts () = st_pres;
                  mod.get_backtrace ().push_back (st->get_line_number ());
                });
              }

            DR (cond_eval);
            mod.get_stmts () = st_pres;
          }
          break;

        case StatementType::RepeatStmt:
          {
            RepeatStatement *rs = static_cast<RepeatStatement *> (st);

            Expr *cond = rs->get_cond ();
            Vec<Statement *> &body = rs->get_body ();

            Object *o_cond = nullptr;

            Vec<Statement *> st_pres = mod.get_stmts ();
            mod.get_stmts () = body;

            TC (o_cond = expr_eval (mod, cond));
            if (o_cond)
              AMBIG_CHECK (o_cond, {
                mod.get_stmts () = st_pres;
                mod.get_backtrace ().push_back (st->get_line_number ());
              });
            assert (o_cond && OBJ_IS_INT (o_cond));

            int ov
                = static_cast<IntegerConstant *> (
                      static_cast<ConstantObject *> (o_cond)->get_c ().get ())
                      ->get_value ();

            while (ov > 0 && mod.get_continue_exec ())
              {
                mod_exec (mod);
                ov--;
              }

            DR (o_cond);
            mod.get_stmts () = st_pres;
          }
          break;

        case StatementType::ClassDeclStmt:
          {
            ClassDeclStatement *cds = static_cast<ClassDeclStatement *> (st);
            Module *cmod = new Module (ModuleType::Class);

            cmod->set_parent (&mod);
            cmod->get_stmts () = cds->get_body ();

            TC (mod_exec (*cmod));

            SfClass *sfc = new SfClass (cds->get_name (), cmod);

            mod.set_variable (sfc->get_name ().get_internal_buffer (),
                              static_cast<Object *> (sfc));
          }
          break;

        case StatementType::ImportStmt:
          {
            ImportStatement *ist = static_cast<ImportStatement *> (st);
            Str &alias = ist->get_alias ();
            Str &path = ist->get_filepath ();

            std::ifstream mfp (path.get_internal_buffer ());
            std::ifstream main_f;

            bool file_opened = false;
            bool is_native_module = false;

            Environment *n_env = new Environment ();
            n_env->get_args () = mod.get_env ()->get_args ();
            n_env->get_syspaths () = mod.get_env ()->get_syspaths ();

            if (!mfp)
              {
                /* go through env paths */
                Environment *&env = mod.get_env ();

                if (env == nullptr)
                  {
                    std::cerr << "File '" << path << "' does not exist"
                              << std::endl;
                    exit (-1);
                  }
                else
                  {
                    for (Str &pt : env->get_syspaths ())
                      {
                        Str np = pt + path;

                        if (std::filesystem::is_directory (
                                np.get_internal_buffer ()))
                          np = np + "/_init.sf";

                        std::ifstream file_np (np.get_internal_buffer ());

                        if (!!file_np)
                          {
                            file_np.close ();
                            main_f = std::ifstream (np.get_internal_buffer ());

                            int p_sl = path.find ('/');
                            Str p_dir;

                            for (int i = 0; i < p_sl; i++)
                              p_dir.push_back (path[i]);

                            n_env->add_path (pt + p_dir);
                            file_opened = true;
                            break;
                          }
                      }
                  }
              }
            else
              {
                file_opened = true;
                mfp.close ();
                main_f = std::ifstream (path.get_internal_buffer ());
              }

            Module *m;

            if (!file_opened)
              {
                /* check through native modules */
                Module *gm = native_mod::get_mod (path.get_internal_buffer ());

                if (gm != nullptr)
                  {
                    is_native_module = true;
                    file_opened = true;
                    m = gm;
                  }
              }

            if (!file_opened)
              {
                std::cerr << "File '" << path << "' does not exist"
                          << std::endl;
                exit (-1);
              }

            std::string s, p;
            while (std::getline (main_f, p))
              s += p + '\n';

            Vec<Str> lines;
            lines.push_back ("");

            for (char c : s)
              {
                if (c == '\n')
                  lines.push_back ("");
                else
                  lines.back ().push_back (c);
              }

            if (!is_native_module)
              {
                // std::cout << "(" << s << ")" << std::endl;
                Vec<Token *> r = tokenize ((char *)s.c_str ());
                // Vec<Token *> r;

                Vec<Statement *> ast = stmt_gen (r);
                // Vec<Statement *> ast;

                // for (Statement *&i : ast)
                //   i->print ();

                native::add_natives (ast);

                m = new Module (ModuleType::File, ast);
                m->get_env () = n_env;

                mod_exec (*m);

                for (Token *&i : r)
                  delete i;
              }
            else
              {
                m->get_env () = n_env;
              }

            // for (auto &&i : m->get_vtable ())
            //   std::cout << i.first << '\t' << i.second->get_ref_count ()
            //             << '\n';

            Object *mo = static_cast<Object *> (new ModuleObject (m));

            mod.set_variable (alias.get_internal_buffer (), mo);
            main_f.close ();

            // for (Statement *i : m->get_stmts ())
            //   {
            //     if (i != nullptr)
            //       delete i;
            //   }

            // m->get_stmts ().clear ();
          }
          break;

        case StatementType::TryCatchStmt:
          {
            TryCatchStmt *tcs = static_cast<TryCatchStmt *> (st);

            Vec<Statement *> &try_body = tcs->get_try_body ();
            Vec<Statement *> &catch_body = tcs->get_catch_body ();
            Expr *&catch_clause = tcs->get_cclause ();

            Module *m = new Module (ModuleType::File, try_body,
                                    mod.get_code_lines ());
            m->set_parent (&mod);

            mod_exec (*m);

            if (m->get_saw_ambig ())
              {
                m->get_stmts () = catch_body;
                m->set_parent (nullptr);

                Object *amb_obj = m->get_ambig ();
                assert (OBJ_IS_AMBIG (amb_obj));

                Object *amb_val
                    = static_cast<AmbigObject *> (amb_obj)->get_val ();

                if (amb_val == nullptr)
                  amb_val = static_cast<Object *> (new ConstantObject (
                      static_cast<Constant *> (new NoneConstant ())));

                // std::cout << amb_obj->get_ref_count () << '\t'
                //           << amb_val->get_ref_count () << '\n';

                switch (catch_clause->get_type ())
                  {
                  case ExprType::Variable:
                    {
                      Str &vname = static_cast<VariableExpr *> (catch_clause)
                                       ->get_name ();

                      m->set_variable (vname.get_internal_buffer (), amb_val);
                    }
                    break;

                  default:
                    break;
                  }

                m->set_parent (&mod);

                m->get_saw_ambig () = false;
                m->get_continue_exec () = true;

                // std::cout << amb_obj->get_ref_count () << '\t'
                //           << amb_val->get_ref_count () << '\n';

                m->get_backtrace ().clear ();
                mod_exec (*m);

                // std::cout << amb_obj->get_ref_count () << '\t'
                //           << amb_val->get_ref_count () << '\n';

                if (m->get_saw_ambig ())
                  {
                    mod.get_ambig () = m->get_ambig ();
                    mod.get_saw_ambig () = true;

                    IR (mod.get_ambig ());

                    for (int i = 0; i < m->get_backtrace ().get_size (); i++)
                      {
                        mod.get_backtrace ().push_back (
                            m->get_backtrace ()[i]);
                      }

                    delete m;

                    static_cast<AmbigObject *> (amb_obj)->get_val () = nullptr;
                    DR (amb_obj);

                    goto ambig_test;
                  }

                delete m;

                static_cast<AmbigObject *> (amb_obj)->get_val () = nullptr;
                DR (amb_obj);
              }
            else
              delete m;
          }
          break;

        default:
          std::cerr << "invalid type: " << (int)st->get_type () << std::endl;
          break;
        }

    end:
      i++;
    }

  goto ret;

ambig_test:
  mod.get_saw_ambig () = true;
  if (mod.get_parent () == nullptr) /* only check in top-level module */
    {
      Object *amb = mod.get_ambig ();

      std::cerr << "\n====== EXECUTION ERROR ======\n";
      std::cerr << "Error: Uncaught Ambiguity\n";

      // Display detailed ambiguity information if available
      if (OBJ_IS_AMBIG (amb))
        {
          AmbigObject *ao = static_cast<AmbigObject *> (amb);
          if (ao->get_val () != nullptr)
            std::cerr << "Value: " << ao->get_val ()->get_stdout_repr ()
                      << "\n";
        }
      else if (amb != nullptr)
        {
          std::cerr << "Associated Object: ";
          amb->print ();
        }

      // Display backtrace information
      std::cerr << "\n------ Backtrace ------\n";
      if (!mod.get_backtrace ().get_size ())
        {
          std::cerr << "No backtrace information available.\n";
        }
      else
        {
          for (int i = 0; i < mod.get_backtrace ().get_size (); i++)
            {
              std::cerr << "#" << (i + 1) << " ";
              if (mod.get_backtrace ()[i] < mod.get_code_lines ().get_size ()
                  && mod.get_backtrace ()[i] - 1
                         < mod.get_code_lines ().get_size ())
                {
                  std::string s
                      = mod.get_code_lines ()[mod.get_backtrace ()[i] - 1]
                            .get_internal_buffer ();
                  while (s.front () == ' ' || s.front () == '\t')
                    s.erase (s.begin ());

                  std::cerr << "Line " << mod.get_backtrace ()[i] << ": " << s
                            << "\n";
                }
              else
                {
                  std::cerr << "Line " << mod.get_backtrace ()[i]
                            << ": <source not available>\n";
                }
            }
        }
      std::cerr << "========================\n\n";

      DR (amb);
      return;
    }

ret:;
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

              delete[] p;
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

          case ConstantType::AmbigType:
            {
              AmbigConstant *ac = static_cast<AmbigConstant *> (
                  static_cast<ConstantExpr *> (e)->get_c ());

              // res = static_cast<Object *> (new ConstantObject (
              //     static_cast<Constant *> (new AmbigConstant ())));
              res = static_cast<Object *> (new AmbigObject (
                  ac->get_val () != nullptr ? expr_eval (mod, ac->get_val ())
                                            : nullptr));
            }
            break;

          default:
            break;
          }

        IR (res);
        AMBIG_CHECK (res, {});
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
            assert (res != nullptr);
            IR (res);

            AMBIG_CHECK (res, {});

            delete[] p;
          }
        catch (const char *&e)
          {
            std::cerr << e << '\n';

            delete[] p;
            exit (EXIT_FAILURE);
          }
      }
      break;

    case ExprType::Function:
      {
        FunctionExpr *fe = static_cast<FunctionExpr *> (e);

        if (res != nullptr)
          DR (res);

        Function *f = fe->get_v ().get ();
        if (f->get_type () == FuncType::Native)
          {
            res = static_cast<Object *> (
                new FunctionObject (new NativeFunction (
                    *static_cast<NativeFunction *> (fe->get_v ().get ()))));
          }
        else if (f->get_type () == FuncType::Coded)
          {
            res = static_cast<Object *> (
                new FunctionObject (new CodedFunction (
                    *static_cast<CodedFunction *> (fe->get_v ().get ()))));
          }

        IR (res);
      }
      break;

    case ExprType::Conditional:
      {
        ConditionalExpr *ce = static_cast<ConditionalExpr *> (e);
        Object *lobj;
        TC (lobj = expr_eval (mod, ce->get_lval ()));
        AMBIG_CHECK (lobj, {});

        Object *robj;
        TC (robj = expr_eval (mod, ce->get_rval ()));
        AMBIG_CHECK (robj, {});

        bool cnd = _sfobj_cmp (lobj, robj, ce->get_cond_type ());
        // lobj->print ();
        // std::cout << '\n';
        // robj->print ();
        // std::cout << '\n';

        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new BooleanConstant (cnd))));

        IR (res);

        DR (lobj);
        DR (robj);
      }
      break;

    case ExprType::ArrayExp:
      {
        ArrayExpr *ae = static_cast<ArrayExpr *> (e);
        Vec<Object *> ev_idcs;

        for (Expr *&i : ae->get_vals ())
          {
            /**
             * expr_eval returns an object that with one
             * extra incremented ref_count which
             * we can use for index in an array
             */
            TC (ev_idcs.push_back (expr_eval (mod, i)));
            AMBIG_CHECK (ev_idcs.back (), {
              res = ev_idcs.back ();
              for (Object *&j : ev_idcs)
                {
                  DR (j);
                }
            });
          }

        if (res != nullptr)
          DR (res);

        res = new ArrayObject (ev_idcs);
        IR (res);
      }
      break;

    case ExprType::ExprDict:
      {
        DictExpr *de = static_cast<DictExpr *> (e);
        std::map<std::string, Object *> ev_idcs;

        for (auto &&i : de->get_vals ())
          {
            /**
             * expr_eval returns an object that with one
             * extra incremented ref_count which
             * we can use for value in dictionary
             * (see rule for arrays)
             */

            Object *keval;
            TC (keval = expr_eval (mod, i.first));

            assert (OBJ_IS_STR (keval)
                    && "Dictionaries only support string keys.");

            char *k
                = static_cast<StringConstant *> (
                      static_cast<ConstantObject *> (keval)->get_c ().get ())
                      ->get_value ()
                      .c_str ();

            TC (ev_idcs[std::string (k)] = expr_eval (mod, i.second));
            AMBIG_CHECK (ev_idcs[std::string (k)],
                         { res = ev_idcs[std::string (k)]; });

            delete[] k;
            DR (keval);
          }

        if (res != nullptr)
          DR (res);

        res = new DictObject (ev_idcs);
        IR (res);
      }
      break;

    case ExprType::ArrayAccess:
      {
        ArrayAccess *ac = static_cast<ArrayAccess *> (e);

        Object *arr_eval;
        TC (arr_eval = expr_eval (mod, ac->get_arr ()));
        AMBIG_CHECK (arr_eval, {});

        Object *idx_eval;
        TC (idx_eval = expr_eval (mod, ac->get_idx ()));
        AMBIG_CHECK (idx_eval, {});

        switch (arr_eval->get_type ())
          {
          case ObjectType::ArrayObj:
            {
              ArrayObject *ao = static_cast<ArrayObject *> (arr_eval);
              if (OBJ_IS_INT (idx_eval))
                {
                  Constant *ci = static_cast<Constant *> (
                      static_cast<ConstantObject *> (idx_eval)
                          ->get_c ()
                          .get ());

                  int idx = static_cast<IntegerConstant *> (ci)->get_value ();

                  while (idx < 0)
                    idx += ao->get_vals ().get_size ();

                  assert (idx < ao->get_vals ().get_size ()
                          && "Array index out of bounds.");

                  if (res != nullptr)
                    DR (res);

                  res = ao->get_vals ()[idx];
                  IR (res);
                }
              else if (idx_eval->get_type () == ObjectType::ArrayObj)
                {
                  Vec<Object *> rvals;

                  ArrayObject *iao = static_cast<ArrayObject *> (idx_eval);
                  Vec<Object *> &viao = iao->get_vals ();

                  for (Object *&k : viao)
                    {
                      assert (OBJ_IS_INT (k));

                      int kv = static_cast<IntegerConstant *> (
                                   static_cast<ConstantObject *> (k)
                                       ->get_c ()
                                       .get ())
                                   ->get_value ();

                      if (kv >= ao->get_vals ().get_size ())
                        break;

                      rvals.push_back (ao->get_vals ()[kv]);
                      IR (ao->get_vals ()[kv]);
                    }

                  if (res != nullptr)
                    DR (res);

                  res = static_cast<Object *> (new ArrayObject (rvals));
                  IR (res);
                }
              else
                {
                  std::cerr << "Invalid array access. Exiting..." << std::endl;
                  exit (EXIT_FAILURE);
                }
            }
            break;
          case ObjectType::DictObj:
            {
              DictObject *dobj = static_cast<DictObject *> (arr_eval);
              assert (OBJ_IS_STR (idx_eval));

              std::string k (static_cast<StringConstant *> (
                                 static_cast<ConstantObject *> (idx_eval)
                                     ->get_c ()
                                     .get ())
                                 ->get_value ()
                                 .get_internal_buffer ());

              assert (dobj->get_vals ().find (k) != dobj->get_vals ().end ());

              if (res != nullptr)
                DR (res);

              res = dobj->get_vals ()[k];
              IR (res);
            }
            break;

          case ObjectType::Constant:
            {
              assert (OBJ_IS_STR (arr_eval));

              StringConstant *sc
                  = static_cast<StringConstant *> (static_cast<Constant *> (
                      static_cast<ConstantObject *> (arr_eval)
                          ->get_c ()
                          .get ()));

              Str &s = sc->get_value ();

              switch (idx_eval->get_type ())
                {
                case ObjectType::Constant:
                  {
                    assert (OBJ_IS_INT (idx_eval));

                    int iv = static_cast<IntegerConstant *> (
                                 static_cast<ConstantObject *> (idx_eval)
                                     ->get_c ()
                                     .get ())
                                 ->get_value ();

                    assert (iv < s.size () && iv >= 0
                            && "String index out of range");

                    if (res != nullptr)
                      DR (res);

                    res = static_cast<Object *> (new ConstantObject (
                        static_cast<Constant *> (new StringConstant (s[iv]))));

                    IR (res);
                  }
                  break;
                case ObjectType::ArrayObj:
                  {
                    Str rs{ "" };

                    ArrayObject *iao = static_cast<ArrayObject *> (idx_eval);
                    Vec<Object *> &viao = iao->get_vals ();

                    for (Object *&k : viao)
                      {
                        assert (OBJ_IS_INT (k));

                        int kv = static_cast<IntegerConstant *> (
                                     static_cast<ConstantObject *> (k)
                                         ->get_c ()
                                         .get ())
                                     ->get_value ();

                        if (kv >= s.size ())
                          break;

                        rs.push_back (s[kv]);
                      }

                    if (res != nullptr)
                      DR (res);

                    res = static_cast<Object *> (new ConstantObject (
                        static_cast<Constant *> (new StringConstant (rs))));

                    IR (res);
                  }
                  break;

                default:
                  {
                    ERRMSG ("Invalid index type '"
                            << int (idx_eval->get_type ()) << "'");
                  }
                  break;
                }
            }
            break;

          default:
            std::cerr << "Cannot overload [] on type "
                      << (int)arr_eval->get_type () << ". Exiting..."
                      << std::endl;
            exit (EXIT_FAILURE);
            break;
          }

        DR (arr_eval);
        DR (idx_eval);
      }
      break;

    case ExprType::ExprToStep:
      {
        ToStepClause *tsc = static_cast<ToStepClause *> (e);

        Object *lv_eval;
        TC (lv_eval = expr_eval (mod, tsc->get_lval ()));
        AMBIG_CHECK (lv_eval, {
          /**
           * mod already had an increase
           * in ref_count as it was assigned
           * ambig.
           * Here we reclaim before reassigning
           * the same ambig
           * In future, there might be
           * a better way to not reassign
           * ambig over and over
           * One approach could be
           * to check before assigning,
           * if an ambig already is associated
           * with mod.
           * If yes, then we free it, and assign the
           * new ambig.
           * This, however, violates the principle
           * that ambig should stop program execution
           * Currently, we use the simple workaround:
           * reduce (ref_count) and reassign (ambig)
           */
          DR (lv_eval);
          res = lv_eval;
        });

        Object *rv_eval;
        TC (rv_eval = expr_eval (mod, tsc->get_rval ()));
        AMBIG_CHECK (rv_eval, {
          DR (lv_eval);
          DR (rv_eval);
          res = rv_eval;
        });

        Object *step_eval;
        TC (step_eval = tsc->get_step () != nullptr
                            ? expr_eval (mod, tsc->get_step ())
                            : nullptr);

        if (step_eval != nullptr)
          AMBIG_CHECK (step_eval, {
            DR (step_eval);
            DR (lv_eval);
            DR (rv_eval);
            res = step_eval;
          });

        assert (OBJ_IS_INT (lv_eval) && OBJ_IS_INT (rv_eval));
        if (step_eval != nullptr)
          assert (OBJ_IS_INT (step_eval));

        int lvi = static_cast<IntegerConstant *> (
                      static_cast<ConstantObject *> (lv_eval)->get_c ().get ())
                      ->get_value ();

        int rvi = static_cast<IntegerConstant *> (
                      static_cast<ConstantObject *> (rv_eval)->get_c ().get ())
                      ->get_value ();

        int stepi = step_eval != nullptr
                        ? static_cast<IntegerConstant *> (
                              static_cast<ConstantObject *> (step_eval)
                                  ->get_c ()
                                  .get ())
                              ->get_value ()
                        : 1;

        Vec<Object *> arr;

        for (int j = lvi; j < rvi; j += stepi)
          {
            Object *v = static_cast<Object *> (new ConstantObject (
                static_cast<Constant *> (new IntegerConstant (j))));

            /**
             * Since we are not using expr_eval
             * we need to manually increase ref_count
             * of v
             */
            IR (v);

            arr.push_back (v);
          }

        if (res != nullptr)
          DR (res);

        res = new ArrayObject (arr);
        IR (res);

        DR (lv_eval);
        DR (rv_eval);

        if (step_eval != nullptr)
          DR (step_eval);
        // res = static_cast<Object *> (new AmbigObject (nullptr));
        // AMBIG_CHECK (res, {});
      }
      break;
    case ExprType::ExprArith:
      {
        Arithmetic *a = static_cast<Arithmetic *> (e);
        Vec<AVBase *> &vals = a->get_vals ();
        Vec<Object *> st;

        for (AVBase *&j : vals)
          {
            if (j->get_type () == AVTypeEnum::Operator)
              {
                assert (st.get_size () > 1);

                /* l1 op l2 */
                Object *l2 = st.pop_back ();
                Object *l1 = st.pop_back ();

                AVOperator *op = static_cast<AVOperator *> (j);
                Object *res_op = nullptr;

                if (OBJ_IS_NUMBER (l1) && OBJ_IS_NUMBER (l2))
                  {
                    ConstantObject *l1co = static_cast<ConstantObject *> (l1);
                    ConstantObject *l2co = static_cast<ConstantObject *> (l2);

                    float val1 = l1co->get_c ().get ()->get_type ()
                                         == ConstantType::Float
                                     ? static_cast<FloatConstant *> (
                                           l1co->get_c ().get ())
                                           ->get_value ()
                                     : static_cast<IntegerConstant *> (
                                           l1co->get_c ().get ())
                                           ->get_value ();

                    float val2 = l2co->get_c ().get ()->get_type ()
                                         == ConstantType::Float
                                     ? static_cast<FloatConstant *> (
                                           l2co->get_c ().get ())
                                           ->get_value ()
                                     : static_cast<IntegerConstant *> (
                                           l2co->get_c ().get ())
                                           ->get_value ();

                    float result = 0.0f;
                    bool cast_int = OBJ_IS_INT (l1) && OBJ_IS_INT (l2);

                    switch (op->get_op ()[0])
                      {
                      case '+':
                        result = val1 + val2;
                        break;
                      case '-':
                        result = val1 - val2;
                        break;
                      case '*':
                        result = val1 * val2;
                        break;
                      case '/':
                        cast_int = false;
                        if (val2 == 0)
                          throw "division_by_zero";
                        result = val1 / val2;
                        break;
                      default:
                        std::cout << op->get_op () << '\n';
                        throw "unsupported_float_operation";
                      }

                    if (cast_int && result != int (result)) /* no data loss */
                      cast_int = false;

                    if (cast_int)
                      res_op = static_cast<Object *> (
                          new ConstantObject (static_cast<Constant *> (
                              new IntegerConstant (int (result)))));
                    else
                      res_op = static_cast<Object *> (
                          new ConstantObject (static_cast<Constant *> (
                              new FloatConstant (result))));
                  }
                else if (OBJ_IS_STR (l1) && OBJ_IS_STR (l2)
                         && op->get_op ()[0] == '+')
                  {
                    std::string val1 = static_cast<StringConstant *> (
                                           static_cast<ConstantObject *> (l1)
                                               ->get_c ()
                                               .get ())
                                           ->get_value ()
                                           .get_internal_buffer ();
                    std::string val2 = static_cast<StringConstant *> (
                                           static_cast<ConstantObject *> (l2)
                                               ->get_c ()
                                               .get ())
                                           ->get_value ()
                                           .get_internal_buffer ();

                    res_op = static_cast<Object *> (
                        new ConstantObject (static_cast<Constant *> (
                            new StringConstant ((val1 + val2).c_str ()))));
                  }
                else
                  {
                    l1->print ();
                    std::cout << '\n';
                    l2->print ();
                    std::cout << '\n';

                    throw "unsupported_arithmetic_operation";
                  }

                IR (res_op);
                DR (l1);
                DR (l2);

                st.push_back (res_op);
              }
            else
              {
                st.push_back (
                    expr_eval (mod, static_cast<AVOperand *> (j)->get_val ()));
                AMBIG_CHECK (st.back (), {
                  for (Object *&i : st)
                    DR (i);
                  res = st.back ();
                });
              }
          }

        res = st.pop_back ();
        /**
         * DO NOT DO IR(res)
         * st has already done it once.
         * we will use that reference.
         */
      }
      break;

    case ExprType::FuncCall:
      {
        FuncCallExpr *fce = static_cast<FuncCallExpr *> (e);

        Object *name_eval;

        TC (name_eval = expr_eval (mod, fce->get_name ()));
        AMBIG_CHECK (name_eval, {
          DR (res);
          res = name_eval;
        });
        // std::cout << (name_eval->get_self_arg () == nullptr) << '\n';

        Vec<Object *> args_eval;

        for (auto j : fce->get_args ())
          {
            Object *t = nullptr;
            TC (t = expr_eval (mod, j));
            AMBIG_CHECK (t, {
              DR (name_eval);
              DR (t);
              res = t;
            });

            /**
             * We could use DR here
             * but we do not need
             * to deallocate the object
             * so we only do 2 out of 3
             * steps of DR
             */
            D (t);
            _sfobj_removeownership (t);

            args_eval.push_back (t);
          }

        switch (name_eval->get_type ())
          {
          case ObjectType::FuncObject:
            {
              res = call_func (mod, name_eval, args_eval);
            }
            break;
          case ObjectType::SfClass:
            {
              res = call_func (mod, name_eval, args_eval);
            }
            break;

          case ObjectType::ModuleObject:
            {
              res = call_func (mod, name_eval, args_eval);
            }
            break;

          default:
            ERRMSG ("Entity with type " << (int)name_eval->get_type ()
                                        << " is not callable");
            break;
          }

        // for (auto o : args_eval)
        //   {
        //     R (o);
        //   }
        DR (name_eval);
      }
      break;

    case ExprType::DotAccess:
      {
        DotAccess *da = static_cast<DotAccess *> (e);

        Expr *&e_parent = da->get_parent ();
        assert (e_parent != nullptr);

        Expr *&e_child = da->get_child ();
        assert (e_child != nullptr
                && e_child->get_type () == ExprType::Variable);

        Str &member = static_cast<VariableExpr *> (e_child)->get_name ();

        Object *o_parent;
        TC (o_parent = expr_eval (mod, e_parent));
        AMBIG_CHECK (o_parent, {});
        // std::cout << "Parent type: " << int (o_parent->get_type ())
        //           << std::endl;

        switch (o_parent->get_type ())
          {
          case ObjectType::ClassObj:
            {
              ClassObject *co = static_cast<ClassObject *> (o_parent);

              if (!co->get_mod ()->has_variable (
                      member.get_internal_buffer ()))
                throw std::runtime_error (
                    (Str{ "Class does not have member " } + member)
                        .get_internal_buffer ());

              res = co->get_mod ()->get_variable (
                  member.get_internal_buffer ());

              IR (res);
              AMBIG_CHECK (res, {});
              // std::cout << int (res->get_type ()) << '\t'
              //           << static_cast<FunctionObject *> (res)
              //                  ->get_v ()
              //                  ->get_self_arg ()
              //           << '\n';

              if (res->get_type () == ObjectType::FuncObject
                  && static_cast<FunctionObject *> (res)
                         ->get_v ()
                         ->get_self_arg ())
                {
                  if (res->get_self_arg () != nullptr)
                    {
                      DR (res->get_self_arg ());
                      res->get_self_arg () = nullptr;
                    }

                  res->get_self_arg () = o_parent;
                  // std::cout << o_parent->get_ref_count () << '\n';
                  IR (o_parent);
                }
            }
            break;

          case ObjectType::SfClass:
            {
              SfClass *cl = static_cast<SfClass *> (o_parent);

              if (!cl->get_mod ()->has_variable (
                      member.get_internal_buffer ()))
                throw std::runtime_error (
                    (Str{ "Class does not have member " } + member)
                        .get_internal_buffer ());

              res = cl->get_mod ()->get_variable (
                  member.get_internal_buffer ());

              IR (res);
              AMBIG_CHECK (res, {});
            }
            break;

          case ObjectType::ModuleObject:
            {
              ModuleObject *mo = static_cast<ModuleObject *> (o_parent);
              Module *mo_mod = mo->get_mod ();

              if (!mo_mod->has_variable (member.get_internal_buffer ()))
                {
                  throw std::runtime_error (
                      (Str{ "Module does not have member " } + member)
                          .get_internal_buffer ());
                }

              res = mo_mod->get_variable (member.get_internal_buffer ());

              IR (res);
              AMBIG_CHECK (res, {});
            }
            break;

          case ObjectType::ArrayObj:
            {
              Str meth_name = Str{ "[]." } + member;

              if (!mod.has_variable (meth_name.get_internal_buffer ()))
                {
                  throw std::runtime_error (
                      (Str{ "No such native method found: " } + meth_name)
                          .get_internal_buffer ());
                }

              res = mod.get_variable (meth_name.get_internal_buffer ());

              IR (res);
              AMBIG_CHECK (res, {});

              if (res->get_type () == ObjectType::FuncObject
                  && static_cast<FunctionObject *> (res)
                         ->get_v ()
                         ->get_self_arg ())
                {
                  if (res->get_self_arg () != nullptr)
                    {
                      DR (res->get_self_arg ());
                      res->get_self_arg () = nullptr;
                    }

                  res->get_self_arg () = o_parent;

                  /**
                   * This leaks memory for some reason
                   * I figured constants and arrays don't need
                   * to tract their parent in self arg
                   */
                  // IR (o_parent);
                }
            }
            break;

          case ObjectType::Constant:
            {
              ConstantObject *co = static_cast<ConstantObject *> (o_parent);
              Constant *co_c = co->get_c ().get ();

              switch (co_c->get_type ())
                {
                case ConstantType::String:
                  {
                    Str meth_name = Str{ "''." } + member;

                    if (!mod.has_variable (meth_name.get_internal_buffer ()))
                      {
                        throw std::runtime_error (
                            (Str{ "No such native method found: " }
                             + meth_name)
                                .get_internal_buffer ());
                      }

                    res = mod.get_variable (meth_name.get_internal_buffer ());

                    IR (res);
                    AMBIG_CHECK (res, {});

                    if (res->get_type () == ObjectType::FuncObject
                        && static_cast<FunctionObject *> (res)
                               ->get_v ()
                               ->get_self_arg ())
                      {
                        if (res->get_self_arg () != nullptr)
                          {
                            DR (res->get_self_arg ());
                            res->get_self_arg () = nullptr;
                          }

                        res->get_self_arg () = o_parent;
                        // IR (o_parent);
                      }
                  }
                  break;

                case ConstantType::Integer:
                  {
                    Str meth_name = Str{ "0." } + member;

                    if (!mod.has_variable (meth_name.get_internal_buffer ()))
                      {
                        throw std::runtime_error (
                            (Str{ "No such native method found: " }
                             + meth_name)
                                .get_internal_buffer ());
                      }

                    res = mod.get_variable (meth_name.get_internal_buffer ());

                    IR (res);
                    AMBIG_CHECK (res, {});

                    if (res->get_type () == ObjectType::FuncObject
                        && static_cast<FunctionObject *> (res)
                               ->get_v ()
                               ->get_self_arg ())
                      {
                        if (res->get_self_arg () != nullptr)
                          {
                            DR (res->get_self_arg ());
                            res->get_self_arg () = nullptr;
                          }

                        res->get_self_arg () = o_parent;
                        // IR (o_parent);
                      }
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

        DR (o_parent);
      }
      break;

    case ExprType::LogicalAnd:
      {
        LogicalAndExpr *lae = static_cast<LogicalAndExpr *> (e);

        Expr *&left = lae->get_left ();
        Expr *&right = lae->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {});

        if (res != nullptr)
          DR (res);

        if (_sfobj_isfalse (mod, o_left))
          res = o_left;
        else
          {
            TC (o_right = expr_eval (mod, right));
            res = o_right;
          }

        IR (res);
        AMBIG_CHECK (res, {});

        if (o_right != nullptr)
          DR (o_right);
        if (o_left != nullptr)
          DR (o_left);
      }
      break;

    case ExprType::LogicalOr:
      {
        LogicalOrExpr *loe = static_cast<LogicalOrExpr *> (e);

        Expr *&left = loe->get_left ();
        Expr *&right = loe->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {});

        if (res != nullptr)
          DR (res);

        if (_sfobj_isfalse (mod, o_left))
          {
            TC (o_right = expr_eval (mod, right));
            res = o_right;
          }
        else
          res = o_left;

        IR (res);
        AMBIG_CHECK (res, {});

        if (o_right != nullptr)
          DR (o_right);
        if (o_left != nullptr)
          DR (o_left);
      }
      break;

    case ExprType::LogicalNot:
      {
        LogicalNotExpr *lne = static_cast<LogicalNotExpr *> (e);

        Expr *&val = lne->get_val ();
        Object *o_val = nullptr;

        TC (o_val = expr_eval (mod, val));
        AMBIG_CHECK (o_val, {});

        if (_sfobj_isfalse (mod, o_val))
          res = static_cast<Object *> (new ConstantObject (
              static_cast<Constant *> (new BooleanConstant (true))));
        else
          res = static_cast<Object *> (new ConstantObject (
              static_cast<Constant *> (new BooleanConstant (false))));

        IR (res);
        DR (o_val);
      }
      break;

    case ExprType::BitLeftShift:
      {
        BitLeftShiftExpr *blse = static_cast<BitLeftShiftExpr *> (e);

        Expr *left = blse->get_left ();
        Expr *right = blse->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {});

        TC (o_right = expr_eval (mod, right));
        AMBIG_CHECK (o_right, {});

        assert (o_left && OBJ_IS_INT (o_left));
        assert (o_right && OBJ_IS_INT (o_right));

        int lv = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (o_left)->get_c ().get ())
                     ->get_value ();

        int rv = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (o_right)->get_c ().get ())
                     ->get_value ();

        int rs = lv << rv;
        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new IntegerConstant (rs))));

        IR (res);
        DR (o_left);
        DR (o_right);
      }
      break;

    case ExprType::BitRightShift:
      {
        BitRightShiftExpr *blse = static_cast<BitRightShiftExpr *> (e);

        Expr *left = blse->get_left ();
        Expr *right = blse->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {});

        TC (o_right = expr_eval (mod, right));
        AMBIG_CHECK (o_right, {});

        assert (o_left && OBJ_IS_INT (o_left));
        assert (o_right && OBJ_IS_INT (o_right));

        int lv = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (o_left)->get_c ().get ())
                     ->get_value ();

        int rv = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (o_right)->get_c ().get ())
                     ->get_value ();

        int rs = lv >> rv;
        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new IntegerConstant (rs))));

        IR (res);
        DR (o_left);
        DR (o_right);
      }
      break;

    case ExprType::BitAnd:
      {
        BitAndExpr *blse = static_cast<BitAndExpr *> (e);

        Expr *left = blse->get_left ();
        Expr *right = blse->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {
          res = o_left;
          DR (o_left);
        });

        TC (o_right = expr_eval (mod, right));
        AMBIG_CHECK (o_right, {
          DR (o_left);
          res = o_right;
          DR (o_right);
        });

        assert (o_left && (OBJ_IS_INT (o_left) || OBJ_IS_BOOL (o_left)));
        assert (o_right && (OBJ_IS_INT (o_right) || OBJ_IS_BOOL (o_right)));

        int lv = OBJ_IS_INT (o_left)
                     ? static_cast<IntegerConstant *> (
                           static_cast<ConstantObject *> (o_left)
                               ->get_c ()
                               .get ())
                           ->get_value ()
                     : static_cast<BooleanConstant *> (
                           static_cast<ConstantObject *> (o_left)
                               ->get_c ()
                               .get ())
                           ->get_value ();

        int rv = OBJ_IS_INT (o_right)
                     ? static_cast<IntegerConstant *> (
                           static_cast<ConstantObject *> (o_right)
                               ->get_c ()
                               .get ())
                           ->get_value ()
                     : static_cast<BooleanConstant *> (
                           static_cast<ConstantObject *> (o_right)
                               ->get_c ()
                               .get ())
                           ->get_value ();

        int rs = lv & rv;
        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new IntegerConstant (rs))));

        IR (res);
        DR (o_left);
        DR (o_right);
      }
      break;

    case ExprType::BitOr:
      {
        BitOrExpr *blse = static_cast<BitOrExpr *> (e);

        Expr *left = blse->get_left ();
        Expr *right = blse->get_right ();

        Object *o_left = nullptr;
        Object *o_right = nullptr;

        TC (o_left = expr_eval (mod, left));
        AMBIG_CHECK (o_left, {
          res = o_left;
          DR (o_left);
        });

        TC (o_right = expr_eval (mod, right));
        AMBIG_CHECK (o_right, {
          DR (o_left);
          res = o_right;
          DR (o_right);
        });

        assert (o_left && (OBJ_IS_INT (o_left) || OBJ_IS_BOOL (o_left)));
        assert (o_right && (OBJ_IS_INT (o_right) || OBJ_IS_BOOL (o_right)));

        int lv = OBJ_IS_INT (o_left)
                     ? static_cast<IntegerConstant *> (
                           static_cast<ConstantObject *> (o_left)
                               ->get_c ()
                               .get ())
                           ->get_value ()
                     : static_cast<BooleanConstant *> (
                           static_cast<ConstantObject *> (o_left)
                               ->get_c ()
                               .get ())
                           ->get_value ();

        int rv = OBJ_IS_INT (o_right)
                     ? static_cast<IntegerConstant *> (
                           static_cast<ConstantObject *> (o_right)
                               ->get_c ()
                               .get ())
                           ->get_value ()
                     : static_cast<BooleanConstant *> (
                           static_cast<ConstantObject *> (o_right)
                               ->get_c ()
                               .get ())
                           ->get_value ();

        int rs = lv | rv;
        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new IntegerConstant (rs))));

        IR (res);
        DR (o_left);
        DR (o_right);
      }
      break;

    case ExprType::BitNegate:
      {
        BitNegateExpr *bne = static_cast<BitNegateExpr *> (e);
        Expr *v = bne->get_val ();
        Object *o_v = nullptr;

        TC (o_v = expr_eval (mod, v));
        if (o_v)
          AMBIG_CHECK (o_v, {
            res = o_v;
            DR (o_v);
          });
        assert (o_v && OBJ_IS_INT (o_v));

        int vv = static_cast<IntegerConstant *> (
                     static_cast<ConstantObject *> (o_v)->get_c ().get ())
                     ->get_value ();

        int rv = ~vv;

        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ConstantObject (
            static_cast<Constant *> (new IntegerConstant (rv))));

        IR (res);
        DR (o_v);
      }
      break;

    case ExprType::Repeat:
      {
        RepeatExpr *re = static_cast<RepeatExpr *> (e);
        Expr *times = re->get_times ();
        Expr *body = re->get_body ();

        Object *o_times = nullptr;
        Object *o_body = nullptr;

        TC (o_times = expr_eval (mod, times));
        AMBIG_CHECK (o_times, {
          res = o_times;
          DR (o_times);
        });

        TC (o_body = expr_eval (mod, body));
        AMBIG_CHECK (o_body, {
          res = o_body;
          DR (o_body);
        });

        assert (o_times && OBJ_IS_INT (o_times));

        int t = static_cast<IntegerConstant *> (
                    static_cast<ConstantObject *> (o_times)->get_c ().get ())
                    ->get_value ();

        Vec<Object *> vls;

        for (int i = 0; i < t; i++)
          {
            IR (o_body);
            vls.push_back (o_body);
          }

        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ArrayObject (vls));
        IR (res);

        DR (o_times);
        DR (o_body);
      }
      break;

    case ExprType::InlineFor:
      {
        InlineForExpr *ife = static_cast<InlineForExpr *> (e);
        Expr *iterable = ife->get_iterable ();
        Expr *body = ife->get_body ();
        Vec<Expr *> &vars = ife->get_vars ();

        Object *o_iterable = nullptr;

        TC (o_iterable = expr_eval (mod, iterable));
        AMBIG_CHECK (o_iterable, {
          res = o_iterable;
          DR (o_iterable);
        });

        Vec<Object *> res_objs;

        switch (o_iterable->get_type ())
          {
          case ObjectType::ArrayObj:
            {
              ArrayObject *ao = static_cast<ArrayObject *> (o_iterable);
              Vec<Object *> ao_vals = ao->get_vals ();

              for (Object *&av : ao_vals)
                {
                  if (vars.get_size () == 1)
                    {
                      Expr *v = vars[0];

                      switch (v->get_type ())
                        {
                        case ExprType::Variable:
                          {
                            char *p = static_cast<VariableExpr *> (v)
                                          ->get_name ()
                                          .c_str ();
                            mod.set_variable (p, av);

                            delete[] p;
                          }
                          break;

                        default:
                          break;
                        }
                    }
                  else
                    {
                      switch (av->get_type ())
                        {
                        case ObjectType::ArrayObj:
                          {
                            ArrayObject *avo = static_cast<ArrayObject *> (av);

                            assert (avo->get_vals ().get_size ()
                                    >= vars.get_size ());

                            if (avo->get_vals ().get_size ()
                                == vars.get_size ())
                              {
                                /* == */

                                size_t vi = 0;
                                for (Expr *&vv : vars)
                                  {
                                    switch (vv->get_type ())
                                      {
                                      case ExprType::Variable:
                                        {
                                          char *p
                                              = static_cast<VariableExpr *> (
                                                    vv)
                                                    ->get_name ()
                                                    .c_str ();

                                          mod.set_variable (
                                              p, avo->get_vals ()[vi++]);

                                          delete[] p;
                                        }
                                        break;

                                      default:
                                        break;
                                      }
                                  }
                              }
                            else
                              {
                                /* > */
                              }
                          }
                          break;

                        default:
                          break;
                        }
                    }

                  // mod_exec (mod);
                  res_objs.push_back (expr_eval (
                      mod, body)); /* consume obj_count given by expr_eval */

                  AMBIG_CHECK (res_objs.back (), {
                    res = res_objs.back ();
                    DR (res_objs.back ());
                    DR (o_iterable);
                  });
                }
            }
            break;

          case ObjectType::DictObj:
            {
              DictObject *dobj = static_cast<DictObject *> (o_iterable);
              std::map<std::string, Object *> dobj_vals = dobj->get_vals ();

              for (auto &&i : dobj_vals)
                {
                  Object *kobj = static_cast<Object *> (
                      new ConstantObject (static_cast<Constant *> (
                          new StringConstant (i.first.c_str ()))));

                  IR (kobj);

                  assert (vars.get_size () == 1);
                  Expr *v = vars[0];

                  switch (v->get_type ())
                    {
                    case ExprType::Variable:
                      {
                        char *p = static_cast<VariableExpr *> (v)
                                      ->get_name ()
                                      .c_str ();
                        mod.set_variable (p, kobj);

                        delete[] p;
                      }
                      break;

                    default:
                      break;
                    }

                  // mod_exec (mod);
                  res_objs.push_back (expr_eval (mod, body));
                  AMBIG_CHECK (res_objs.back (), {
                    res = res_objs.back ();
                    DR (res_objs.back ());

                    DR (kobj);
                    DR (o_iterable);
                  });
                  DR (kobj);
                }
            }
            break;

          default:
            break;
          }

        DR (o_iterable);

        if (res != nullptr)
          DR (res);

        res = static_cast<Object *> (new ArrayObject (res_objs));
        IR (res);
      }
      break;

    case ExprType::TryCatch:
      {
        TryCatchExpr *tce = static_cast<TryCatchExpr *> (e);

        Expr *e_try = tce->get_tryexpr ();
        Expr *e_catch_var = tce->get_catchcvar ();
        Expr *e_catch = tce->get_catchexpr ();

        Object *ev = expr_eval (mod, e_try);

        if (OBJ_IS_AMBIG (ev))
          {
            mod.get_continue_exec () = true;
            DR (mod.get_ambig ());
            mod.get_ambig () = nullptr;

            AmbigObject *ao = static_cast<AmbigObject *> (ev);

            Object *aov = ao->get_val ();
            ao->get_val () = nullptr;
            DR (ev);

            if (e_catch != nullptr)
              {
                if (e_catch_var == nullptr)
                  {
                    res = expr_eval (mod, e_catch);
                  }
                else
                  {
                    assert (e_catch_var->get_type () == ExprType::Variable);
                    Str &vn = static_cast<VariableExpr *> (e_catch_var)
                                  ->get_name ();

                    Module *m = new Module (ModuleType::File);
                    m->set_variable (vn.get_internal_buffer (), aov);
                    m->set_parent (&mod);

                    res = expr_eval (*m, e_catch);

                    m->set_parent (nullptr);
                    delete m;
                  }
              }
            else
              {
                res = static_cast<Object *> (new ConstantObject (
                    static_cast<Constant *> (new NoneConstant ())));

                IR (res);
              }

            DR (aov);
          }
        else
          {
            res = ev;
          }

        /**
         * * Note
         * do not call IR (res)
         * we will reuse the ref count obtained from expr_eval calls.
         * Where there are no such calls, IR will be done separately
         */
      }
      break;

    default:
      std::cerr << "invalid expr type: " << (int)e->get_type () << std::endl;
      break;
    }

  if (res == nullptr)
    {
      std::cout << "res_expr_is_null";
      throw "res_expr_is_null";
    }

  if (!res->get_ref_count ())
    {
      std::cout << "res->ref_count is 0, possible seg_fault. Did you call IR "
                   "(res)?\n";
    }

  if (OBJ_IS_AMBIG (res))
    {
      mod.get_continue_exec () = false;
      mod.get_ambig () = res;

      IR (res);
    }

  goto ret;

ambig_test:; /* skip by default */
  // mod.get_continue_exec () = false;
  // mod.get_ambig () = res;
  /**
   * we received an intermediate
   * ambig which needs to be highlighted
   * (like in an array index)
   * So we replace res with an ambig
   * constant and return it
   */
  if (res != nullptr)
    {
      if (!OBJ_IS_AMBIG (res))
        {
          res = static_cast<Object *> (new AmbigObject (nullptr));
        }
      // else
      //   here;
    }
  else
    {
      res = static_cast<Object *> (new AmbigObject (nullptr));
    }

  if (!res->get_ref_count ())
    IR (res);

  mod.get_continue_exec () = false;
  mod.get_ambig () = res;
  IR (res);

  // std::cout << res->get_ref_count () << '\n';

ret:
  return res;
}

bool
Module::has_variable (std::string rhs)
{
  if (!vtable.count (rhs))
    return parent == nullptr ? false : parent->has_variable (rhs);

  return true;
}

Object *
call_func (Module &mod, Object *fname, Vec<Object *> &fargs,
           Object *__self_Arg)
{
  Module *nmod = new Module (ModuleType::Function);
  Object *res = nullptr;

  switch (fname->get_type ())
    {
    case ObjectType::FuncObject:
      {
        FunctionObject *fo = static_cast<FunctionObject *> (fname);
        Function *&fv = fo->get_v ();

        switch (fv->get_type ())
          {
          case FuncType::Coded:
            {
              CodedFunction *cf = static_cast<CodedFunction *> (fv);
              if (!cf->get_va_args ())
                {
                  if (__self_Arg == nullptr)
                    assert (cf->get_args ().get_size ()
                            == fargs.get_size () + fv->get_self_arg ());
                  else
                    {
                      assert (cf->get_args ().get_size ()
                              == fargs.get_size () + 1);
                    }
                }
              else
                assert (cf->get_args ().get_size ()
                        > 0); /* at least one arg */

              Object *self_arg = __self_Arg;

              if (self_arg != nullptr)
                {
                  IR (self_arg);
                  fargs.insert (0, self_arg);

                  switch (self_arg->get_type ())
                    {
                    case ObjectType::ClassObj:
                      {
                        ClassObject *co
                            = static_cast<ClassObject *> (self_arg);

                        Module *co_mod = co->get_mod ();
                        nmod->set_parent (co_mod->get_parent ());
                      }
                      break;

                    case ObjectType::Constant:
                    case ObjectType::ArrayObj:
                      {
                        nmod->set_parent (cf->get_parent ());
                      }
                      break;

                    default:
                      assert (0 && "TODO");
                      break;
                    }
                }
              else
                {
                  if (fv->get_self_arg ())
                    {
                      assert (fname->get_self_arg () != nullptr);
                      fargs.insert (0, fname->get_self_arg ());
                      self_arg = fname->get_self_arg ();
                      IR (self_arg);

                      switch (self_arg->get_type ())
                        {
                        case ObjectType::ClassObj:
                          {
                            ClassObject *co
                                = static_cast<ClassObject *> (self_arg);

                            Module *co_mod = co->get_mod ();
                            nmod->set_parent (co_mod->get_parent ());
                          }
                          break;

                        case ObjectType::Constant:
                        case ObjectType::ArrayObj:
                          {
                            nmod->set_parent (cf->get_parent ());
                          }
                          break;

                        default:
                          assert (0 && "TODO");
                          break;
                        }
                    }
                  else
                    nmod->set_parent (cf->get_parent ());
                }

              nmod->get_stmts () = cf->get_body ();

              size_t j = 0;
              for (Expr *&a : cf->get_args ())
                {
                  switch (a->get_type ())
                    {
                    case ExprType::Variable:
                      {
                        char *p = static_cast<VariableExpr *> (a)
                                      ->get_name ()
                                      .c_str ();
                        nmod->set_variable (p, fargs[j++]);

                        delete[] p;
                      }
                      break;

                    default:
                      break;
                    }
                }

              mod_exec (*nmod);

              if (nmod->get_saw_ambig ())
                {
                  // here;
                  // std::cout << nmod->get_ambig ()->get_ref_count
                  // ()
                  //           << '\n';
                  mod.get_ambig () = nmod->get_ambig ();

                  for (int i : nmod->get_backtrace ())
                    {
                      mod.get_backtrace ().push_back (i);
                    }

                  IR (mod.get_ambig ());
                  mod.get_saw_ambig () = true;

                  delete nmod;
                  // here;
                  // std::cout << mod.get_ambig ()->get_ref_count ()
                  //           << '\n';
                  // DR (mod.get_ambig ());
                }

              if (__self_Arg != nullptr)
                {
                  DR (self_arg);
                }
              else
                {
                  if (fv->get_self_arg ())
                    DR (self_arg);
                }

              Object *ret = nmod->get_ret ();

              if (nmod->get_ret () == nullptr)
                {
                  ret = static_cast<Object *> (new ConstantObject (
                      static_cast<Constant *> (new NoneConstant ())));
                }

              res = ret;
              IR (ret);
            }
            break;

          case FuncType::Native:
            {
              NativeFunction *nf = static_cast<NativeFunction *> (fv);

              if (!nf->get_va_args ())
                {
                  assert (nf->get_args ().get_size ()
                          == fargs.get_size () + fv->get_self_arg ());
                }
              else
                assert (nf->get_args ().get_size ()
                        > 0); /* at least one arg */

              Object *self_arg = nullptr;

              if (fv->get_self_arg ())
                {
                  assert (fname->get_self_arg () != nullptr);
                  fargs.insert (0, fname->get_self_arg ());
                  self_arg = fname->get_self_arg ();
                  IR (self_arg);

                  switch (self_arg->get_type ())
                    {
                    case ObjectType::ClassObj:
                      {
                        ClassObject *co
                            = static_cast<ClassObject *> (self_arg);

                        Module *co_mod = co->get_mod ();
                        nmod->set_parent (co_mod->get_parent ());
                      }
                      break;

                    case ObjectType::Constant:
                    case ObjectType::ArrayObj:
                      {
                        nmod->set_parent (nf->get_parent ());
                      }
                      break;

                    default:
                      assert (0 && "TODO");
                      break;
                    }
                }
              else
                nmod->set_parent (nf->get_parent ());

              Module *fmod
                  = new Module (ModuleType::Function, Vec<Statement *> ());
              fmod->set_parent (&mod);

              if (nf->get_va_args ())
                {
                  /**
                   * Function uses variable arguments
                   */
                  Str arg_a = nf->get_args ()[nf->get_args ().get_size () - 1];

                  /* convert fargs to ArrayObject */
                  for (Object *&i : fargs)
                    IR (i); /* transfer back ownership */

                  ArrayObject *ao = new ArrayObject (fargs);

                  char *p = arg_a.c_str ();
                  fmod->set_variable (p, ao);

                  delete[] p;
                }
              else
                {
                  size_t j = 0;
                  for (Str &a : nf->get_args ())
                    {
                      char *p = a.c_str ();
                      fmod->set_variable (p, fargs[j++]);

                      delete[] p;
                    }
                }

              Object *ret;

              TC (ret = nf->call (fmod));
              res = ret;

              delete fmod;
            }

          default:
            break;
          }
      }
      break;

    case ObjectType::SfClass:
      {
        SfClass *sfc = static_cast<SfClass *> (fname);
        Module *&sm = sfc->get_mod ();

        Module *objm = new Module (ModuleType::Class);
        Object *co = static_cast<Object *> (new ClassObject (objm));
        IR (co); /* prevent from deallocation in _init */

        for (auto &&i : sm->get_vtable ())
          objm->set_variable (i.first, i.second);

        if (objm->has_variable ("_init"))
          {
            Object *f_init = objm->get_variable ("_init");

            if (!_sfobj_iscallable (*objm, f_init))
              throw std::runtime_error ("class._init is not callable");

            // args_eval.insert (0, co);
            objm->set_parent (sm->get_parent ());

            Object *ret = call_func (*sm->get_parent (), f_init, fargs, co);
            DR (ret);
          }

        res = co;
      }
      break;

    case ObjectType::ModuleObject:
      {
        ModuleObject *mo = static_cast<ModuleObject *> (fname);
        Module *&mo_mod = mo->get_mod ();

        if (mo_mod->has_variable ("_init"))
          {
            Object *mo_init = mo_mod->get_variable ("_init");

            if (mo_init->get_type ()
                != ObjectType::FuncObject) /* only functions for now
                                            */
              {
                ERRMSG ("Module._init is not a function");
              }

            FunctionObject *fo = static_cast<FunctionObject *> (mo_init);

            Function *fv = fo->get_v ();

            Object *o_init = call_func (*mo_mod, mo_init, fargs);
            res = o_init;
          }
        else
          {
            ERRMSG ("Module is not callable (lacks _init () method)");
          }
      }
      break;

    default:
      break;
    }

  delete nmod;

  if (res == nullptr)
    {
      res = static_cast<Object *> (
          new ConstantObject (static_cast<Constant *> (new NoneConstant ())));

      IR (res);
    }

  return res;
}
} // namespace sf
#include "module.hpp"

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
            // D (val_eval); /* this is done after the switch () */
            // _sfobj_removeownership (val_eval);

            // std::cout << val_eval->get_ref_count () << '\n';

            switch (ne->get_type ())
              {
              case ExprType::Variable:
                {
                  VariableExpr *nv = static_cast<VariableExpr *> (ne);

                  char *p = (char *)nv->get_name ().c_str ();
                  mod.set_variable (p, val_eval); /* set_variable takes care of
                                                     ref_count increment */

                  delete[] p;
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
                  TC (oidx = expr_eval (mod, idx));

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

                  Object *o_parent = expr_eval (mod, e_parent);

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

            Vec<Object *> args_eval;

            for (auto j : fst->get_args ())
              {
                Object *t = nullptr;
                TC (t = expr_eval (mod, j));

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
                  FunctionObject *fo
                      = static_cast<FunctionObject *> (name_eval);

                  Function *fv = fo->get_v ();

                  switch (fv->get_type ())
                    {
                    case FuncType::Native:
                      {
                        NativeFunction *nf
                            = static_cast<NativeFunction *> (fv);

                        if (!nf->get_va_args ())
                          assert (nf->get_args ().get_size ()
                                  == args_eval.get_size ()
                                         + fv->get_self_arg ());
                        else
                          assert (nf->get_args ().get_size ()
                                  > 0); /* at least one arg */

                        if (fv->get_self_arg ())
                          {
                            assert (name_eval->get_self_arg () != nullptr);
                            args_eval.insert (0, name_eval->get_self_arg ());
                          }

                        Module *fmod = new Module (ModuleType::Function,
                                                   Vec<Statement *> ());
                        fmod->set_parent (&mod);

                        if (nf->get_va_args ())
                          {
                            /**
                             * Function uses variable arguments
                             */
                            Str arg_a = nf->get_args ()[0];

                            /* convert args_eval to ArrayObject */
                            for (Object *&i : args_eval)
                              IR (i); /* transfer back ownership */

                            ArrayObject *ao = new ArrayObject (args_eval);

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
                                fmod->set_variable (p, args_eval[j++]);

                                delete[] p;
                              }
                          }

                        Object *ret;

                        TC (ret = nf->call (fmod));
                        DR (ret);

                        delete fmod;
                      }
                      break;

                    case FuncType::Coded:
                      {
                        CodedFunction *cf = static_cast<CodedFunction *> (fv);

                        // printf ("%d %d\n", cf->get_args ().get_size (),
                        //         args_eval.get_size ());
                        if (!cf->get_va_args ())
                          assert (cf->get_args ().get_size ()
                                  == args_eval.get_size ()
                                         + fv->get_self_arg ());
                        else
                          assert (cf->get_args ().get_size ()
                                  > 0); /* at least one arg */

                        if (fv->get_self_arg ())
                          {
                            assert (name_eval->get_self_arg () != nullptr);
                            args_eval.insert (0, name_eval->get_self_arg ());
                          }

                        Module *fmod = new Module (ModuleType::Function,
                                                   cf->get_body ());
                        fmod->set_parent (&mod);

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
                                  fmod->set_variable (p, args_eval[j++]);

                                  delete[] p;
                                }
                                break;

                              default:
                                break;
                              }
                          }

                        mod_exec (*fmod);

                        /**
                         * In statement side we
                         * do not really care about
                         * the return value of the function
                         */

                        delete fmod;
                      }
                      break;

                    default:
                      /* unreachable */
                      here;
                      exit (__LINE__);
                      break;
                    }
                }
                break;
              case ObjectType::SfClass:
                {
                  SfClass *sfc = static_cast<SfClass *> (name_eval);
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
                        throw std::runtime_error (
                            "class._init is not callable");

                      args_eval.insert (0, co);
                      objm->set_parent (sm->get_parent ());

                      switch (f_init->get_type ())
                        {
                        case ObjectType::FuncObject:
                          {
                            FunctionObject *fo
                                = static_cast<FunctionObject *> (f_init);

                            Function *fo_f = fo->get_v ();

                            switch (fo_f->get_type ())
                              {
                              case FuncType::Coded:
                                {
                                  CodedFunction *cf
                                      = static_cast<CodedFunction *> (fo_f);

                                  // printf ("%d %d\n", cf->get_args
                                  // ().get_size (),
                                  //         args_eval.get_size ());
                                  assert (cf->get_args ().get_size ()
                                          == args_eval.get_size ());

                                  Module *fmod = new Module (
                                      ModuleType::Function, cf->get_body ());
                                  fmod->set_parent (&mod);

                                  size_t j = 0;
                                  for (Expr *&a : cf->get_args ())
                                    {
                                      switch (a->get_type ())
                                        {
                                        case ExprType::Variable:
                                          {
                                            char *p
                                                = static_cast<VariableExpr *> (
                                                      a)
                                                      ->get_name ()
                                                      .c_str ();
                                            fmod->set_variable (
                                                p, args_eval[j++]);

                                            delete[] p;
                                          }
                                          break;

                                        default:
                                          break;
                                        }
                                    }

                                  mod_exec (*fmod);

                                  /**
                                   * In statement side we
                                   * do not really care about
                                   * the return value of the function
                                   */

                                  delete fmod;
                                }
                                break;

                              case FuncType::Native:
                                {
                                  NativeFunction *nf
                                      = static_cast<NativeFunction *> (fo_f);

                                  if (!nf->get_va_args ())
                                    assert (nf->get_args ().get_size ()
                                            == args_eval.get_size ());
                                  else
                                    assert (nf->get_args ().get_size ()
                                            > 0); /* at least one arg */

                                  Module *fmod
                                      = new Module (ModuleType::Function,
                                                    Vec<Statement *> ());
                                  fmod->set_parent (&mod);

                                  if (nf->get_va_args ())
                                    {
                                      /**
                                       * Function uses variable arguments
                                       */
                                      Str arg_a = nf->get_args ()[0];

                                      /* convert args_eval to ArrayObject */
                                      for (Object *&i : args_eval)
                                        IR (i); /* transfer back ownership
                                                 */

                                      ArrayObject *ao
                                          = new ArrayObject (args_eval);

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
                                          fmod->set_variable (p,
                                                              args_eval[j++]);

                                          delete[] p;
                                        }
                                    }

                                  Object *ret;

                                  TC (ret = nf->call (fmod));
                                  DR (ret);

                                  delete fmod;
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
                    }

                  /* in statement side, a class declaration is a dangling class
                   * allocation, so it can be destroyed as soon as it is
                   * declared */
                  DR (co);
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
                mod.get_ret () = expr_eval (mod, rt->get_val ());
                mod.get_continue_exec () = false;
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
                    throw "return_used_outside_function";
                  }

                goto func_end;
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

            while (!_sfobj_isfalse (mod, cond_eval))
              {
                mod_exec (mod);
                DR (cond_eval);
                TC (cond_eval = expr_eval (mod, cond));
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
            assert (o_cond && OBJ_IS_INT (o_cond));

            int ov
                = static_cast<IntegerConstant *> (
                      static_cast<ConstantObject *> (o_cond)->get_c ().get ())
                      ->get_value ();

            while (ov > 0)
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

            try
              {
                mod_exec (*cmod);
              }
            catch (const std::exception &e)
              {
                here;
                std::cerr << e.what () << '\n';
              }

            SfClass *sfc = new SfClass (cds->get_name (), cmod);

            mod.set_variable (sfc->get_name ().get_internal_buffer (),
                              static_cast<Object *> (sfc));
          }
          break;

        default:
          std::cerr << "invalid type: " << (int)st->get_type () << std::endl;
          break;
        }

    end:
      i++;
    }

func_end:;
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
              res = static_cast<Object *> (new ConstantObject (
                  static_cast<Constant *> (new AmbigConstant ())));
            }
            break;

          default:
            break;
          }

        IR (res);
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

            IR (res);

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

        res = static_cast<Object *> (new FunctionObject (fe->get_v ().get ()));
        IR (res);
      }
      break;

    case ExprType::Conditional:
      {
        ConditionalExpr *ce = static_cast<ConditionalExpr *> (e);
        Object *lobj;
        TC (lobj = expr_eval (mod, ce->get_lval ()));
        Object *robj;
        TC (robj = expr_eval (mod, ce->get_rval ()));

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
        Object *idx_eval;
        TC (idx_eval = expr_eval (mod, ac->get_idx ()));

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

                  if (idx < 0)
                    idx = idx % ao->get_vals ().get_size ();

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
        Object *rv_eval;
        TC (rv_eval = expr_eval (mod, tsc->get_rval ()));
        Object *step_eval;
        TC (step_eval = tsc->get_step () != nullptr
                            ? expr_eval (mod, tsc->get_step ())
                            : nullptr);

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
              }
          }

        res = st.pop_back ();
        /**
         * DO NOT DO IR(res)
         * st already has done it once.
         * we will use that reference.
         */
      }
      break;

    case ExprType::FuncCall:
      {
        FuncCallExpr *fce = static_cast<FuncCallExpr *> (e);

        Object *name_eval;

        TC (name_eval = expr_eval (mod, fce->get_name ()));
        // std::cout << (name_eval->get_self_arg () == nullptr) << '\n';

        Vec<Object *> args_eval;

        for (auto j : fce->get_args ())
          {
            Object *t = nullptr;
            TC (t = expr_eval (mod, j));

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
              FunctionObject *fo = static_cast<FunctionObject *> (name_eval);

              Function *fv = fo->get_v ();

              switch (fv->get_type ())
                {
                case FuncType::Native:
                  {
                    NativeFunction *nf = static_cast<NativeFunction *> (fv);

                    if (!nf->get_va_args ())
                      assert (nf->get_args ().get_size ()
                              == args_eval.get_size () + fv->get_self_arg ());
                    else
                      assert (nf->get_args ().get_size ()
                              > 0); /* at least one arg */

                    if (fv->get_self_arg ())
                      {
                        assert (name_eval->get_self_arg () != nullptr);
                        args_eval.insert (0, name_eval->get_self_arg ());
                      }

                    Module *fmod = new Module (ModuleType::Function,
                                               Vec<Statement *> ());
                    fmod->set_parent (&mod);

                    if (nf->get_va_args ())
                      {
                        /**
                         * Function uses variable arguments
                         */
                        Str arg_a = nf->get_args ()[0];

                        /* convert args_eval to ArrayObject */
                        for (Object *&i : args_eval)
                          IR (i); /* transfer back ownership */

                        ArrayObject *ao = new ArrayObject (args_eval);

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
                            fmod->set_variable (p, args_eval[j++]);

                            delete[] p;
                          }
                      }

                    Object *ret;

                    TC (ret = nf->call (fmod));
                    // DR (ret);

                    res = ret;

                    /**
                     * nf->call returns an object with
                     * an increased refcount (to save ownership
                     * transfer during return)
                     * we will use that refcount for res.
                     * So we will not call an IR(X)/I(X) here
                     */

                    delete fmod;
                  }
                  break;

                case FuncType::Coded:
                  {
                    CodedFunction *cf = static_cast<CodedFunction *> (fv);

                    // printf ("%d %d\n", cf->get_args ().get_size (),
                    //         args_eval.get_size ());
                    if (!cf->get_va_args ())
                      assert (cf->get_args ().get_size ()
                              == args_eval.get_size () + fv->get_self_arg ());
                    else
                      assert (cf->get_args ().get_size ()
                              > 0); /* at least one arg */

                    if (fv->get_self_arg ())
                      {
                        assert (name_eval->get_self_arg () != nullptr);
                        args_eval.insert (0, name_eval->get_self_arg ());
                      }

                    Module *fmod
                        = new Module (ModuleType::Function, cf->get_body ());
                    fmod->set_parent (&mod);

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
                              fmod->set_variable (p, args_eval[j++]);

                              delete[] p;
                            }
                            break;

                          default:
                            break;
                          }
                      }

                    mod_exec (*fmod);

                    if (fmod->get_ret () != nullptr)
                      res = fmod->get_ret ();
                    else
                      {
                        Constant *p;
                        Expr *t = static_cast<Expr *> (
                            new ConstantExpr (static_cast<Constant *> (
                                p = new NoneConstant ())));
                        res = expr_eval (mod, t);

                        delete p;
                        delete t;
                      }

                    /**
                     * TODO: check whether coded functions
                     * return an increased refcount which
                     * we could possibly use.
                     *
                     * ? Check:
                     * ?  So I checked the code for ReturnStmt rule
                     * ?  and it uses expr_eval to evaluate return
                     * ?  object, so we can use that extra RCI
                     * ?  here.
                     *
                     * ! But:
                     * !  when we delete fmod the RCI is lost
                     * !  so we need proper ownership for res
                     * !  in this scope
                     *
                     * * Conclusion:
                     * *  Do call IR(res)/I(res) here
                     */
                    IR (res);

                    delete fmod;
                  }
                  break;

                default:
                  /* unreachable */
                  here;
                  exit (__LINE__);
                  break;
                }
            }
            break;
          case ObjectType::SfClass:
            {
              SfClass *sfc = static_cast<SfClass *> (name_eval);
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

                  args_eval.insert (0, co);
                  objm->set_parent (sm->get_parent ());

                  switch (f_init->get_type ())
                    {
                    case ObjectType::FuncObject:
                      {
                        FunctionObject *fo
                            = static_cast<FunctionObject *> (f_init);

                        Function *fo_f = fo->get_v ();

                        switch (fo_f->get_type ())
                          {
                          case FuncType::Coded:
                            {
                              CodedFunction *cf
                                  = static_cast<CodedFunction *> (fo_f);

                              // printf ("%d %d\n", cf->get_args
                              // ().get_size (),
                              //         args_eval.get_size ());
                              assert (cf->get_args ().get_size ()
                                      == args_eval.get_size ());

                              Module *fmod = new Module (ModuleType::Function,
                                                         cf->get_body ());
                              fmod->set_parent (&mod);

                              size_t j = 0;
                              for (Expr *&a : cf->get_args ())
                                {
                                  switch (a->get_type ())
                                    {
                                    case ExprType::Variable:
                                      {
                                        char *p
                                            = static_cast<VariableExpr *> (a)
                                                  ->get_name ()
                                                  .c_str ();
                                        fmod->set_variable (p, args_eval[j++]);

                                        delete[] p;
                                      }
                                      break;

                                    default:
                                      break;
                                    }
                                }

                              mod_exec (*fmod);

                              /**
                               * In statement side we
                               * do not really care about
                               * the return value of the function
                               */

                              delete fmod;
                            }
                            break;

                          case FuncType::Native:
                            {
                              NativeFunction *nf
                                  = static_cast<NativeFunction *> (fo_f);

                              if (!nf->get_va_args ())
                                assert (nf->get_args ().get_size ()
                                        == args_eval.get_size ());
                              else
                                assert (nf->get_args ().get_size ()
                                        > 0); /* at least one arg */

                              Module *fmod = new Module (ModuleType::Function,
                                                         Vec<Statement *> ());
                              fmod->set_parent (&mod);

                              if (nf->get_va_args ())
                                {
                                  /**
                                   * Function uses variable arguments
                                   */
                                  Str arg_a = nf->get_args ()[0];

                                  /* convert args_eval to ArrayObject */
                                  for (Object *&i : args_eval)
                                    IR (i); /* transfer back ownership */

                                  ArrayObject *ao
                                      = new ArrayObject (args_eval);

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
                                      fmod->set_variable (p, args_eval[j++]);

                                      delete[] p;
                                    }
                                }

                              Object *ret;

                              TC (ret = nf->call (fmod));
                              DR (ret);
                              delete fmod;
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
                }
              else
                assert (args_eval.get_size () == 0);

              res = co;

              /**
               * NOTE: We have already done an IR(co)
               * at the very beginning. So now if
               * we do a IR(res) then we will be increasing
               * ref_count by 2. This will lead to memory leaks
               *
               * Conclusion:
               * ! DO NOT CALL IR(res) HERE
               */
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

        Object *o_parent = expr_eval (mod, e_parent);
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
        TC (o_right = expr_eval (mod, right));

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
        TC (o_right = expr_eval (mod, right));

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
        TC (o_right = expr_eval (mod, right));

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
        TC (o_right = expr_eval (mod, right));

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
        TC (o_body = expr_eval (mod, body));

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

    default:
      std::cerr << "invalid expr type: " << (int)e->get_type () << std::endl;
      break;
    }

  if (res == nullptr)
    throw "res_expr_is_null";

  if (!res->get_ref_count ())
    {
      std::cout << "res->ref_count is 0, possible seg_fault. Did you call IR "
                   "(res)?\n";
    }

  return res;
}

bool
Module::has_variable (std::string rhs)
{
  if (!vtable.count (rhs))
    return parent == nullptr ? false : parent->has_variable (rhs);

  return true;
}
} // namespace sf

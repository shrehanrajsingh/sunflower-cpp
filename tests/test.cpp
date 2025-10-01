#include <sunflower.hpp>

#define TEST(X) test##X ()

sf::Environment *sf_env = nullptr;

/* vector test */
void
test1 ()
{
  using namespace sf;

  Vec<int> v;

  for (int i = 0; i < 100; i++)
    v.push_back (i * i);

  for (int i : v)
    std::cout << i << ", ";
  putchar ('\n');

  v.clear ();

  for (int i = 0; i < 10; i++)
    v.push_back (i);

  v.insert (2, 100);
  v.insert (2, 101);
  v.insert (3, 102);

  for (int i : v)
    std::cout << i << ", ";
  putchar ('\n');

  v = { 81, 82, 83, 84, 85 };

  for (int i : v)
    std::cout << i << ", ";
  std::cout << std::endl;

  v.reverse ();

  for (int i : v)
    std::cout << i << ", ";
  std::cout << std::endl;
}

/* string test */
void
test2 ()
{
  using namespace sf;
  Str s = "hello, world!";
  s.push_back ('h');

  std::cout << s << '\n';
}

sf::Object *
native_putln (sf::Module *m)
{
  sf::Object *a;
  TC (a = m->get_variable ("a"));

  // std::cout << "Inside native_putln!\n";
  std::cout << a->get_stdout_repr () << std::endl;

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (r);
  //   std::cout << r->get_ref_count () << '\n';
  return r;
}

sf::Object *
native_print (sf::Module *m)
{
  sf::Object *a;
  TC (a = m->get_variable ("a"));

  sf::ArrayObject *ao = static_cast<sf::ArrayObject *> (a);

  for (size_t i = 0; i < ao->get_vals ().get_size (); i++)
    {
      std::cout << ao->get_vals ()[i]->get_stdout_repr ();

      if (i != ao->get_vals ().get_size () - 1)
        std::cout << ' ';
      else
        std::cout << std::endl;
    }

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  IR (r);
  //   std::cout << r->get_ref_count () << '\n';
  return r;
}

void
test3 ()
{
  using namespace sf;

  Vec<Statement *> sts;

  /* a = 10 */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("a")),
      static_cast<Expr *> (new ConstantExpr (
          static_cast<Constant *> (new IntegerConstant (10)))))));

  //   std::cout << (int)sts[0]->get_type ();
  NativeFunction *nv = new NativeFunction (native_putln, { "a" });
  I (nv);

  /* putln = <function...> */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("putln")),
      static_cast<Expr *> (new FunctionExpr (static_cast<Function *> (nv))))));

  /* b = 20 */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("b")),
      static_cast<Expr *> (new ConstantExpr (
          static_cast<Constant *> (new IntegerConstant (20)))))));

  /* b = 30 */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("b")),
      static_cast<Expr *> (new ConstantExpr (
          static_cast<Constant *> (new IntegerConstant (30)))))));

  /* c = a */
  sts.push_back (static_cast<Statement *> (
      new VarDeclStatement (static_cast<Expr *> (new VariableExpr ("c")),
                            static_cast<Expr *> (new VariableExpr ("a")))));

  /* d = [10] */
  // sts.push_back (static_cast<Statement *> (new VarDeclStatement (
  //     static_cast<Expr *> (new VariableExpr ("d")),
  //     static_cast<Expr *> (new ArrayExpr (
  //         { static_cast<Expr *> (new ConstantExpr (
  //               static_cast<Constant *> (new IntegerConstant (10)))),
  //           static_cast<Expr *> (new ConstantExpr (
  //               static_cast<Constant *> (new FloatConstant (4.5)))) })))));

  /* putln ("Hello, World!") */
  sts.push_back (static_cast<Statement *> (new FuncCallStatement (
      static_cast<Expr *> (new VariableExpr ("putln")),
      { static_cast<Expr *> (new ConstantExpr (static_cast<Constant *> (
          new StringConstant ("Hello, World!")))) })));

  /*
        if none
          putln ("Inside if!!")
        else
          putln ("Inside else!!")
   */
  sts.push_back (static_cast<Statement *> (new IfConstruct (
      static_cast<Expr *> (
          new ConstantExpr (static_cast<Constant *> (new NoneConstant ()))),
      { static_cast<Statement *> (new FuncCallStatement (
          static_cast<Expr *> (new VariableExpr ("putln")),
          { static_cast<Expr *> (new ConstantExpr (static_cast<Constant *> (
              new StringConstant ("Inside if!!")))) })) },
      {},
      { static_cast<Statement *> (new FuncCallStatement (
          static_cast<Expr *> (new VariableExpr ("putln")),
          { static_cast<Expr *> (new ConstantExpr (static_cast<Constant *> (
              new StringConstant ("Inside else!!")))) })) })));

  //   for (size_t i = 0; i < sts.get_size (); i++)
  //     {
  //       std::cout << (int)sts[i]->get_type () << '\n';
  //     }
  // while (1)
  {
    Module *m = new Module (ModuleType::File, sts);

    mod_exec (*m);

    for (auto i : m->get_vtable ())
      {
        std::cout << i.first << '\t' << i.second->get_ref_count () << '\t';
        i.second->print ();
        std::cout << std::endl;
      }

    delete m;

    // for (auto i : vt)
    //   {
    //     // std::cout << i.first << '\t' << i.second->get_ref_count () <<
    //     '\t';
    //     // i.second->print ();
    //     std::cout << i.first << '\t' << (i.second == nullptr);
    //     std::cout << std::endl;
    //   }
  }

  for (auto i : sts)
    delete i;
}

void
test4 ()
{
  using namespace sf;

  std::ifstream fl ("../../tests/test.sf");

  sf_env->add_path ("../../tests/");

  if (!fl)
    {
      ERRMSG ("Invalid file path");
    }

  std::string s, p;
  while (std::getline (fl, p))
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

  // std::cout << s << std::endl;
  Vec<Token *> r = tokenize ((char *)s.c_str ());

  std::cout << r.get_size () << '\n';
  for (auto &&i : r)
    {
      i->print ();
      i->print_pos ();
    }

  Vec<Statement *> ast = stmt_gen (r);

  // NativeFunction *nv_putln = new NativeFunction (native_putln, { "a" });

  // /**
  //  * When testing for memory leaks
  //  * by looping mod_exec routine
  //  * infinite times, we preserve
  //  * the Function * in AST by increasing
  //  * its ref_count by 1 exactly so that
  //  * after each mod_exec the reference count
  //  * falls back to 1 and we do not free the object.
  //  * Freeing the object would result in SEG_FAULT in the
  //  * next iteration because the Function* is used in AST
  //  * (see the next statement)
  //  * and we would be storing a nullptr.
  //  * NOTE: In actual definition of I(X), I recommend
  //  * not using I(X) anywhere (use IR(X) instead), however
  //  * it is absolutely fine to use I(X) in this context since we are
  //  * using it with Function* and not Object*
  //  */
  // I (nv_putln);

  // /* putln = <function...> */
  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("putln")),
  //                    static_cast<Expr *> (new FunctionExpr (
  //                        static_cast<Function *> (nv_putln))))));

  // NativeFunction *nv_print = new NativeFunction (native_print, { "a" });
  // nv_print->set_va_args (true); /* any number of arguments */

  // I (nv_print);

  // ast.insert (0, static_cast<Statement *> (new VarDeclStatement (
  //                    static_cast<Expr *> (new VariableExpr ("write")),
  //                    static_cast<Expr *> (new FunctionExpr (
  //                        static_cast<Function *> (nv_print))))));

  std::cout << ast.get_size () << std::endl;
  for (auto &&i : ast)
    {
      i->print ();
    }

  std::cout << "--------------" << std::endl;

  native::add_natives (ast);
  try
    {
      /* do not check for leak by uncommenting this */
      /* this practice is deprecated */
      /* use while 1 loop within Sunflower */
      // while (1)
      {
        Module *m = new Module (ModuleType::File, ast, lines, sf_env);

        mod_exec (*m);

        std::cout << "--------------" << std::endl;

        for (auto i : m->get_vtable ())
          {
            std::cout << i.first << std::endl;
            i.second->print ();
            std::cout << std::endl;
          }

        delete m;
      }
    }
  catch (const char *e)
    {
      std::cerr << e << std::endl;
    }

  for (auto &&i : ast)
    delete i;

  fl.close ();
}

void
test5 ()
{
  /* only reads file and prints output, no debug information */
  using namespace sf;

  std::ifstream fl ("../../tests/test.sf");

  if (!fl)
    {
      ERRMSG ("Invalid file path");
    }

  std::string s, p;
  while (std::getline (fl, p))
    s += p + '\n';

  // std::cout << s << std::endl;
  Vec<Token *> r = tokenize ((char *)s.c_str ());

  // std::cout << r.get_size () << '\n';
  // for (auto &&i : r)
  //   {
  //     i->print ();
  //   }

  Vec<Statement *> ast = stmt_gen (r);

  native::add_natives (ast);

  // std::cout << ast.get_size () << std::endl;
  // for (auto &&i : ast)
  //   {
  //     i->print ();
  //   }

  // std::cout << "--------------" << std::endl;

  try
    {
      // while (1)
      {
        Module *m = new Module (ModuleType::File, ast);

        mod_exec (*m);

        // std::cout << "--------------" << std::endl;

        // for (auto i : m->get_vtable ())
        //   {
        //     std::cout << i.first << std::endl;
        //     i.second->print ();
        //     std::cout << std::endl;
        //   }

        delete m;
      }
    }
  catch (const char *e)
    {
      std::cerr << e << std::endl;
    }

  for (auto &&i : ast)
    delete i;

  fl.close ();
}

/* detailed */
void
test6 ()
{
  using namespace sf;

  std::ifstream ifs ("../../tests/test.sf");

  std::string s, p;
  while (std::getline (ifs, p))
    s += p + '\n';

  Vec<Token *> r = tokenize ((char *)s.c_str ());
  Vec<Statement *> ast = stmt_gen (r);

  native::add_natives (ast);

  Module *mod = new Module (ModuleType::File, ast);
  mod_exec (*mod);

  delete mod;

  ifs.close ();
}

int
main (int argc, char const *argv[])
{
  sf_env = new sf::Environment ();
  for (int i = 0; i < argc; i++)
    {
      sf_env->add_arg (argv[i]);
    }

  TEST (4);
  // std::cout << "Program ended." << std::endl;
  return 0;
}
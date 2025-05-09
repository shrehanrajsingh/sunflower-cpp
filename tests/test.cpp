#include <sunflower.hpp>

#define TEST(X) test##X ()

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

  std::cout << "Inside native_putln!\n";
  a->print ();
  std::cout << std::endl;

  sf::Object *r = static_cast<sf::Object *> (new sf::ConstantObject (
      static_cast<sf::Constant *> (new sf::NoneConstant ())));

  I (r);
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

  /* putln = <function...> */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("putln")),
      static_cast<Expr *> (new FunctionExpr (static_cast<Function *> (nv))))));

  /* b = 20 */
  sts.push_back (static_cast<Statement *> (new VarDeclStatement (
      static_cast<Expr *> (new VariableExpr ("b")),
      static_cast<Expr *> (new ConstantExpr (
          static_cast<Constant *> (new IntegerConstant (20)))))));

  /* c = a */
  sts.push_back (static_cast<Statement *> (
      new VarDeclStatement (static_cast<Expr *> (new VariableExpr ("c")),
                            static_cast<Expr *> (new VariableExpr ("a")))));

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
  while (1)
    {
      Module *m = new Module (ModuleType::File, sts);

      mod_exec (*m);

      //   for (auto i : m->get_vtable ())
      //     {
      //       std::cout << i.first << '\t' << i.second->get_ref_count () <<
      //       '\t'; i.second->print (); std::cout << std::endl;
      //     }

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

int
main (int argc, char const *argv[])
{
  TEST (3);
  std::cout << "Program ended." << std::endl;
  return 0;
}
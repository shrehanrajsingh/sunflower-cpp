#include "sf-executable.hpp"

struct cmd_line_args
{
  bool help_requested = false;
  std::vector<std::string> fpaths;
};

void
print_help ()
{
  std::cout << "Usage: sunflower [options] [file...]\n"
            << "Options:\n"
            << "  -h, --help     Display this help message\n"
            << "  -f, --file     Specify a file to process\n"
            << "\nIf no options are provided, arguments are treated as file "
               "paths.\n";
}

cmd_line_args
parse_cmdline (int argc, char *argv[])
{
  cmd_line_args args;

  for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg == "-h" || arg == "--help")
        {
          args.help_requested = true;
        }
      else if (arg == "-f" || arg == "--file")
        {
          if (i + 1 < argc)
            {
              args.fpaths.push_back (argv[++i]);
            }
          else
            {
              std::cerr << "Error: -f/--file option requires a file path.\n";
              args.help_requested = true;
            }
        }
      else
        {
          /* treat as file */
          args.fpaths.push_back (arg);
        }
    }

  return args;
}

int
main (int argc, char *argv[])
{
  cmd_line_args args = parse_cmdline (argc, argv);

  if (args.help_requested)
    {
      print_help ();
      return 0;
    }

  if (args.fpaths.empty ())
    {
      std::cerr << "Error: No input files specified.\n";
      print_help ();
      return 1;
    }

  for (const auto &fp : args.fpaths)
    {
      if (args.fpaths.size () > 1)
        std::cout << "Processing file: " << fp << std::endl;

      std::ifstream ifs (fp);

      if (!ifs)
        {
          std::cerr << "Invalid file path '" << fp << "'" << std::endl;
          exit (EXIT_FAILURE);
        }

      std::string s, p;
      while (std::getline (ifs, p))
        s += p + '\n';

      sf::Vec<sf::Str> lines;
      lines.push_back ("");

      for (char c : s)
        {
          if (c == '\n')
            lines.push_back ("");
          else
            lines.back ().push_back (c);
        }

      sf::Vec<sf::Token *> r = sf::tokenize ((char *)s.c_str ());
      sf::Vec<sf::Statement *> ast = sf::stmt_gen (r);

      sf::native::add_natives (ast);

      sf::Module *m = new sf::Module (ModuleType::File, ast, lines);

      try
        {
          sf::mod_exec (*m);

          delete m;
        }
      catch (const char *e)
        {
          std::cerr << "Error processing file '" << fp << "': " << e
                    << std::endl;
        }

      for (auto &&i : ast)
        delete i;

      ifs.close ();
    }

  return 0;
}
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

sf::Environment *sf_env = nullptr;

std::filesystem::path
get_exe_path ()
{
#if defined(__linux__)
  return std::filesystem::read_symlink ("/proc/self/exe");

#elif defined(__APPLE__)
  char buf[PATH_MAX];
  uint32_t size = sizeof (buf);

  if (_NSGetExecutablePath (buf, &size) != 0)
    throw std::runtime_error ("Executable path too long");
  return std::filesystem::canonical (buf);

#elif defined(_WIN32)
  std::vector<wchar_t> buffer (32768);
  DWORD len = GetModuleFileNameW (nullptr, buffer.data (),
                                  static_cast<DWORD> (buffer.size ()));

  if (len == 0)
    throw std::runtime_error ("GetModuleFileNameW failed");

  return std::filesystem::path (buffer.data ());

#else
  return std::filesystem::absolute (".");

#endif // __linux__
}

void
signal_catch (int signal)
{
  switch (signal)
    {
    case SIGINT:
      {
        sf::native_mod::nmod_destroy ();
        exit (1);
      }
      break;

    default:
      break;
    }
}

int
main (int argc, char *argv[])
{
  std::signal (SIGINT, signal_catch);

  cmd_line_args args = parse_cmdline (argc, argv);

  sf_env = new sf::Environment ();
  std::string can_path = get_exe_path ().string ();

  size_t last_slash = 0;
  bool saw_slash = false;
  for (size_t i = 0; i < can_path.size (); i++)
    {
      if (can_path[i] == '/' || can_path[i] == '\\')
        {
          saw_slash = true;
          last_slash = i;
        }
    }

  if (saw_slash)
    {
      can_path = can_path.substr (0, last_slash + 1);

      // sf_env->add_path (can_path.c_str ());
      sf_env->add_path ((can_path + "lib/").c_str ());
    }

  sf_env->add_path (std::filesystem::current_path ().string ().c_str ());

  // for (sf::Str &i : sf_env->get_syspaths ())
  //   std::cout << i << '\n';

  for (int i = 0; i < argc; i++)
    {
      sf_env->add_arg (argv[i]);
    }

  sf::native_mod::nmod_init ();
  sf::native_mod::Thread::init_runtime_threads ();

  if (args.help_requested)
    {
      print_help ();

      sf::native_mod::nmod_destroy ();
      return 0;
    }

  if (args.fpaths.empty ())
    {
      std::cerr << "Error: No input files specified.\n";
      print_help ();

      sf::native_mod::nmod_destroy ();
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
          sf::native_mod::nmod_destroy ();

          exit (EXIT_FAILURE);
        }

      std::filesystem::path full_path (fp);
      std::filesystem::path abc_can = std::filesystem::canonical (full_path);

      std::string abc_c_path = abc_can.string ().c_str ();
      size_t last_slash = 0;
      bool saw_slash = false;

      for (size_t i = 0; i < abc_c_path.size (); i++)
        {
          if (abc_c_path[i] == '/' || abc_c_path[i] == '\\')
            {
              saw_slash = true;
              last_slash = i;
            }
        }

      if (saw_slash)
        {
          abc_c_path = abc_c_path.substr (0, last_slash + 1);
          sf_env->add_path (abc_c_path.c_str ());

          sf::__sf_get_global_env ().add ("FILE_PATH", abc_c_path);
        }

      // for (sf::Str &i : sf_env->get_syspaths ())
      //   std::cout << i << '\n';

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

      sf::Module *m = new sf::Module (ModuleType::File, ast, lines, sf_env);

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

  sf::native_mod::nmod_destroy ();

  return 0;
}
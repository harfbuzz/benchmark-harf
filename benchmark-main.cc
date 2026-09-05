#include "benchmark-common.hh"
#include "benchmark-config.hh"

#include <benchmark/benchmark.h>
#include <glib.h>

#include <cstdlib>
#include <cstring>

#define HB_FILE(path) HB_SOURCE_ROOT "/" path

static test_input_t default_tests[] =
{
  {HB_FILE ("perf/fonts/Roboto-Regular.ttf"),
   HB_FILE ("perf/texts/en-thelittleprince.txt")},

  {HB_FILE ("perf/fonts/Roboto-Regular.ttf"),
   HB_FILE ("perf/texts/en-words.txt")},

  {HB_FILE ("test/subset/data/fonts/SourceSerifVariable-Roman.ttf"),
   HB_FILE ("perf/texts/react-dom.txt")},

  {HB_FILE ("test/subset/data/fonts/NotoSansDevanagari-Regular.ttf"),
   HB_FILE ("perf/texts/hi-words.txt")},

  {HB_FILE ("perf/fonts/Amiri-Regular.ttf"),
   HB_FILE ("perf/texts/fa-thelittleprince.txt")},

  {HB_FILE ("perf/fonts/NotoNastaliqUrdu-Regular.ttf"),
   HB_FILE ("perf/texts/fa-thelittleprince.txt")},

  {HB_FILE ("perf/fonts/NotoNastaliqUrdu-Regular.ttf"),
   HB_FILE ("perf/texts/fa-words.txt")},

  {HB_FILE ("perf/fonts/Gulzar-Regular.ttf"),
   HB_FILE ("perf/texts/fa-thelittleprince.txt")},

  {HB_FILE ("perf/fonts/Gulzar-Regular.ttf"),
   HB_FILE ("perf/texts/fa-words.txt")},

  {HB_FILE ("perf/fonts/NotoSansDuployan-Regular.otf"),
   HB_FILE ("perf/texts/duployan.txt")},
};

static char *variations_string = nullptr;
static char *direction = nullptr;

static GOptionEntry entries[] =
{
  {"variations", 0, 0, G_OPTION_ARG_STRING, &variations_string,
   "Comma-separated font variations to apply during shaping", "VARIATIONS"},
  {"direction", 0, 0, G_OPTION_ARG_STRING, &direction,
   "Direction to apply during shaping", "DIRECTION"},
  {nullptr},
};

static void
print_usage (const char *prgname)
{
  g_print ("Usage: %s [OPTIONS] [FONTFILE TEXTFILE]...\n", prgname);
}

int
main (int argc, char **argv)
{
  char *prgname = g_path_get_basename (argv[0]);

  GOptionContext *context = g_option_context_new ("");
  g_option_context_set_summary (
    context, "Benchmark HarfBuzz and HarfRust text shaping");
  g_option_context_set_description (context, "Benchmark Options:");
  g_option_context_add_main_entries (context, entries, nullptr);

  bool show_help = false;
  for (int i = 1; i < argc; i++)
  {
    if (std::strcmp (argv[i], "--help") == 0 ||
	std::strcmp (argv[i], "-h") == 0)
    {
      argv[i] = const_cast<char *> ("--help");
      show_help = true;
      break;
    }
  }
  if (show_help)
  {
    print_usage (prgname);

    char *help_text = g_option_context_get_help (context, false, nullptr);
    char *options = std::strstr (help_text, "\n\n");
    g_print ("%s", options ? options : help_text);
    g_free (help_text);

    benchmark::Initialize (&argc, argv);
  }

  benchmark::Initialize (&argc, argv);

  GError *error = nullptr;
  if (!g_option_context_parse (context, &argc, &argv, &error))
  {
    g_printerr ("%s\n", error->message);
    g_error_free (error);
    g_option_context_free (context);
    g_free (prgname);
    return 1;
  }
  g_option_context_free (context);

  argc--;
  argv++;
  if (argc % 2)
  {
    g_printerr ("Font and text files must be provided in pairs.\n\n");
    print_usage (prgname);
    g_free (prgname);
    return 1;
  }

  test_input_t *tests = default_tests;
  unsigned num_tests = sizeof (default_tests) / sizeof (default_tests[0]);
  unsigned custom_num_tests = argc / 2;
  if (custom_num_tests)
  {
    tests = static_cast<test_input_t *> (
      std::calloc (custom_num_tests, sizeof (*tests)));
    if (!tests)
    {
      g_printerr ("Failed to allocate test inputs.\n");
      g_free (prgname);
      return 1;
    }

    for (unsigned i = 0; i < custom_num_tests; i++)
    {
      tests[i].font_path = argv[2 * i];
      tests[i].text_path = argv[2 * i + 1];
    }
    num_tests = custom_num_tests;
  }

  benchmark_options_t options = {variations_string, direction};
  for (unsigned i = 0; i < num_tests; i++)
  {
    register_harfbuzz_benchmarks (tests[i], options);
    register_harfrust_benchmarks (tests[i], options);
  }

  benchmark::RunSpecifiedBenchmarks ();
  benchmark::Shutdown ();

  if (tests != default_tests)
    std::free (tests);
  g_free (variations_string);
  g_free (direction);
  g_free (prgname);
  return 0;
}

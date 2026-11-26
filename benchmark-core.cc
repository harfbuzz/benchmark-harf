#include "shaping-workload.hh"
#include <benchmark/benchmark.h>

#if defined(BENCHMARK_HARFBUZZ)
#define REGISTER_BENCHMARKS register_harfbuzz_benchmarks
#else
#define REGISTER_BENCHMARKS register_harfrust_benchmarks
#endif

static const char *
path_basename (const char *path)
{
  const char *p = std::strrchr (path, '/');
  return p ? p + 1 : path;
}

static void
BM_Shape (benchmark::State &state,
          const char *shaper,
          const test_input_t &input,
          const benchmark_options_t &options)
{
  shaping_workload_t workload;
  if (auto error = workload.prepare (input, options, shaper))
  {
    state.SkipWithError (error);
    return;
  }

  for (auto _ : state)
    if (!workload.run ())
      std::abort ();
}

static void
register_benchmark (const test_input_t &input,
		    const benchmark_options_t &options,
		    const char *shaper,
		    const char *engine_name)
{
  std::string name = "BM_Shape/";
  name += path_basename (input.font_path);
  name += "/";
  name += path_basename (input.text_path);
  name += "/";
  name += engine_name;

  benchmark::RegisterBenchmark (name.c_str (), BM_Shape,
				shaper, input, options)
    ->Unit (benchmark::kMillisecond);
}

void
REGISTER_BENCHMARKS (const test_input_t &input,
		     const benchmark_options_t &options)
{
#if defined(BENCHMARK_HARFBUZZ)
  register_benchmark (input, options, "ot", "hb");

  const char **shapers = hb_shape_list_shapers ();
  for (const char **shaper = shapers; *shaper; shaper++)
    if (std::strcmp (*shaper, "harfrust") == 0)
    {
      register_benchmark (input, options, "harfrust", "hb-hr");
      break;
    }
#else
  register_benchmark (input, options, "ot", "hr");
#endif
}

#pragma once

struct test_input_t
{
  const char *font_path;
  const char *text_path;
};

struct benchmark_options_t
{
  const char *variations;
  const char *direction;
};

void register_harfbuzz_benchmarks (const test_input_t &input,
				    const benchmark_options_t &options);
void register_harfrust_benchmarks (const test_input_t &input,
				   const benchmark_options_t &options);

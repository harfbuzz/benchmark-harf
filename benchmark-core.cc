#if defined(BENCHMARK_HARFBUZZ)
#include <hb.h>
#define REGISTER_BENCHMARKS register_harfbuzz_benchmarks
#elif defined(BENCHMARK_HARFRUST)
#include <hr-hb.h>
#define REGISTER_BENCHMARKS register_harfrust_benchmarks
#else
#error "A benchmark engine must be selected"
#endif

#include "benchmark-common.hh"

#include <benchmark/benchmark.h>

#include <cstdlib>
#include <cstring>
#include <string>

static const char *
path_basename (const char *path)
{
  const char *p = std::strrchr (path, '/');
  return p ? p + 1 : path;
}

static bool
shape (hb_buffer_t *buf,
       hb_font_t *font,
       const char *text,
       unsigned text_length,
       const char *shaper,
       const char *direction)
{
  const char *end;
  while ((end = static_cast<const char *> (std::memchr (text, '\n', text_length))))
  {
    hb_buffer_clear_contents (buf);
    hb_buffer_add_utf8 (buf, text, text_length, 0, end - text);
    hb_buffer_guess_segment_properties (buf);
    if (direction)
      hb_buffer_set_direction (buf, hb_direction_from_string (direction, -1));
    const char *shaper_list[] = {shaper, nullptr};
    if (!hb_shape_full (font, buf, nullptr, 0, shaper_list))
      return false;

    unsigned skip = end - text + 1;
    text_length -= skip;
    text += skip;
  }
  return true;
}

static hb_variation_t *
parse_variations (const char *variation_string, unsigned *num_variations)
{
  *num_variations = 0;
  if (!variation_string || !*variation_string)
    return nullptr;

  unsigned allocation_length = 1;
  for (const char *p = variation_string; *p; p++)
    if (*p == ',')
      allocation_length++;

  hb_variation_t *variations = static_cast<hb_variation_t *> (
    std::calloc (allocation_length, sizeof (*variations)));
  if (!variations)
    return nullptr;

  const char *p = variation_string;
  while (p)
  {
    const char *end = std::strchr (p, ',');
    if (hb_variation_from_string (p, end ? end - p : -1,
				  &variations[*num_variations]))
      (*num_variations)++;
    p = end ? end + 1 : nullptr;
  }

  return variations;
}

static void
BM_Shape (benchmark::State &state,
	  const char *shaper,
	  const test_input_t &input,
	  const benchmark_options_t &options)
{
  hb_blob_t *font_blob = hb_blob_create_from_file_or_fail (input.font_path);
  if (!font_blob)
  {
    state.SkipWithError ("Failed to open font file.");
    return;
  }

  hb_face_t *face = hb_face_create (font_blob, 0);
  hb_blob_destroy (font_blob);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);

  unsigned num_variations;
  hb_variation_t *variations = parse_variations (options.variations,
						 &num_variations);
  if (options.variations && *options.variations && !variations)
  {
    state.SkipWithError ("Failed to allocate font variations.");
    hb_font_destroy (font);
    return;
  }
  if (variations)
    hb_font_set_variations (font, variations, num_variations);

  hb_blob_t *text_blob = hb_blob_create_from_file_or_fail (input.text_path);
  if (!text_blob)
  {
    state.SkipWithError ("Failed to open text file.");
    std::free (variations);
    hb_font_destroy (font);
    return;
  }

  unsigned text_length;
  const char *text = hb_blob_get_data (text_blob, &text_length);
  hb_buffer_t *buf = hb_buffer_create ();

  if (!shape (buf, font, text, text_length, shaper, options.direction))
  {
    state.SkipWithError ("Shaping failed.");
    goto done;
  }

  for (auto _ : state)
  {
    if (!shape (buf, font, text, text_length, shaper, options.direction))
      std::abort ();
  }

done:
  hb_buffer_destroy (buf);
  hb_blob_destroy (text_blob);
  std::free (variations);
  hb_font_destroy (font);
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

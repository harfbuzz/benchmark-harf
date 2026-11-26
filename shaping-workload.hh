#pragma once

#if defined(BENCHMARK_HARFBUZZ)
#include <hb.h>
#elif defined(BENCHMARK_HARFRUST)
#include <hr-hb.h>
#else
#error "A benchmark engine must be selected"
#endif

#include "benchmark-common.hh"

#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>

// Each translation unit uses the functions from its selected C API.
namespace {

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

// Own one font, buffer and corpus for all passes through a timed workload.
class shaping_workload_t
{
public:
  shaping_workload_t () = default;
  shaping_workload_t (const shaping_workload_t &) = delete;
  shaping_workload_t &operator= (const shaping_workload_t &) = delete;

  ~shaping_workload_t ()
  {
    if (buf) hb_buffer_destroy (buf);
    if (text_blob) hb_blob_destroy (text_blob);
    if (font) hb_font_destroy (font);
  }

  const char *prepare (const test_input_t &input, const benchmark_options_t &options,
                       const char *shaper_name)
  {
    shaper = shaper_name;
    if (options.direction) direction = options.direction;

    hb_blob_t *font_blob = hb_blob_create_from_file_or_fail (input.font_path);
    if (!font_blob) return "Failed to open font file.";
    hb_face_t *face = hb_face_create (font_blob, 0);
    hb_blob_destroy (font_blob);
    font = hb_font_create (face);
    hb_face_destroy (face);

    unsigned num_variations;
    hb_variation_t *variations = parse_variations (options.variations, &num_variations);
    if (options.variations && *options.variations && !variations)
      return "Failed to allocate font variations.";
    if (variations)
      hb_font_set_variations (font, variations, num_variations);
    std::free (variations);

    text_blob = hb_blob_create_from_file_or_fail (input.text_path);
    if (!text_blob) return "Failed to open text file.";
    text = hb_blob_get_data (text_blob, &text_length);
    buf = hb_buffer_create ();
    return run () ? nullptr : "Shaping failed.";
  }

  bool run ()
  {
    return shape (buf, font, text, text_length, shaper,
                  direction ? direction->c_str () : nullptr);
  }

private:
  hb_buffer_t *buf = nullptr;
  hb_font_t *font = nullptr;
  hb_blob_t *text_blob = nullptr;
  const char *text = nullptr;
  unsigned text_length = 0;
  const char *shaper = nullptr;
  std::optional<std::string> direction;
};

} // namespace

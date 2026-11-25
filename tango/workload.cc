#include "../shaping-workload.hh"
#include <cstdio>
#include <memory>

extern "C" void *
harf_prepare (const char *font, const char *text,
              const char *variations, const char *direction)
{
  auto workload = std::make_unique<shaping_workload_t> ();
  if (auto error = workload->prepare ({font, text}, {variations, direction}, "ot"))
  {
    std::fprintf (stderr, "%s (%s, %s)\n", error, font, text);
    return nullptr;
  }
  return workload.release ();
}

extern "C" bool harf_run (void *workload) { return static_cast<shaping_workload_t *> (workload)->run (); }
extern "C" void harf_free (void *workload) { delete static_cast<shaping_workload_t *> (workload); }

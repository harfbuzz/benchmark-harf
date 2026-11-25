# benchmark-harf

This builds one `benchmark-harf` executable containing the shaping benchmark
twice: once against HarfBuzz's `<hb.h>` API and once against HarfRust's
`<hr-hb.h>` compatibility API. The HarfBuzz compilation registers its native
OpenType/AAT shaper and, when available, its built-in HarfRust shaper. The output
suffixes are:

- `hb`: HarfBuzz's native OpenType/AAT shaper
- `hr`: HarfRust through its C API
- `hb-hr`: HarfRust through HarfBuzz's shaper integration

The default paths expect release builds in `~/hb/build` and
`~/hr/target/release`. Configure and build with:

```sh
meson setup build
meson compile -C build
```

Run the default workload corpus with:

```sh
./build/benchmark-harf
```

The default workloads also include these optional local AAT collections when
they are present in the configured HarfBuzz source directory:

| Font collection | Text from `hb_root/perf/texts/` |
| --- | --- |
| `LucidaGrande.ttc` | `en-thelittleprince.txt` |
| `Menlo.ttc` | `en-thelittleprince.txt` |
| `GeezaPro.ttc` | `fa-thelittleprince.txt` |
| `Devanagari Sangam MN.ttc` | `hi-words.txt` |

These fonts are not included or copied into this project. Both engines support
AAT. Each collection uses face 0, like other font inputs. To use copies in a
different directory:

```sh
./build/benchmark-harf --aat-font-dir "$HOME/hb"
```

As with HarfBuzz's `perf/benchmark-shape`, custom inputs are font/text pairs:

```sh
./build/benchmark-harf font.ttf text.txt \
  --benchmark_min_time=.1s --benchmark_repetitions=5
```

Custom font/text pairs replace all default workloads, including the optional
AAT collections. Missing optional AAT fonts are skipped.

The HarfBuzz, HarfBuzz build, HarfRust, and HarfRust build paths can be
overridden at setup time:

```sh
meson setup build \
  -Dhb_root=/path/to/harfbuzz \
  -Dhb_build=/path/to/harfbuzz/build \
  -Dhr_root=/path/to/harfrust \
  -Dhr_build=/path/to/harfrust/target/release
```

## Paired comparisons with Tango

Tango compares two warmed shaping workloads on the same runner. Each worker
has its own executable, engine library, font and buffer. Both use the same
C++ shaping loop as the Google Benchmark executable. A sample measures one
pass through the newline-delimited corpus. Font loading and warm-up happen
before timing.

On Linux, build a worker for each source revision you want to compare. Use
a separate output directory per revision so each executable loads its own
library. For example, to compare two HarfBuzz revisions:

```sh
tango/build-engine.sh hb /path/to/harfbuzz-base /tmp/hb-base
tango/build-engine.sh hb /path/to/harfbuzz-pr /tmp/hb-pr

HARF_FONT=/path/to/font.ttf HARF_TEXT=/path/to/text.txt \
  /tmp/hb-pr/tango/release/benchmark-harf-tango compare \
  /tmp/hb-base/tango/release/benchmark-harf-tango --time 5
```

Use `hr` for two HarfRust revisions, or compare an `hr` candidate with an
`hb` baseline. Both workers inherit the same `HARF_FONT`, `HARF_TEXT` and
optional `HARF_DIRECTION` and `HARF_VARIATIONS`. End each line with a newline.

The GitHub action takes `engine` (`hb` or `hr`), `baseline`, and `candidate`
(full commit SHAs). Use the PR base SHA and its test merge SHA to measure the
change on top of the current base. Equal SHAs run an A/A control using the same
built executable in both workers. The action uses pinned font and text data
for both workers, runs three five-second comparisons per workload, writes
baseline and candidate times to the job summary, and uploads raw paired samples.
Measured slowdowns are reported. Build, shaping and measurement failures fail
the job.

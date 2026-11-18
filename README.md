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

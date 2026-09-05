# benchmark-harf

This builds one `benchmark-harf` executable containing the shaping benchmark
twice: once against HarfBuzz's `<hb.h>` API and once against HarfRust's
`<hr-hb.h>` compatibility API. The HarfBuzz compilation registers its native
OpenType shaper and, when available, its built-in HarfRust shaper. The output
suffixes are:

- `hb`: HarfBuzz's native OpenType shaper
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

As with HarfBuzz's `perf/benchmark-shape`, custom inputs are font/text pairs:

```sh
./build/benchmark-harf font.ttf text.txt \
  --benchmark_min_time=.1s --benchmark_repetitions=5
```

The HarfBuzz, HarfBuzz build, HarfRust, and HarfRust build paths can be
overridden at setup time:

```sh
meson setup build \
  -Dhb_root=/path/to/harfbuzz \
  -Dhb_build=/path/to/harfbuzz/build \
  -Dhr_root=/path/to/harfrust \
  -Dhr_build=/path/to/harfrust/target/release
```

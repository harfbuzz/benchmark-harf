#!/usr/bin/env bash
# Build one engine and a Tango executable linked to that exact build.
set -euo pipefail
engine=$1
src=$(realpath "$2")
out=$(realpath -m "$3")
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$out"

case "$engine" in
  hb)
    meson setup "$out/hb" "$src" --buildtype=release -Dauto_features=disabled
    meson compile -C "$out/hb" harfbuzz:shared_library
    export HARF_INCLUDE="$src/src" HARF_LIB="$out/hb/src"
    ;;
  hr)
    cargo build --release --manifest-path "$src/Cargo.toml" -p harfrust_capi --target-dir "$out/hr"
    export HARF_INCLUDE="$src/harfrust_capi/include" HARF_LIB="$out/hr/release"
    ;;
  *) echo "Expected hb or hr, got $engine" >&2; exit 2 ;;
esac

HARF_ENGINE="$engine" cargo build --release --locked --manifest-path "$here/Cargo.toml" --target-dir "$out/tango"

use std::{env, fs};

fn setting(name: &str) -> String {
    println!("cargo:rerun-if-env-changed={name}");
    env::var(name).unwrap_or_else(|_| panic!("set {name} (see README.md)"))
}

fn main() {
    let (define, library) = match setting("HARF_ENGINE").as_str() {
        "hb" => ("BENCHMARK_HARFBUZZ", "harfbuzz"),
        "hr" => ("BENCHMARK_HARFRUST", "harfrust_c"),
        _ => panic!("HARF_ENGINE must be hb or hr"),
    };
    let include = fs::canonicalize(setting("HARF_INCLUDE")).expect("engine headers");
    let lib = fs::canonicalize(setting("HARF_LIB")).expect("engine library directory");
    println!("cargo:rerun-if-changed=workload.cc");
    println!("cargo:rerun-if-changed=../benchmark-common.hh");
    println!("cargo:rerun-if-changed=../shaping-workload.hh");
    cc::Build::new()
        .cpp(true)
        .std("c++17")
        .define(define, None)
        .include(include)
        .file("workload.cc")
        .compile("harf_workload");
    println!("cargo:rustc-link-search=native={}", lib.display());
    println!("cargo:rustc-link-lib=dylib={library}");
    if env::var("CARGO_CFG_TARGET_FAMILY").as_deref() == Ok("unix") {
        println!("cargo:rustc-link-arg=-Wl,-rpath,{}", lib.display());
    }
}

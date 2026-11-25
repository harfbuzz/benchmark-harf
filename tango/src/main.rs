use std::env;
use std::ffi::{c_char, c_void, CString};
use std::path::Path;
use std::ptr::NonNull;
use tango_bench::{benchmark_fn, tango_benchmarks, IntoBenchmarks};

extern "C" {
    fn harf_prepare(
        font: *const c_char,
        text: *const c_char,
        variations: *const c_char,
        direction: *const c_char,
    ) -> *mut c_void;
    fn harf_run(workload: *mut c_void) -> bool;
    fn harf_free(workload: *mut c_void);
}

struct Workload(NonNull<c_void>);

impl Drop for Workload {
    fn drop(&mut self) {
        // The constructor transfers ownership of this C++ workload to us.
        unsafe { harf_free(self.0.as_ptr()) }
    }
}

impl Workload {
    fn run(&mut self) {
        // Each worker owns its buffer and its warmed font for the whole trial.
        assert!(unsafe { harf_run(self.0.as_ptr()) }, "shaping failed");
    }
}

fn benchmarks() -> impl IntoBenchmarks {
    let font = env::var("HARF_FONT").expect("set HARF_FONT to a font file");
    let text = env::var("HARF_TEXT").expect("set HARF_TEXT to a newline-terminated text corpus");
    let name = format!(
        "{}/{}",
        Path::new(&font).file_name().unwrap().to_string_lossy(),
        Path::new(&text).file_name().unwrap().to_string_lossy()
    );
    let font = CString::new(font).unwrap();
    let text = CString::new(text).unwrap();
    let variations = CString::new(env::var("HARF_VARIATIONS").unwrap_or_default()).unwrap();
    let direction = CString::new(env::var("HARF_DIRECTION").unwrap_or_default()).unwrap();
    [benchmark_fn(name, move |b| {
        // prepare opens the files and completes one warm-up pass before Tango
        // starts measuring. It retains no pointers into these strings.
        let raw = unsafe {
            harf_prepare(
                font.as_ptr(),
                text.as_ptr(),
                variations.as_ptr(),
                direction.as_ptr(),
            )
        };
        let mut workload = Workload(NonNull::new(raw).expect("could not prepare workload"));
        b.iter(move || workload.run())
    })]
}
tango_benchmarks!(benchmarks());

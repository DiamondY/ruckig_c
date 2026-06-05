use std::env;
use std::path::PathBuf;

fn main() {
    let lib_dir = env::var_os("RUCKIG_C_LIB_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            let manifest_dir =
                PathBuf::from(env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR"));
            manifest_dir
                .join("..")
                .join("..")
                .join("out")
                .join("build")
                .join("windows-clang-ninja")
        });

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=ruckig_c");
    if env::var("CARGO_CFG_TARGET_FAMILY").as_deref() == Ok("unix") {
        println!("cargo:rustc-link-lib=m");
    }
}


use std::env;
use std::path::PathBuf;

use cmake::Config;

fn main() {
    // Detect relevant file changes
    println!("cargo:rerun-if-changed=src/");
    println!("cargo:rerun-if-changed=inc/");
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed=CMakeLists.txt");

    // Find spec dir, set by embedded-wasm-spec build.rs
    let spec_dir = std::env::var("DEP_EMBEDDED_WASM_SPEC_ROOT").unwrap();

    // Export root dir for other crates that might need headers from here
    let base_dir = std::env::var("CARGO_MANIFEST_DIR").unwrap();
    println!("cargo:ROOT={}", base_dir);

    // Setup binding generation
    let mut builder = bindgen::Builder::default()
        .use_core()
        .clang_arg(format!("-I{}/inc", spec_dir))
        .ctypes_prefix("::cty")
        .header("inc/wasm_embedded/wasm3/core.h")
        .header("inc/wasm_embedded/wasm3/i2c.h")
        .header("inc/wasm_embedded/wasm3/spi.h")
        .header("inc/wasm_embedded/wasm3/gpio.h")
        .blocklist_type("gpio_drv_t")
        .blocklist_type("spi_drv_t")
        .blocklist_type("i2c_drv_t")
        .allowlist_type("wasme.*")
        .allowlist_function("WASME.*");

    // Patches to help bindgen with cross compiling
    // See: https://github.com/rust-lang/rust-bindgen/issues/1229#issuecomment-366522257
    builder = match std::env::var("TARGET").as_deref() {
        Ok("armv7-unknown-linux-gnueabihf") => {
            println!("cargo:rustc-env=CC=arm-linux-gnueabihf-gcc");
            builder
                .clang_arg("-target")
                .clang_arg("arm-linux-gnueabihf")
                .clang_arg("-I/usr/arm-linux-gnueabihf/include/")
        },
        Ok("aarch64-unknown-linux-gnu") => {
            println!("cargo:rustc-env=CC=aarch64-linux-gnu-gcc");
            builder
                .clang_arg("-target")
                .clang_arg("aarch64-linux-gnu")
                .clang_arg("-I/usr/aarch64-linux-gnu/include/")
        },
        Ok("thumbv7em-none-eabihf") => {
            println!("cargo:rustc-env=CC=arm-none-eabi-gcc");
            builder
                .use_core()
                .clang_arg("-target")
                .clang_arg("arm-none-eabihf")
                // TODO: this seems... fragile
                //.clang_arg("-I/usr/lib/gcc/arm-none-eabi/8.3.1/include/")
        },
        _ => builder,
    };

    let bindings = builder.generate()
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    // Build library
    // CC has to be set here because CMAKE_C_COMPILER is ignored by ExternalProject_Add
    // and setting CC at the top level causes build.rs to collapse...
    let mut builder = Config::new("./");
    
    #[cfg(feature = "build-wasm3")]
    builder.define("WASME_BUILD_WASM3", "ON");
    #[cfg(not(feature = "build-wasm3"))]
    builder.define("WASME_BUILD_WASM3", "OFF");

    builder.define("WASME_SPEC_DIR", spec_dir);
    
    match std::env::var("TARGET").as_deref() {
        Ok("armv7-unknown-linux-gnueabihf") => {
            builder.env("CC", "arm-linux-gnueabihf-gcc");
        },
        Ok("aarch64-unknown-linux-gnu") => {
            builder.env("CC", "aarch64-linux-gnu-gcc");
        },
        Ok("thumbv7em-none-eabihf") => {
            builder.env("CC", "arm-none-eabi-gcc");
            builder.env("CFLAGS", "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 --specs=nosys.specs -lnosys -lm -lc -lgcc");
            builder.env("LDFLAGS", "-lnosys -lm -lc -lgcc");

            builder.define("WASME_USE_WASI", "OFF");
            
            builder.define("BUILD_NATIVE", "OFF");
            builder.build_arg("VERBOSE=1");
            //builder.very_verbose(true);
        },
        #[cfg(nope)]
        Ok("thumbv7em-none-eabihf") => {
            // Invoke cmake
            let o = std::process::Command::new("cmake")
                .arg(format!("-B{}", out_path.to_string_lossy()))
                .arg(env::var("CARGO_MANIFEST_DIR").unwrap())
                .output().expect("Failed to invoke cmake");

            return;
        }
        _ => (),
    };

    let dst = builder.build();

    println!("cargo:rustc-link-search=native={}", dst.display());
    println!("cargo:rustc-link-lib=static=wasme");
    println!("cargo:rustc-link-lib=static=m3");
    //println!("cargo:rustc-link-lib=static=m");
}

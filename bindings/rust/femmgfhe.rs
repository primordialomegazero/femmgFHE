// ═══════════════════════════════════════════════════════════════
// femmgFHE — Rust Bindings (FFI)
// ═══════════════════════════════════════════════════════════════

use std::ffi::{CStr, CString};
use std::os::raw::c_char;

#[link(name = "femmgfhe_c", kind = "dylib")]
extern "C" {
    fn femmgfhe_version() -> *const c_char;
    fn femmgfhe_phi() -> f64;
    fn femmgfhe_psi() -> f64;
    fn femmgfhe_pi() -> f64;
    fn femmgfhe_hw_max_ring_dim() -> i32;
    fn femmgfhe_hw_cpu_cores() -> i32;
    fn femmgfhe_hw_cpu_brand() -> *const c_char;
    
    fn femmgfhe_create() -> *mut std::ffi::c_void;
    fn femmgfhe_destroy(handle: *mut std::ffi::c_void);
    fn femmgfhe_set_dev_mode(handle: *mut std::ffi::c_void);
    fn femmgfhe_set_test_mode(handle: *mut std::ffi::c_void);
    fn femmgfhe_set_prod_mode(handle: *mut std::ffi::c_void);
    fn femmgfhe_set_enterprise_mode(handle: *mut std::ffi::c_void);
    fn femmgfhe_get_ring_dim(handle: *mut std::ffi::c_void) -> i32;
    fn femmgfhe_get_variants(handle: *mut std::ffi::c_void) -> i32;
    fn femmgfhe_get_fractal_layers(handle: *mut std::ffi::c_void) -> i32;
}

// Safe Rust wrapper
pub struct FemmgFHE {
    handle: *mut std::ffi::c_void,
}

impl FemmgFHE {
    pub fn new() -> Self {
        unsafe {
            FemmgFHE {
                handle: femmgfhe_create(),
            }
        }
    }
    
    pub fn version() -> String {
        unsafe {
            CStr::from_ptr(femmgfhe_version())
                .to_string_lossy()
                .into_owned()
        }
    }
    
    pub fn phi() -> f64 { unsafe { femmgfhe_phi() } }
    pub fn psi() -> f64 { unsafe { femmgfhe_psi() } }
    pub fn pi() -> f64  { unsafe { femmgfhe_pi() } }
    
    pub fn max_ring_dim() -> i32 { unsafe { femmgfhe_hw_max_ring_dim() } }
    pub fn cpu_cores() -> i32   { unsafe { femmgfhe_hw_cpu_cores() } }
    pub fn cpu_brand() -> String {
        unsafe {
            CStr::from_ptr(femmgfhe_hw_cpu_brand())
                .to_string_lossy()
                .into_owned()
        }
    }
    
    pub fn set_dev_mode(&self)        { unsafe { femmgfhe_set_dev_mode(self.handle) } }
    pub fn set_test_mode(&self)       { unsafe { femmgfhe_set_test_mode(self.handle) } }
    pub fn set_prod_mode(&self)       { unsafe { femmgfhe_set_prod_mode(self.handle) } }
    pub fn set_enterprise_mode(&self) { unsafe { femmgfhe_set_enterprise_mode(self.handle) } }
    
    pub fn ring_dim(&self) -> i32       { unsafe { femmgfhe_get_ring_dim(self.handle) } }
    pub fn variants(&self) -> i32      { unsafe { femmgfhe_get_variants(self.handle) } }
    pub fn fractal_layers(&self) -> i32 { unsafe { femmgfhe_get_fractal_layers(self.handle) } }
}

impl Drop for FemmgFHE {
    fn drop(&mut self) {
        unsafe { femmgfhe_destroy(self.handle) }
    }
}

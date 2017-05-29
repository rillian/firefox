#[macro_use]
extern crate lazy_static;

lazy_static! {
    static ref MSG: std::ffi::CString =
        std::ffi::CString::new("Hello from Rust!").unwrap();
}

#[no_mangle]
pub extern "C" fn rustifoo_msg() -> *const std::os::raw::c_char {
    MSG.as_ptr()
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        use std::ffi::CStr;
        let r = unsafe {
            CStr::from_ptr(::rustifoo_msg()).to_str().unwrap()
        };
        assert_eq!(r, "Hello from Rust!");
    }
}

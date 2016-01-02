
#[no_mangle]
pub unsafe fn memcpy(dest: *mut u8, source: *mut u8, len: usize) -> *mut u8
{
	for x in 0..len {
		*dest.offset(x as isize) = *source.offset(x as isize);
	}
	dest
}

#[no_mangle]
pub unsafe fn memset(dest: *mut u8, value: i32, len: usize) -> *mut u8
{
	for x in 0..len {
		*dest.offset(x as isize) = value as u8;
	};
	dest
}
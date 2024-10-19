use std::ptr::null_mut;

use neon::prelude::*;
use windows::Win32::UI::WindowsAndMessaging;

pub fn reset(mut cx: FunctionContext) -> JsResult<JsUndefined> {
    unsafe {
        WindowsAndMessaging::SystemParametersInfoA(
            WindowsAndMessaging::SPI_SETDESKWALLPAPER,
            0,
            Some(null_mut() as *mut _),
            WindowsAndMessaging::SPIF_SENDCHANGE,
        )
        .unwrap();
    }
    Ok(cx.undefined())
}

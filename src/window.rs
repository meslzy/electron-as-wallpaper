use neon::prelude::*;

use windows::Win32::{
    Foundation::{COLORREF, HWND},
    UI::WindowsAndMessaging,
};

pub fn get_window_handle(cx: &mut FunctionContext) -> NeonResult<HWND> {
    let ptr: Handle<JsNumber> = cx.argument::<JsNumber>(0)?;
    let hwnd = ptr.value(cx) as isize;
    Ok(HWND(hwnd as _))
}

pub fn toggle_window_transparent(hwnd: HWND, transparent: bool) {
    if transparent {
        unsafe {
            let styles =
                WindowsAndMessaging::GetWindowLongA(hwnd, WindowsAndMessaging::GWL_EXSTYLE);
            WindowsAndMessaging::SetWindowLongA(
                hwnd,
                WindowsAndMessaging::GWL_EXSTYLE,
                styles | WindowsAndMessaging::WS_EX_LAYERED.0 as i32,
            );
            WindowsAndMessaging::SetLayeredWindowAttributes(
                hwnd,
                COLORREF::default(),
                255,
                WindowsAndMessaging::LWA_ALPHA,
            )
            .unwrap();
        }
    } else {
        unsafe {
            let styles =
                WindowsAndMessaging::GetWindowLongA(hwnd, WindowsAndMessaging::GWL_EXSTYLE);
            WindowsAndMessaging::SetWindowLongA(
                hwnd,
                WindowsAndMessaging::GWL_EXSTYLE,
                styles & !WindowsAndMessaging::WS_EX_LAYERED.0 as i32,
            );
        }
    }
}

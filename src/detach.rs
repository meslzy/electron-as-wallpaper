use neon::prelude::*;

use windows::Win32::UI::WindowsAndMessaging;

use crate::input::stop_input_forwarding;
use crate::window::{get_window_handle, toggle_window_transparent};

#[derive(Default)]
struct DetachOptions {
    transparent: bool,
    forward_mouse_input: bool,
    forward_keyboard_input: bool,
}

impl DetachOptions {
    fn from_cx(cx: &mut FunctionContext) -> NeonResult<Self> {
        let mut options = Self::default();

        let obj = cx.argument::<JsObject>(1)?;

        options.transparent = obj.get::<JsBoolean, _, _>(cx, "transparent")?.value(cx);
        options.forward_mouse_input = obj
            .get::<JsBoolean, _, _>(cx, "forwardMouseInput")?
            .value(cx);
        options.forward_keyboard_input = obj
            .get::<JsBoolean, _, _>(cx, "forwardKeyboardInput")?
            .value(cx);

        Ok(options)
    }
}

pub fn detach(mut cx: FunctionContext) -> JsResult<JsUndefined> {
    let hwnd = get_window_handle(&mut cx)?;
    let options = DetachOptions::from_cx(&mut cx)?;

    unsafe {
        WindowsAndMessaging::SetParent(hwnd, None).unwrap();

        if options.transparent {
            toggle_window_transparent(hwnd, false);
        }
    };

    stop_input_forwarding(hwnd);

    Ok(cx.undefined())
}

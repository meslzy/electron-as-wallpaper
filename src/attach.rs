use neon::prelude::*;

use windows::core::s;
use windows::Win32::{
    Foundation::{BOOL, HWND, LPARAM, WPARAM},
    UI::WindowsAndMessaging,
};

use crate::input::start_input_forwarding;
use crate::window::{get_window_handle, toggle_window_transparent};

extern "system" fn enum_window(window: HWND, ref_worker_w: LPARAM) -> BOOL {
    unsafe {
        let shell_dll_def_view = WindowsAndMessaging::FindWindowExA(
            window,
            HWND::default(),
            s!("SHELLDLL_DefView"),
            None,
        )
        .unwrap_or(HWND::default());

        if !HWND::is_invalid(&shell_dll_def_view) {
            let worker_w =
                WindowsAndMessaging::FindWindowExA(HWND::default(), window, s!("WorkerW"), None)
                    .unwrap_or(HWND::default());

            if !HWND::is_invalid(&worker_w) {
                *(ref_worker_w.0 as *mut HWND) = worker_w;
            }
        }

        BOOL(1)
    }
}

#[derive(Default)]
struct AttachOptions {
    transparent: bool,
    forward_mouse_input: bool,
    forward_keyboard_input: bool,
}

impl AttachOptions {
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

pub fn attach(mut cx: FunctionContext) -> JsResult<JsUndefined> {
    let hwnd = get_window_handle(&mut cx)?;
    let options = AttachOptions::from_cx(&mut cx)?;

    unsafe {
        let progman_hwnd = WindowsAndMessaging::FindWindowA(s!("Progman"), None).unwrap();

        WindowsAndMessaging::SendMessageTimeoutA(
            progman_hwnd,
            0x052C,
            WPARAM(0xD),
            LPARAM(0x1),
            WindowsAndMessaging::SMTO_NORMAL,
            1000,
            None,
        );

        let mut worker_w: HWND = HWND::default();

        WindowsAndMessaging::EnumWindows(
            Some(enum_window),
            LPARAM(&mut worker_w as *mut HWND as isize),
        )
        .unwrap();

        if HWND::is_invalid(&worker_w) {
            worker_w = WindowsAndMessaging::FindWindowExA(
                progman_hwnd,
                HWND::default(),
                s!("WorkerW"),
                None,
            )
            .unwrap();
        }

        WindowsAndMessaging::SetParent(hwnd, worker_w).unwrap();

        if options.transparent {
            toggle_window_transparent(hwnd, true);
        }

        start_input_forwarding(
            hwnd,
            options.forward_mouse_input,
            options.forward_keyboard_input,
        );

        Ok(cx.undefined())
    }
}

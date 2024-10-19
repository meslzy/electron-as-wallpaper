use std::{cell::OnceCell, sync::Mutex};

use windows::{
    core::s,
    Win32::{
        Devices::HumanInterfaceDevice::{
            HID_USAGE_GENERIC_KEYBOARD, HID_USAGE_GENERIC_MOUSE, HID_USAGE_PAGE_GENERIC,
        },
        Foundation::{HINSTANCE, HWND, LPARAM, LRESULT, POINT, WPARAM},
        Graphics::Gdi::ScreenToClient,
        System::LibraryLoader::GetModuleHandleW,
        UI::{Input, WindowsAndMessaging},
    },
};

use windows::Win32::UI::Input::RAWINPUT;

struct WindowContext {
    window_handle: HWND,
    forward_mouse_input: bool,
    forward_keyboard_input: bool,
}

static mut WINDOWS: OnceCell<Mutex<Vec<WindowContext>>> = OnceCell::new();

static mut RAW_INPUT_WINDOW: Option<HWND> = None;

fn make_wparam(low: usize, high: usize) -> WPARAM {
    let combined = (high << 16) | low;
    WPARAM(combined)
}

fn make_lparam(low: isize, high: isize) -> LPARAM {
    let combined = (high << 16) | low;
    LPARAM(combined)
}

fn send_mouse_input(msg: u32, point: POINT, w_param: WPARAM) {
    unsafe {
        let windows: &Mutex<Vec<WindowContext>> = WINDOWS.get().unwrap();
        let windows_lock = windows.lock().unwrap();

        for window in windows_lock.iter() {
            let mut client_point = point.clone();

            if window.forward_mouse_input {
                ScreenToClient(window.window_handle, &mut client_point).unwrap();

                let l_param = make_lparam(client_point.x as isize, client_point.y as isize);

                WindowsAndMessaging::PostMessageA(window.window_handle, msg, w_param, l_param)
                    .unwrap();
            }
        }
    }
}

fn handle_mouse_input(raw_data: &RAWINPUT) {
    let mouse_data = unsafe { raw_data.data.mouse };
    let mouse_button_flags = unsafe { mouse_data.Anonymous.Anonymous.usButtonFlags } as u32;

    let mut point = POINT::default();

    unsafe {
        WindowsAndMessaging::GetCursorPos(&mut point).unwrap();
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_1_DOWN != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_LBUTTONDOWN,
            point,
            WPARAM::default(),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_1_UP != 0 {
        send_mouse_input(WindowsAndMessaging::WM_LBUTTONUP, point, WPARAM::default());
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_2_DOWN != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_RBUTTONDOWN,
            point,
            WPARAM::default(),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_2_UP != 0 {
        send_mouse_input(WindowsAndMessaging::WM_RBUTTONUP, point, WPARAM::default());
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_3_DOWN != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_MBUTTONDOWN,
            point,
            WPARAM::default(),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_3_UP != 0 {
        send_mouse_input(WindowsAndMessaging::WM_MBUTTONUP, point, WPARAM::default());
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_4_DOWN != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_XBUTTONDOWN,
            point,
            make_wparam(0, WindowsAndMessaging::XBUTTON1 as usize),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_4_UP != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_XBUTTONUP,
            point,
            make_wparam(0, WindowsAndMessaging::XBUTTON1 as usize),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_5_DOWN != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_XBUTTONDOWN,
            point,
            make_wparam(0, WindowsAndMessaging::XBUTTON2 as usize),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_BUTTON_5_UP != 0 {
        send_mouse_input(
            WindowsAndMessaging::WM_XBUTTONUP,
            point,
            make_wparam(0, WindowsAndMessaging::XBUTTON2 as usize),
        );
        return;
    }

    if mouse_button_flags & WindowsAndMessaging::RI_MOUSE_HWHEEL != 0 {
        return;
    }

    send_mouse_input(WindowsAndMessaging::WM_MOUSEMOVE, point, WPARAM::default());
}

fn send_keyboard_input(msg: u32, w_param: WPARAM, l_param: LPARAM) {
    unsafe {
        let windows: &Mutex<Vec<WindowContext>> = WINDOWS.get().unwrap();
        let windows_lock = windows.lock().unwrap();

        for window in windows_lock.iter() {
            if window.forward_keyboard_input {
                WindowsAndMessaging::PostMessageA(window.window_handle, msg, w_param, l_param)
                    .unwrap();
            }
        }
    }
}

// ref: https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown#parameters
fn keyboard_lparam(flags: u16, make_code: u16) -> LPARAM {
    // Determine whether the key is pressed (0 = key down, 1 = key up)
    let is_pressed = (flags as u32 & WindowsAndMessaging::RI_KEY_BREAK) == 0;

    // For keydown, we set repeat count to 0 initially
    // But for keyup, we set repeat count to 1
    let repeat_count = if is_pressed { 0 } else { 1 };

    // The scan code (make_code)
    let scan_code = make_code as u32;

    // Determine whether the key is an extended key
    let is_extended = if scan_code > 0x80 { 1 } else { 0 };

    // The value is always 0 for a WM_KEYDOWN message.
    let context_code = 0;

    // Previous key state (1 if the key is pressed, otherwise 0)
    let previous_key_state = if is_pressed { 1 } else { 0 };

    // Transition state: 0 for key down, 1 for key up
    let transition_state = if is_pressed { 0 } else { 1 };

    // Construct the lParam using bitwise operations:
    // - repeat_count occupies bits 0-15
    // - scan_code occupies bits 16-23
    // - is_extended occupies bit 24
    // - context_code occupies bit 29
    // - previous_key_state occupies bit 30
    // - transition_state occupies bit 31
    let l_param = (repeat_count << 0)
        | (scan_code << 16)
        | (is_extended << 24)
        | (context_code << 29)
        | (previous_key_state << 30)
        | (transition_state << 31);

    LPARAM(l_param as isize)
}

fn handle_keyboard_input(raw_data: &RAWINPUT) {
    let keyboard_data = unsafe { raw_data.data.keyboard };

    let message = keyboard_data.Message;
    let key_code = keyboard_data.VKey;
    let flags = keyboard_data.Flags;
    let make_code = keyboard_data.MakeCode;

    let w_param = WPARAM(key_code as usize);
    let l_param = keyboard_lparam(flags, make_code);

    send_keyboard_input(message, w_param, l_param);
}

fn read_input(l_param: LPARAM) {
    let mut raw_data = RAWINPUT {
        ..Default::default()
    };
    let mut raw_data_size = size_of::<RAWINPUT>() as u32;
    let raw_data_header_size = size_of::<Input::RAWINPUTHEADER>() as u32;

    let raw_input_data = unsafe {
        Input::GetRawInputData(
            Input::HRAWINPUT(l_param.0 as _),
            Input::RID_INPUT,
            Some(&mut raw_data as *mut _ as *mut _),
            &mut raw_data_size,
            raw_data_header_size,
        )
    };

    if raw_input_data == u32::MAX {
        return;
    }

    let device_type = Input::RID_DEVICE_INFO_TYPE(raw_data.header.dwType);

    if device_type == Input::RIM_TYPEMOUSE {
        handle_mouse_input(&raw_data);
        return;
    }

    if device_type == Input::RIM_TYPEKEYBOARD {
        handle_keyboard_input(&raw_data);
        return;
    }
}

unsafe extern "system" fn handle_window_message(
    hwnd: HWND,
    msg: u32,
    w_param: WPARAM,
    l_param: LPARAM,
) -> LRESULT {
    // Handle raw input messages
    if msg == WindowsAndMessaging::WM_INPUT {
        read_input(l_param);
    }

    if msg == WindowsAndMessaging::WM_CLOSE {
        println!("Raw input window closing");
    }

    // Call the default window procedure
    WindowsAndMessaging::DefWindowProcA(hwnd, msg, w_param, l_param)
}

pub fn start_input_forwarding(hwnd: HWND, forward_mouse_input: bool, forward_keyboard_input: bool) {
    // If we are not forwarding mouse or keyboard input, return
    if !forward_mouse_input && !forward_keyboard_input {
        return;
    }

    unsafe {
        let windows = WINDOWS.get_or_init(|| Mutex::new(Vec::new()));
        let mut windows_lock = windows.lock().unwrap();
        windows_lock.push(WindowContext {
            window_handle: hwnd,
            forward_mouse_input,
            forward_keyboard_input,
        });
    }

    unsafe {
        // if we are already forwarding input, return
        if RAW_INPUT_WINDOW.is_some() {
            return;
        }
    }

    unsafe {
        // Create a window to receive raw input messages
        let h_instance = GetModuleHandleW(None).unwrap();

        // Create a window class for the raw input window
        let wnd_class = WindowsAndMessaging::WNDCLASSA {
            lpfnWndProc: Some(handle_window_message),
            hInstance: HINSTANCE::from(h_instance),
            lpszClassName: s!("RawInputWindowClass"),
            ..WindowsAndMessaging::WNDCLASSA::default()
        };

        // Register the window class
        WindowsAndMessaging::RegisterClassA(&wnd_class);

        // Create the raw input window
        let raw_input_window = WindowsAndMessaging::CreateWindowExA(
            WindowsAndMessaging::WINDOW_EX_STYLE::default(),
            wnd_class.lpszClassName,
            s!("RawInputWindow"),
            WindowsAndMessaging::WINDOW_STYLE::default(),
            0,
            0,
            0,
            0,
            None,
            None,
            h_instance,
            None,
        )
        .unwrap();

        RAW_INPUT_WINDOW = Some(raw_input_window);

        // Input devices to forward
        let inputs: [Input::RAWINPUTDEVICE; 2] = [
            Input::RAWINPUTDEVICE {
                hwndTarget: raw_input_window,
                usUsagePage: HID_USAGE_PAGE_GENERIC,
                usUsage: HID_USAGE_GENERIC_MOUSE,
                dwFlags: Input::RIDEV_INPUTSINK,
            },
            Input::RAWINPUTDEVICE {
                hwndTarget: raw_input_window,
                usUsagePage: HID_USAGE_PAGE_GENERIC,
                usUsage: HID_USAGE_GENERIC_KEYBOARD,
                dwFlags: Input::RIDEV_INPUTSINK,
            },
        ];

        // Register the input devices
        Input::RegisterRawInputDevices(&inputs, size_of::<Input::RAWINPUTDEVICE>() as u32).unwrap();
    }
}

pub fn stop_input_forwarding(hwnd: HWND) {
    unsafe {
        if let Some(windows) = WINDOWS.get() {
            let mut windows_lock = windows.lock().unwrap();
            windows_lock.retain(|window| window.window_handle != hwnd);

            if windows_lock.is_empty() {
                if let Some(raw_input_window) = RAW_INPUT_WINDOW {
                    WindowsAndMessaging::DestroyWindow(raw_input_window).unwrap();
                    RAW_INPUT_WINDOW = None;
                }
            }
        }
    }
}

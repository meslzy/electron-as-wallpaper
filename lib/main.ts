import bindings from "bindings";
import { BrowserWindow } from "electron";

const neon = bindings("neon");

declare module "electron" {
  interface BrowserWindow {
    wallpaperState?: {
      isAttached?: boolean;
      isTransparent?: boolean;
      isForwardMouseInput?: boolean;
      isForwardKeyboardInput?: boolean;
    };
  }
}

const getNativeWindowHandle = (window: BrowserWindow) => {
  let buffer = window.getNativeWindowHandle();
  return buffer.readUInt32LE(0);
};

interface AttachOptions {
  /**
   * Makes the window transparent.
   * platform: Windows
   * @default false
   */
  transparent?: boolean;
  /**
   * Forward mouse input-forwarding to the window.
   * platform: Windows
   * @default false
   */
  forwardMouseInput?: boolean;
  /**
   * Forward keyboard input-forwarding to the window.
   * platform: Windows
   * @default false
   */
  forwardKeyboardInput?: boolean;
}

const attachOptions: AttachOptions = {
  transparent: false,
  forwardMouseInput: false,
  forwardKeyboardInput: false,
};

export const attach = (window: BrowserWindow, options?: AttachOptions) => {
  if (!(window instanceof BrowserWindow)) {
    throw new Error("window must be an instance of Electron.BrowserWindow");
  }

  const nativeWindowHandle = getNativeWindowHandle(window);

  neon.attach(nativeWindowHandle, {
    ...attachOptions,
    ...options,
  });

  window.wallpaperState = {
    isAttached: true,
    isTransparent: attachOptions.transparent,
    isForwardMouseInput: attachOptions.forwardMouseInput,
    isForwardKeyboardInput: attachOptions.forwardKeyboardInput,
  };
};

export const detach = (window: BrowserWindow) => {
  if (!(window instanceof BrowserWindow)) {
    throw new Error("window must be an instance of Electron.BrowserWindow");
  }

  if (!window.wallpaperState?.isAttached) {
    return;
  }

  const nativeWindowHandle = getNativeWindowHandle(window);

  neon.detach(nativeWindowHandle, {
    transparent: window.wallpaperState?.isTransparent,
    forwardMouseInput: window.wallpaperState?.isForwardMouseInput,
    forwardKeyboardInput: window.wallpaperState?.isForwardKeyboardInput,
  });

  window.wallpaperState = {
    isAttached: false,
    isTransparent: false,
    isForwardMouseInput: false,
  };
};

export const reset = () => {
  neon.reset();
};

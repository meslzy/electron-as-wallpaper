import type {BrowserWindow} from "electron";

import bindings from "bindings";

const electronAsWallpaper = bindings("electron-as-wallpaper");

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

/**
 * Set window behind desktop icons
 * platform: Windows
 */
const attach = (win: BrowserWindow, options?: AttachOptions) => {
  if (win === undefined) {
    throw Error("You need to pass a window to be able to attaching");
  }

  if (typeof win.getNativeWindowHandle !== "function") {
    throw Error("You need too pass a window type of Electron.BrowserWindow");
  }

  electronAsWallpaper.attach(win.getNativeWindowHandle(), {
    ...attachOptions,
    ...options,
  });
};

/**
 * Remove a window from desktop icons
 * platform: Windows
 */
const detach = (win: BrowserWindow) => {
  if (win === undefined) {
    throw Error("You need to pass a window to be able to detaching");
  }

  if (typeof win.getNativeWindowHandle !== "function") {
    throw Error("You need too pass a window type of Electron.BrowserWindow");
  }

  electronAsWallpaper.detach(win.getNativeWindowHandle());
};

/**
 * Setting default desktop wallpaper
 * platform: Windows
 */
const refresh = () => {
  electronAsWallpaper.refresh();
};

export type {
  AttachOptions,
};

export {
  attach,
  detach,
  refresh,
};

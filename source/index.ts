import {BrowserWindow} from "electron";

const bindings = require("bindings")("electron-as-wallpaper");

interface AttachOptions {
	/**
	 * Makes the window transparent.
	 * @default false
	 */
	transparent?: boolean;
}

const attachOptions: AttachOptions = {
	transparent: false,
};

/**
 * Set window behind desktop icons
 **/
export const attach = (win: BrowserWindow, options?: AttachOptions) => {
	options = Object.assign({}, attachOptions, options);

	if (win === undefined) throw Error("You need to pass a window to be able to attaching");

	if (typeof win.getNativeWindowHandle !== "function") throw Error("You need too pass a window of type Electron.BrowserWindow");

	bindings.attach(win.getNativeWindowHandle(), options);
};

/**
 * Remove window from desktop icons
 **/
export const detach = (win: BrowserWindow) => {
	if (win === undefined) throw Error("You need to pass a window to be able to detaching");
	
	if (typeof win.getNativeWindowHandle !== "function") throw Error("You need too pass a window of type Electron.BrowserWindow");
	
	bindings.detach(win.getNativeWindowHandle());
};

/**
 * Refresh desktop
 **/
export const refresh = () => bindings.refresh();

export default {
	attach,
	detach,
	refresh
};

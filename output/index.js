"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.refresh = exports.detach = exports.attach = void 0;
const bindings = require("bindings")("electron-as-wallpaper");
const attachOptions = {
    transparent: false,
};
/**
 * Set window behind desktop icons
 **/
const attach = (win, options) => {
    options = Object.assign({}, attachOptions, options);
    if (win === undefined)
        throw Error("You need to pass a window to be able to attaching");
    if (typeof win.getNativeWindowHandle !== "function")
        throw Error("You need too pass a window of type Electron.BrowserWindow");
    bindings.attach(win.getNativeWindowHandle(), options);
};
exports.attach = attach;
/**
 * Remove window from desktop icons
 **/
const detach = (win) => {
    if (win === undefined)
        throw Error("You need to pass a window to be able to detaching");
    if (typeof win.getNativeWindowHandle !== "function")
        throw Error("You need too pass a window of type Electron.BrowserWindow");
    bindings.detach(win.getNativeWindowHandle());
};
exports.detach = detach;
/**
 * Refresh desktop
 **/
const refresh = () => bindings.refresh();
exports.refresh = refresh;
exports.default = {
    attach: exports.attach,
    detach: exports.detach,
    refresh: exports.refresh
};

import {app, BrowserWindow, ipcMain, screen} from "electron";

import {attach, detach, refresh} from "../../dist/main.js";

import url from "node:url";
import path from "node:path";

const filename = url.fileURLToPath(import.meta.url);
const dirname = path.dirname(filename);

/**
 * @type {Electron.CrossProcessExports.BrowserWindow}
 */
let win;

/**
 * @type {Electron.CrossProcessExports.BrowserWindow}
 */
let wallpaper;

ipcMain.handle("url", (event, url) => {
  return wallpaper.loadURL(url);
});

ipcMain.handle("attach", () => {
  attach(wallpaper);
});

ipcMain.handle("detach", () => {
  detach(wallpaper);
});

ipcMain.handle("refresh", () => {
  refresh();
});

ipcMain.handle("show", () => {
  wallpaper.show();
});

ipcMain.handle("hide", () => {
  wallpaper.hide();
});

app.on("ready", async () => {
  win = new BrowserWindow({
    width: 600,
    height: 400,
    alwaysOnTop: true,
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(dirname, "preload.mjs"),
    },
  });

  wallpaper = new BrowserWindow({
    show: false,
    frame: false,
    enableLargerThanScreen: true,
    autoHideMenuBar: true,
    webPreferences: {
      backgroundThrottling: false,
    },
  });

  wallpaper.setBounds(screen.getPrimaryDisplay().bounds);

  await win.loadURL(`file://${path.join(dirname, "index.html")}`);

  win.show();
});

import path from "node:path";
import url from "node:url";

import { app, screen, ipcMain, BrowserWindow } from "electron";

import { reset, attach, detach } from "../../dist/main.js";

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

ipcMain.handle("reset", () => {
  reset();
});

ipcMain.handle("show", () => {
  wallpaper.show();
});

ipcMain.handle("hide", () => {
  wallpaper.hide();
});

app.on("ready", async () => {
  wallpaper = new BrowserWindow({
    show: false,
    frame: false,
    enableLargerThanScreen: true,
    autoHideMenuBar: true,
    webPreferences: {
      backgroundThrottling: false,
    },
  });

  wallpaper.on("close", () => {
    detach(wallpaper);
  });

  wallpaper.setBounds(screen.getPrimaryDisplay().bounds);

  win = new BrowserWindow({
    width: 600,
    height: 400,
    alwaysOnTop: true,
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(dirname, "preload.mjs"),
    },
  });

  await win.loadURL(`file://${dirname}/index.html`);

  win.on("closed", () => {
    wallpaper.close();
  });

  win.show();
});

app.on("quit", () => {
  reset();
});

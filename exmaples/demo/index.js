const {app, screen, ipcMain, BrowserWindow} = require("electron");

const {attach, detach, refresh} = require("../../dist/main");

const path = require("node:path");

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
      preload: path.join(__dirname, "preload.js"),
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

  await win.loadURL(`file://${path.join(__dirname, "index.html")}`);

  win.show();
});

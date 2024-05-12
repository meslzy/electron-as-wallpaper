import {app, BrowserWindow, screen} from "electron";

import {attach, refresh} from "../../dist/main.js";

app.on("ready", async () => {
  const win = new BrowserWindow({
    transparent: true,
    autoHideMenuBar: true,
    show: false,
    frame: false,
  });

  const size = screen.getPrimaryDisplay().workAreaSize;

  win.setBounds({
    x: 0,
    y: 0,
    width: size.width,
    height: size.height,
  });

  await win.loadURL("https://www.google.com/");

  win.webContents.openDevTools({
    mode: "detach",
  });

  attach(win, {
    transparent: true,
    forwardKeyboardInput: true,
    forwardMouseInput: true,
  });

  win.show();
});

app.on("quit", () => {
  refresh();
});

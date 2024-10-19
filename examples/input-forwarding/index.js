import { app, screen, BrowserWindow } from "electron";

import { reset, attach } from "../../dist/main.js";

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

  win.on("close", () => {
    detach(wallpaper);
  });

  attach(win, {
    transparent: true,
    forwardKeyboardInput: true,
    forwardMouseInput: true,
  });

  await win.loadURL("https://www.google.com/");

  win.show();
});

app.on("quit", () => {
  reset();
});

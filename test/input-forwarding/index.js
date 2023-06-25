const {app, BrowserWindow, screen} = require("electron");
const {attach, detach} = require("../../dist/index");

app.on("ready", async () => {
  const win = new BrowserWindow({
    autoHideMenuBar: true,
    show: false,
    frame: false,
    transparent: true,
  });

  const size = screen.getPrimaryDisplay().size;

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
    forwardKeyboardInput: true,
    forwardMouseInput: true,
  });

  win.show();
});

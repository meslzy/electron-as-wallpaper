const {app, BrowserWindow, screen} = require("electron");
const {attach, detach} = require("../../dist/index");

app.on("ready", async () => {
  const win = new BrowserWindow({
    transparent: true,
    autoHideMenuBar: true,
    show: false,
    frame: false,
  });

  const size = screen.getPrimaryDisplay().size;

  win.setBounds({
    x: 0,
    y: 200,
    width: size.width,
    height: size.height - 200,
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

  setTimeout(() => {
    detach(win);
  }, 10000);
});

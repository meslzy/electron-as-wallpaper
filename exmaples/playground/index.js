const {app, BrowserWindow, Tray, Menu} = require("electron");

const {attach, detach, refresh} = require("../../dist/main");

app.on("ready", async () => {
  const win = new BrowserWindow({
    width: 500,
    height: 500,
    show: false,
    transparent: true,
    autoHideMenuBar: true,
  });

  const tray = new Tray(`${__dirname}/icon.png`);

  const contextMenu = Menu.buildFromTemplate([{
    id: "attach", label: "Attach", click: () => {
      try {
        attach(win, {
          transparent: true,
          forwardMouseInput: true,
          forwardKeyboardInput: true,
        });
      } catch (e) {
        console.error(e);
      }
    },
  }, {
    id: "detach", label: "Detach", click: () => {
      try {
        detach(win);
      } catch (e) {
        console.error(e);
      }
    },
  }, {
    id: "refresh", label: "Refresh", click: () => {
      try {
        refresh();
      } catch (e) {
        console.error(e);
      }
    },
  }, {
    id: "quit", label: "Quit", click: () => {
      app.quit();
    },
  }]);

  tray.setContextMenu(contextMenu);

  await win.loadURL(`file://${__dirname}/index.html`);

  win.webContents.openDevTools({
    mode: "detach",
  });

  win.show();
});

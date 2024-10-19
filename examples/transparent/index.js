import path from "node:path";
import url from "node:url";

import { app, BrowserWindow } from "electron";

import { reset, attach, detach } from "../../dist/main.js";

const filename = url.fileURLToPath(import.meta.url);
const dirname = path.dirname(filename);

app.on("ready", async () => {
  const win = new BrowserWindow({
    width: 675,
    height: 680,
    enableLargerThanScreen: true,
    autoHideMenuBar: true,
    show: false,
    frame: false,
    transparent: true,
  });

  await win.loadURL(`file://${dirname}/index.html`);

  win.webContents.openDevTools({
    mode: "detach",
  });

  attach(win, {
    transparent: true,
  });

  win.on("close", () => {
    detach(win);
  });

  win.show();
});

app.on("quit", () => {
  reset();
});

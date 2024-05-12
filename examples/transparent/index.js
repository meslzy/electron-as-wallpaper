import {app, BrowserWindow} from "electron";

import {attach} from "../../dist/main.js";

import url from "node:url";
import path from "node:path";

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

  try {
    attach(win, {
      transparent: true,
    });
    win.show();
  } catch (e) {
    console.log(e);
  }
});

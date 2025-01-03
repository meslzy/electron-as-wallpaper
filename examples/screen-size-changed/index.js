import { app, screen, BrowserWindow } from "electron";

import { reset, attach, detach } from "../../dist/main.js";

app.on("ready", async () => {
  const win1 = new BrowserWindow({
    enableLargerThanScreen: true,
    autoHideMenuBar: true,
    frame: false,
    show: false,
    webPreferences: {
      backgroundThrottling: false,
    },
  });
  const win2 = new BrowserWindow({
    enableLargerThanScreen: true,
    autoHideMenuBar: true,
    frame: false,
    show: false,
    webPreferences: {
      backgroundThrottling: false,
    },
  });

  await win1.loadURL("https://www.nayuki.io/res/full-screen-clock-javascript/full-screen-clock-12hr-with-seconds.html");
  await win2.loadURL("https://www.nayuki.io/res/full-screen-clock-javascript/full-screen-clock-12hr-with-seconds.html");

  const [display1, display2] = screen.getAllDisplays();

  // set the first screen bounds to the first window
  win1.setBounds(display1.bounds);
  // set the second screen bounds to the second window
  win2.setBounds(display2.bounds);

  // when display resolution changed
  screen.on("display-metrics-changed", () => {
    const [display1, display2] = screen.getAllDisplays();

    // set the first screen new bounds to the first window again
    win1.setBounds(display1.bounds);
    // set the second screen new bounds to the second window again
    win2.setBounds(display2.bounds);
  });

  win1.on("close", () => {
    detach(win1);
  });

  win2.on("close", () => {
    detach(win2);
  });

  attach(win1);
  win1.show();

  attach(win2);
  win2.show();
});

app.on("quit", () => {
  reset();
});

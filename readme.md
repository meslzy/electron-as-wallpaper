# Electron As Wallpaper (✨)

> set your electron window as wallpaper behind desktop icons

---

<div align="center">

![license](https://badgen.net/badge/license/MIT/blue)
![stars](https://badgen.net/npm/v/electron-as-wallpaper)
![forks](https://badgen.net/npm/dw/electron-as-wallpaper)
![issues](https://badgen.net/github/open-issues/meslzy/electron-as-wallpaper)

</div>

---

## Features (⭐)

- [x] set electron window as wallpaper
- [x] support transparent window
- [x] support mouse forwarding
- [x] support keyboard forwarding

## Requirements (⚙️)

- Window OS
  - Window 11 (Tested)
  - Window 10 (Not Tested)
  - Window 8 (Not Tested)
  - Window 7 (Not Tested)
- [Rust](https://www.rust-lang.org/)
- [Node.js](https://nodejs.org/)
- [Electron](https://www.electronjs.org/)
- [Electron Rebuild](https://www.electronjs.org/docs/latest/tutorial/using-native-node-modules)

## Installation (⏬)

```bash
npm install electron-as-wallpaper@2.0.0 --save
```

## How to use (🌠)

```ts
import {attach, detach, reset} from "electron-as-wallpaper";

// attach the window to the desktop wallpaper
attach(mainWindow, {
  transparent: true,
  forwardKeyboardInput: true,
  forwardMouseInput: true,
});

// detach the window from the desktop wallpaper
detach(mainWindow);

// reset the wallpaper (restore the original wallpaper)
reset();
```

## Examples

- [demo](examples/demo/index.js)
- [transparent](examples/transparent/index.js)
- [input forwarding](examples/input-forwarding/index.js)
- [screen size changed](examples/screen-size-changed/index.js)

---

## The End (💘)

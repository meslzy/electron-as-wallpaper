# Electron As Wallpaper (✨)

> set your electron window as wallpaper behind desktop icons
---

<div align="center">

![license](https://badgen.net/badge/license/MIT/blue)
![stars](https://badgen.net/npm/v/electron-as-wallpaper)
![forks](https://badgen.net/npm/dw/electron-as-wallpaper)
![issues](https://badgen.net/github/issues/meslzy/electron-as-wallpaper)

</div>

---

<div align="center">

![issues](assets/app.gif)

<a href="https://github.com/meslzy/electron-as-wallpaper">`try the app`</a>

</div>

---

## Getting Started (✅)

- ### Installation (⏬)
	- `npm install electron-as-wallpaper --save`

- ### How to use (🌠)
  ```js
	 import {attach, detach, refresh} from "electron-as-wallpaper";
	 // or
	 const {attach, detach, refresh} = require("electron-as-wallpaper");
	 
  const hWind = mainWindow.getNativeWindowHandle();
  
  attach(hWind);
  detach(hWind);
  
  // if the window closed before detach, you can call refresh
  refresh();
	```
---

## The End (💘)

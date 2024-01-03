const {contextBridge, ipcRenderer} = require("electron");

contextBridge.exposeInMainWorld("ipc", {
  url: (url) => ipcRenderer.invoke("url", url),
  //
  attach: () => ipcRenderer.invoke("attach"),
  detach: () => ipcRenderer.invoke("detach"),
  refresh: () => ipcRenderer.invoke("refresh"),
  //
  show: () => ipcRenderer.invoke("show"),
  hide: () => ipcRenderer.invoke("hide"),
});
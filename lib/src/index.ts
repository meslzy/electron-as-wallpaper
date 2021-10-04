const bindings = require('bindings')('electron-as-wallpaper');

/**
 * set window behind desktop icons
 **/
export const attach: (hWind: Buffer) => true | null = (hWind: Buffer) => bindings.Attach(hWind);

/**
 * remove window from desktop icons
 **/
export const detach: (hWind: Buffer) => true | null = (hWind: Buffer) => bindings.Detach(hWind);

/**
 * refresh desktop
 **/
export const refresh = () => bindings.Refresh();

export default {
  attach, detach, refresh
};

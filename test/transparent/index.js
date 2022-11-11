const {app, BrowserWindow} = require("electron");
const {attach} = require("../../dist/index");

app.on("ready", async () => {
	const win = new BrowserWindow({
		width: 675,
		height: 680,
		enableLargerThanScreen: true,
		autoHideMenuBar: true,
		show: false,
		frame: false,
		transparent: true,
		webPreferences: {
			webSecurity: false,
			nodeIntegration: true,
			allowRunningInsecureContent: false,
			webviewTag: true,
			enableRemoteModule: true,
			contextIsolation: false,
			backgroundThrottling: false,
		},
	});

	await win.loadURL(`file://${__dirname}/index.html`);

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

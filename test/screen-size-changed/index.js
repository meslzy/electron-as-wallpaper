const {app, BrowserWindow, screen} = require("electron");
const {attach} = require("../../dist/index");

app.on("ready", async () => {
	const win1 = new BrowserWindow({
		enableLargerThanScreen: true,
		autoHideMenuBar: true,
		frame: false,
		show: false,
		webPreferences: {
			backgroundThrottling: false,
		}
	});
	const win2 = new BrowserWindow({
		enableLargerThanScreen: true,
		autoHideMenuBar: true,
		frame: false,
		show: false,
		webPreferences: {
			backgroundThrottling: false,
		}
	});

	await win1.loadURL("https://www.nayuki.io/res/full-screen-clock-javascript/full-screen-clock-24hr-with-seconds.html");
	await win2.loadURL("https://www.nayuki.io/res/full-screen-clock-javascript/full-screen-clock-24hr-with-seconds.html");

	const displays = screen.getAllDisplays(); // I have 2 display
	// set the first screen bounds to the first window
	win1.setBounds(displays[0].bounds);
	// set the second screen bounds to the second window
	win2.setBounds(displays[1].bounds);

	// when display resolution changed
	screen.on("display-metrics-changed", () => {
		const displays = screen.getAllDisplays(); // i have 2 display
		// set the first screen new bounds to the first window again
		win1.setBounds(displays[0].bounds);
		// set the second screen new bounds to the second window again
		win2.setBounds(displays[1].bounds);
	});
	
	try {
		attach(win1);
		win1.show();
		
		attach(win2);
		win2.show();
	} catch (e) {
		console.log(e);
	}
});

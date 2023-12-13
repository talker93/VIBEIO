const { app, BrowserWindow, ipcMain, nativeTheme } = require('electron')
const path = require('path')

const dgram = require('dgram');
const WebSocket = require('ws');

const udpServer = dgram.createSocket('udp4');
const wss = new WebSocket.Server({ port: 8081 });

udpServer.on('message', (msg, info) => {
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(msg);
        }
    });
});

udpServer.bind(41234);

// const { initializeApp } = require('firebase/app');
// const firebaseConfig = {
//     apiKey: "AIzaSyCpPOh4iI--Zlgs96hB4BFq8n9rGL3y4ks",
//     authDomain: "rtc-server-5d1cf.firebaseapp.com",
//     projectId: "rtc-server-5d1cf",
//     storageBucket: "rtc-server-5d1cf.appspot.com",
//     messagingSenderId: "689362156688",
//     appId: "1:689362156688:web:a2bb68d1624758c56e2c15",
//     measurementId: "G-4DNL0Y0BFR"
// };
// const app2 = initializeApp(firebaseConfig);

function createWindow() {
    const win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            // preload: path.join(__dirname, 'preload.js'),
            nodeIntegration: true,
            contextIsolation: false,
            enableRemoteModule: true,
        }
    })
    // open dev tools
    win.webContents.openDevTools()

    win.loadFile('./renderer/index.html')
}

// ipcMain.handle('dark-mode:toggle', () => {
//     if (nativeTheme.shouldUseDarkColors) {
//         nativeTheme.themeSource = 'light'
//     } else {
//         nativeTheme.themeSource = 'dark'
//     }
//     return nativeTheme.shouldUseDarkColors
// })

// ipcMain.handle('dark-mode:system', () => {
//     nativeTheme.themeSource = 'system'
// })

app.whenReady().then(() => {
    createWindow()

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow()
        }
    })
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit()
    }
})
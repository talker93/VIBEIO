// osc.js

// init the osc client
function oscInit() {
    const { Client } = require('node-osc');
    const client = new Client('127.0.0.1', 9001);
    return client;
}

function oscSendAndPrint(client, target, val) {
    client.send('/juce/' + target, val, () => {
        console.log("sent", val);
    });
}

// Export the functions so they can be used in other modules or the main application.
export { oscInit, oscSendAndPrint };

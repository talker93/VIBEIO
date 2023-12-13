// debuggingUtils.js

// The stateCheck function logs the WebRTC connection states for debugging purposes.
function stateCheck(pc) {
    pc.onicegatheringstatechange = e => {
        console.log("iceGatheringState: ", pc.iceGatheringState);
        checkTime();
    }
    pc.oniceconnectionstatechange = e => {
        console.log("iceConnectionState: ", pc.iceConnectionState);
        checkTime();
    }
    pc.onconnectionstatechange = e => {
        console.log("connectionState: ", pc.connectionState);
        checkTime();
    }
}

// The checkTime function logs the current time for debugging purposes.
function checkTime() {
    const millisecs = Math.floor(Date.now() % 100000);
    console.log("time elapsed: ", millisecs);
}

// Export the functions so they can be used in other modules or the main application.
export { stateCheck, checkTime };

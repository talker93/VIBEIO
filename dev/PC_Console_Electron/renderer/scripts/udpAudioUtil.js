// udpAudioUtil.js

async function startUdpAudio() {
    const context = new AudioContext({ sampleRate: 48000 });
    // window.context = context
    await context.audioWorklet.addModule('./scripts/audio-processor.js');
    const processorNode = new AudioWorkletNode(context, 'audio-processor');
    // processorNode.connect(context.destination);

    var dest = context.createMediaStreamDestination();
    const processedStream = dest.stream;
    processorNode.connect(dest);

    // auto resume audio context as users click anywhere on the page
    function resumeAudioContext() {
        console.log("hey", context.state);
        if (context.state === "suspended") {
            context.resume().then(() => {
                console.log("resumed");
                document.removeEventListener('click', resumeAudioContext);
                document.removeEventListener('touchend', resumeAudioContext);
            });
        }
    }
    document.addEventListener('click', resumeAudioContext);
    document.addEventListener('touchend', resumeAudioContext);

    const ws = new WebSocket('ws://localhost:8081');
    ws.binaryType = 'arraybuffer';

    ws.onmessage = (event) => {
        const audioData = new Float32Array(event.data);

        if (audioData.length > 0) {
            processorNode.port.postMessage(audioData);
        } else {
            console.warn("Received empty audio data.");
        }
    };

    return processedStream;
}

export { startUdpAudio };


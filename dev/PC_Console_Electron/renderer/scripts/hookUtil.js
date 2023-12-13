// hookUtil.js

// init all hooks
function hookInit() {
    const webcamButton = document.getElementById('webcamButton');
    const webcamVideo = document.getElementById('webcamVideo');
    const callButton = document.getElementById('callButton');
    const callInput = document.getElementById('callInput');
    const answerButton = document.getElementById('answerButton');
    const remoteVideo = document.getElementById('remoteVideo');
    const hangupButton = document.getElementById('hangupButton');
    const callMessage = document.getElementById('callMessage');

    const remoteVideoElement = document.getElementById('remoteVideo');
    const audioInputSelect = document.getElementById('audioSource');
    const audioOutputSelect = document.getElementById('audioOutput');
    const selectors = [audioInputSelect, audioOutputSelect];

    return {
        webcamButton,
        webcamVideo,
        callButton,
        callInput,
        answerButton,
        remoteVideo,
        hangupButton,
        callMessage,
        remoteVideoElement,
        audioInputSelect,
        audioOutputSelect,
        selectors
    }
}

function gotDevices(deviceInfos, audioInputSelect, audioOutputSelect) {
    for (let i = 0; i !== deviceInfos.length; ++i) {
        const deviceInfo = deviceInfos[i];
        const option = document.createElement('option');
        option.value = deviceInfo.deviceId;
        if (deviceInfo.kind === 'audioinput') {
            option.text = deviceInfo.label || `microphone ${audioInputSelect.length + 1}`;
            audioInputSelect.appendChild(option);
        } else if (deviceInfo.kind === 'audiooutput') {
            option.text = deviceInfo.label || `speaker ${audioOutputSelect.length + 1}`;
            audioOutputSelect.appendChild(option);
        }
    }
}

function changeAudioDestination(audioOutputSelect, remoteVideoElement) {
    const audioDestination = audioOutputSelect.value;
    remoteVideoElement.setSinkId(audioDestination)
        .then(() => {
            console.log('success');
        })
        .catch((error) => {
            console.log('Error: ', error);
        });
}

function mediaConstructor(audioInputSelect) {
    const audioSource = audioInputSelect.value;
    const constraints = {
        audio: {
            deviceId: audioSource,
            echoCancellation: false,
            noiseSuppression: false,
            autoGainControl: false,
            latency: 0,
            sampleRate: 48000,
            sampleSize: 16,
            volume: 1.0
        },
        video: false
    };
    return (constraints);
}

function mediaQuery(localStream, remoteStream, processedStream, pc,
    webcamVideo, remoteVideo, callButton, answerButton, webcamButton,
    isMuteMe, isMuteMonitor) {
    // push tracks from local stream to peer connections
    processedStream.getTracks().forEach((track) => {
        pc.addTrack(track, processedStream);
    });

    pc.ontrack = (event) => {
        event.streams[0].getTracks().forEach((track) => {
            remoteStream.addTrack(track);
        });
        console.log("remoteStream track: ", event.streams[0].getTracks());
    };

    console.log("localStream: ", localStream.getTracks());
    console.log("channel count: ", localStream.getTracks()[0].getSettings());

    webcamVideo.srcObject = localStream;
    remoteVideo.srcObject = remoteStream;
    webcamVideo.play();
    remoteVideo.play();

    // const node = new MediaStreamAudioDestinationNode(context, { mediaStream: remoteStream })
    // node.connect(networkingNode)

    callButton.disabled = false;
    answerButton.disabled = false;
    webcamButton.disabled = true;


    // mute the local monitor signal
    // const isMuteMonitor = document.getElementById('muteMonitor');
    if (isMuteMonitor.checked == true) {
        webcamVideo.muted = true;
    } else {
        webcamVideo.muted = false;
    }
    isMuteMonitor.addEventListener("change", function () {
        if (isMuteMonitor.checked == true) {
            webcamVideo.muted = true;
            console.log("monitor muted");
        } else {
            webcamVideo.muted = false;
            console.log("monitor unmuted");
        }
    });

    // mute local input
    // function works only when devices started
    // const isMuteMe = document.getElementById('muteMe');
    isMuteMe.checked = false;
    isMuteMe.addEventListener("change", function () {
        if (isMuteMe.checked == true) {
            localStream.getTracks()[0].enabled = false;
            console.log("muted me");
        } else {
            localStream.getTracks()[0].enabled = true;
            console.log("unmuted me");
        }
    });

    return { isMuteMe, isMuteMonitor };
}

export { hookInit, gotDevices, changeAudioDestination, mediaConstructor, mediaQuery };
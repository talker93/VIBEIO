// Step 1. Import and init
// import util functions
import { stateCheck, checkTime } from './scripts/debuggingUtils.js';
// import osc functions
import { oscInit, oscSendAndPrint } from './scripts/oscUtil.js';
// import firebase functions
import { firebaseInit } from './scripts/firebaseUtil.js';
const { getFirestore, collection, addDoc, getDocs, Firestore,
    doc, setDoc, Timestamp, updateDoc, serverTimestamp,
    getDoc, where, query, onSnapshot } = require("firebase/firestore");
// import webrtc functions
import { rtcInit, rtcDataChannelInit, createCall, createAnswer } from './scripts/rtcUtil.js';
// import hook functions
import { hookInit, gotDevices, changeAudioDestination, mediaConstructor, mediaQuery } from './scripts/hookUtil.js';
// import UI Layer for FX Components
import { uiInit } from './scripts/ui.js';
// import web audio functions
import { fxFunctions } from './scripts/audioUtil.js';
import { startUdpAudio } from './scripts/udpAudioUtil.js';

// init osc
const client = oscInit();
// init firebase
const fbStat = firebaseInit();
const servers = fbStat.servers;
const db = fbStat.db;
// init webrtc
const rtcStat = rtcInit(servers);
const pc = rtcStat.pc;
let localStream = rtcStat.localStream;
let remoteStream = rtcStat.remoteStream;
let processedStream = rtcStat.processedStream;
// init webrtc datachannel
const rtcDataStat = rtcDataChannelInit(pc);
let sendChannel = rtcDataStat.sendChannel;
let receiveChannel = rtcDataStat.receiveChannel;
let receiveHandle = rtcDataStat.receiveHandle;
var fxValues = rtcDataStat.fxValues;
// init hooks
const hookStat = hookInit();
const webcamButton = hookStat.webcamButton;
const webcamVideo = hookStat.webcamVideo;
const callButton = hookStat.callButton;
const callInput = hookStat.callInput;
const answerButton = hookStat.answerButton;
const remoteVideo = hookStat.remoteVideo;
const hangupButton = hookStat.hangupButton;
const callMessage = hookStat.callMessage;
const remoteVideoElement = hookStat.remoteVideoElement;
const audioInputSelect = hookStat.audioInputSelect;
const audioOutputSelect = hookStat.audioOutputSelect;
// init web audio components
const uiStat = uiInit(webcamButton, client, pc);
var muteMonitorToggle = uiStat.muteMonitorToggle;
var muteSelfToggle = uiStat.muteSelfToggle;
var audioSourceSelect = uiStat.audioSourceSelect;
var audioOutSelect = uiStat.audioOutSelect;
var startDevices = uiStat.startDevices;
var comp_threshold = uiStat.comp_threshold;
var comp_threshold_val = uiStat.comp_threshold_val;
var comp_knee = uiStat.comp_knee;
var comp_knee_val = uiStat.comp_knee_val;
let comp_ratio = uiStat.comp_ratio;
let comp_ratio_val = uiStat.comp_ratio_val;
let comp_attack = uiStat.comp_attack;
let comp_attack_val = uiStat.comp_attack_val;
let comp_release = uiStat.comp_release;
let comp_release_val = uiStat.comp_release_val;
let comp_bypass = uiStat.comp_bypass;
let rev_type = uiStat.rev_type;
let rev_mix = uiStat.rev_mix;
let rev_bypass = uiStat.rev_bypass;
let eq_bypass = uiStat.eq_bypass;
let eq_type = uiStat.eq_type;
let eq_freq = uiStat.eq_freq;
let eq1 = uiStat.eq1;
let eq2 = uiStat.eq2;
let eq3 = uiStat.eq3;
let gain_bypass = uiStat.gain_bypass;
let gain = uiStat.gain;
let pan_bypass = uiStat.pan_bypass;
let pan = uiStat.pan;
let delayTracking = uiStat.delayTracking;


// Step 2. construct constraints and query media devices
// construct the contraint
navigator.mediaDevices.enumerateDevices()
    .then(deviceInfos => {
        const deviceStat = gotDevices(deviceInfos, audioInputSelect, audioOutputSelect);
    })
    .catch(error => {
        console.log("Error: ", error);
    });

var constraints = mediaConstructor(audioInputSelect);
audioInputSelect.onchange = function () {
    constraints = mediaConstructor(audioInputSelect);
};
audioOutputSelect.onchange = function () {
    changeAudioDestination(audioOutputSelect, remoteVideoElement)
};
// query the media device
webcamButton.onclick = async () => {
    localStream = await navigator.mediaDevices.getUserMedia(constraints);
    remoteStream = new MediaStream();

    // init web audio api
    // processedStream = await fxFunctions(
    //     localStream, fxValues, receiveHandle, sendChannel,
    //     comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
    //     comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
    //     comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
    //     gain_bypass, gain, pan_bypass, pan);

    processedStream = await startUdpAudio(processedStream);

    const isMuteMonitor = document.getElementById('muteMonitor');
    const isMuteMe = document.getElementById('muteMe');
    mediaQuery(localStream, remoteStream, processedStream, pc,
        webcamVideo, remoteVideo, callButton, answerButton, webcamButton,
        isMuteMe, isMuteMonitor);
}


// Step 3. create offer or answer
// offer creation
callButton.onclick = async () => {

    createCall(
        // this is commonly used for both functions
        pc,
        // these arguments are for the callDataChannel
        sendChannel, receiveChannel, receiveHandle, fxValues,
        muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
        comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
        comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
        comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
        gain_bypass, gain, pan_bypass, pan,
        // these arguments are for the callRTC
        callMessage, callButton, answerButton, callInput, hangupButton, db);

};
// answer creation
answerButton.onclick = async () => {

    createAnswer(
        // this is commonly used for both functions
        pc,
        // these arguments are for the callDataChannel
        sendChannel, receiveChannel, receiveHandle, fxValues,
        muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
        comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
        comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
        comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
        gain_bypass, gain, pan_bypass, pan,
        // these arguments are for the callRTC
        callMessage, callButton, answerButton, callInput, hangupButton, db);

};






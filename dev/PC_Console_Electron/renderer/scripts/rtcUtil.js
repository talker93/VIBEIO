// rtcUtil.js
import { stateCheck, checkTime } from "./debuggingUtils.js";
const { getFirestore, collection, addDoc, getDocs, Firestore, doc, setDoc, Timestamp, updateDoc, serverTimestamp, getDoc, where, query, onSnapshot } = require("firebase/firestore");


// init globals for rtc
function rtcInit(servers) {
    const pc = new RTCPeerConnection(servers);
    let localStream = null;
    let remoteStream = null;
    let processedStream = null;
    let sender = null;
    const start = Date.now();
    let millisecs = 0;
    // check status of the peer connection
    stateCheck(pc);
    return { pc, localStream, remoteStream, processedStream };
}

// init globals for rtc datachannel
function rtcDataChannelInit(pc) {
    let sendChannel, receiveChannel;
    let receiveHandle = false;

    sendChannel = pc.createDataChannel("sendDataChannel");
    sendChannel.onopen = e => {
        console.log("send channel opened");
    }
    sendChannel.onclose = e => {
        console.log("send channel closed");
    }

    var fxValues = {
        comp: {
            attack: comp_attack.value,
            bypass: comp_bypass.state,
            knee: comp_knee.value,
            ratio: comp_ratio.value,
            release: comp_release.value,
            threshold: comp_threshold.value
        },
        rev: {
            type: rev_type.value,
            mix: rev_mix.value,
            bypass: rev_bypass.state
        },
        eq: {
            low: eq1.value,
            mid: eq2.value,
            high: eq3.value,
            bypass: eq_bypass.state,
            freq: eq_freq.value,
            type: eq_type.value
        },
        pan: {
            bypass: pan_bypass.state,
            pan: pan.value
        },
        gain: {
            bypass: gain_bypass.state,
            gain: gain.value
        },
        whatChanged: "none"
    };

    return { sendChannel, receiveChannel, receiveHandle, fxValues };
}

// the main function for calls creation
function createCall(
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
    callMessage, callButton, answerButton, callInput, hangupButton, db) {

    callDataChannel(pc, sendChannel, receiveChannel, receiveHandle, fxValues,
        muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
        comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
        comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
        comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
        gain_bypass, gain, pan_bypass, pan);

    callMainRTC(pc, callMessage, callButton, answerButton, callInput, hangupButton, db);
}

// the main function for answer creation
function createAnswer(
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
    callMessage, callButton, answerButton, callInput, hangupButton, db) {
    answerDataChannel(pc, sendChannel, receiveChannel, receiveHandle, fxValues,
        muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
        comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
        comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
        comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
        gain_bypass, gain, pan_bypass, pan);
    answerMainRTC(pc, callMessage, callButton, answerButton, callInput, hangupButton, db);
}

// answer for rtc main thread
async function answerMainRTC(pc, callMessage, callButton, answerButton, callInput, hangupButton, db) {
    let callMsg = document.createTextNode("You answered a call!");
    callMessage.appendChild(callMsg);
    callButton.disabled = true;
    answerButton.disabled = true;
    const callId = callInput.value;
    const callDoc = collection(db, "calls1");
    const answerCandidates = collection(db, 'calls1', callId, 'answerCandidates');
    const offerCandidates = collection(db, 'calls1', callId, 'offerCandidates');

    pc.onicecandidate = (event) => {
        event.candidate && addDoc(answerCandidates, event.candidate.toJSON());
        console.log("PB updated candidate in database");
        checkTime();
    };

    const callData = (await getDoc(doc(callDoc, callId))).data();

    const offerDescription = callData.offer;
    await pc.setRemoteDescription(new RTCSessionDescription(offerDescription));
    console.log("PB set remote desc");
    checkTime();

    const answerDescription = await pc.createAnswer();
    answerDescription.sdp = answerDescription.sdp.replace('useinbandfec=1', 'useinbandfec=1; stereo=1; maxaveragebitrate=510000');
    await pc.setLocalDescription(answerDescription);
    console.log("PB set local desc");
    checkTime();

    const answer = {
        type: answerDescription.type,
        sdp: answerDescription.sdp,
    };

    await updateDoc(doc(db, 'calls1', callId), { answer });
    console.log("PB updated answer in database");
    checkTime();

    const q3 = query(offerCandidates);
    onSnapshot(q3, (snapshot) => {
        snapshot.docChanges().forEach((change) => {
            if (change.type === 'added') {
                let data = change.doc.data();
                let candi = new RTCIceCandidate(data);
                pc.addIceCandidate(candi);
                console.log("PB added ICE Candi: ", candi);
                checkTime();
            }
        });
    });
}

// answer for the data channel thread
function answerDataChannel(pc, sendChannel, receiveChannel, receiveHandle, fxValues,
    muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
    comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
    comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
    comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
    gain_bypass, gain, pan_bypass, pan) {
    // sendChannel = pc.createDataChannel("sendDataChannel2");
    // sendChannel.onopen = e => {
    //     console.log("send channel opened");
    // }
    // sendChannel.onclose = e => {
    //     console.log("send channel closed");
    // }

    pc.ondatachannel = (event) => {
        receiveChannel = event.channel;
        receiveChannel.onmessage = async e => {
            receiveHandle = true;
            fxValues = JSON.parse(e.data);
            switch (fxValues.whatChanged) {
                case 'comp.threshold':
                    await (comp_threshold.value = fxValues.comp.threshold);
                    receiveHandle = false;
                    break;

                case 'comp.release':
                    await (comp_release.value = fxValues.comp.release);
                    receiveHandle = false;
                    break;

                case 'comp.attack':
                    await (comp_attack.value = fxValues.comp.attack);
                    receiveHandle = false;
                    break;

                case 'comp.knee':
                    await (comp_knee.value = fxValues.comp.knee);
                    receiveHandle = false;
                    break;

                case 'comp.ratio':
                    await (comp_ratio.value = fxValues.comp.ratio);
                    receiveHandle = false;
                    break;

                case 'comp.reduction':
                    await (comp_recduction.value = fxValues.comp.reduction);
                    receiveHandle = false;
                    break;

                case 'rev.mix':
                    await (rev_mix.value = fxValues.rev.mix);
                    receiveHandle = false;
                    break;

                case 'rev.type':
                    await (rev_type.value = fxValues.rev.type);
                    receiveHandle = false;
                    break;

                case 'eq.type':
                    await (eq_type.value = fxValues.eq.type);
                    receiveHandle = false;
                    break;

                case 'eq.freq':
                    await (eq_freq.value = fxValues.eq.freq);
                    receiveHandle = false;
                    break;

                case 'eq.low':
                    await (eq1.value = fxValues.eq.low);
                    receiveHandle = false;
                    break;

                case 'eq.mid':
                    await (eq2.value = fxValues.eq.mid);
                    receiveHandle = false;
                    break;

                case 'eq.high':
                    await (eq3.value = fxValues.eq.high);
                    receiveHandle = false;
                    break;

                case 'pan.pan':
                    await (pan.value = fxValues.pan.pan);
                    receiveHandle = false;
                    break;

                case 'gain.gain':
                    await (gain.value = fxValues.gain.gain);
                    receiveHandle = false;
                    break;

                case 'comp.bypass':
                    await (comp_bypass.state = fxValues.comp.bypass);
                    receiveHandle = false;
                    break;

                case 'rev.bypass':
                    await (rev_bypass.state = fxValues.rev.bypass);
                    receiveHandle = false;
                    break;

                case 'eq.bypass':
                    await (eq_bypass.state = fxValues.eq.bypass);
                    receiveHandle = false;
                    break;

                case 'gain.bypass':
                    await (gain_bypass.state = fxValues.gain.bypass);
                    receiveHandle = false;
                    break;

                case 'pan.bypass':
                    await (pan_bypass.state = fxValues.pan.bypass);
                    receiveHandle = false;
                    break;

                default:
                    console.log("invalid exchanged value");
                    break;
            }
        }
        receiveChannel.onopen = e => {
            console.log("receive channel opened");
            checkTime();
        }
        receiveChannel.onclose = e => {
            console.log("receive channel closed");
            checkTime();
        }
    }
}

// offer for the data channel thread
function callDataChannel(pc, sendChannel, receiveChannel, receiveHandle, fxValues,
    muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
    comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
    comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
    comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
    gain_bypass, gain, pan_bypass, pan) {

    // sendChannel = pc.createDataChannel("sendDataChannel");
    // sendChannel.onopen = e => {
    //     console.log("send channel opened");
    // }
    // sendChannel.onclose = e => {
    //     console.log("send channel closed");
    // }

    pc.ondatachannel = (event) => {
        receiveChannel = event.channel;
        receiveChannel.onmessage = async e => {
            receiveHandle = true;
            fxValues = JSON.parse(e.data);
            switch (fxValues.whatChanged) {
                case 'comp.threshold':
                    await (comp_threshold.value = fxValues.comp.threshold);
                    receiveHandle = false;
                    break;

                case 'comp.release':
                    await (comp_release.value = fxValues.comp.release);
                    receiveHandle = false;
                    break;

                case 'comp.attack':
                    await (comp_attack.value = fxValues.comp.attack);
                    receiveHandle = false;
                    break;

                case 'comp.knee':
                    await (comp_knee.value = fxValues.comp.knee);
                    receiveHandle = false;
                    break;

                case 'comp.ratio':
                    await (comp_ratio.value = fxValues.comp.ratio);
                    receiveHandle = false;
                    break;

                case 'comp.reduction':
                    await (comp_recduction.value = fxValues.comp.reduction);
                    receiveHandle = false;
                    break;

                case 'rev.mix':
                    await (rev_mix.value = fxValues.rev.mix);
                    receiveHandle = false;
                    break;

                case 'rev.type':
                    await (rev_type.value = fxValues.rev.type);
                    receiveHandle = false;
                    break;

                case 'eq.type':
                    await (eq_type.value = fxValues.eq.type);
                    receiveHandle = false;
                    break;

                case 'eq.freq':
                    await (eq_freq.value = fxValues.eq.freq);
                    receiveHandle = false;
                    break;

                case 'eq.low':
                    await (eq1.value = fxValues.eq.low);
                    receiveHandle = false;
                    break;

                case 'eq.mid':
                    await (eq2.value = fxValues.eq.mid);
                    receiveHandle = false;
                    break;

                case 'eq.high':
                    await (eq3.value = fxValues.eq.high);
                    receiveHandle = false;
                    break;

                case 'pan.pan':
                    await (pan.value = fxValues.pan.pan);
                    receiveHandle = false;
                    break;

                case 'gain.gain':
                    await (gain.value = fxValues.gain.gain);
                    receiveHandle = false;
                    break;

                case 'comp.bypass':
                    await (comp_bypass.state = fxValues.comp.bypass);
                    receiveHandle = false;
                    break;

                case 'rev.bypass':
                    await (rev_bypass.state = fxValues.rev.bypass);
                    receiveHandle = false;
                    break;

                case 'eq.bypass':
                    await (eq_bypass.state = fxValues.eq.bypass);
                    receiveHandle = false;
                    break;

                case 'gain.bypass':
                    await (gain_bypass.state = fxValues.gain.bypass);
                    receiveHandle = false;
                    break;

                case 'pan.bypass':
                    await (pan_bypass.state = fxValues.pan.bypass);
                    receiveHandle = false;
                    break;

                case 'delayT.delayTracking':

                default:
                    console.log("invalid exchanged value");
                    receiveHandle = false;
                    break;
            }
        }
        receiveChannel.onopen = e => {
            console.log("receive channel opened");
            checkTime();
        }
        receiveChannel.onclose = e => {
            console.log("receive channel closed");
            checkTime();
        }

    }
}

// offer for the rtc main thread
async function callMainRTC(pc, callMessage, callButton, answerButton, callInput, hangupButton, db) {
    let callMsg = document.createTextNode("You created a call.");
    callMessage.appendChild(callMsg);
    callButton.disabled = true;
    answerButton.disabled = true;
    const callDoc = collection(db, 'calls1');
    const callRef = await addDoc(callDoc, {});
    callInput.value = callRef.id;
    const offerCandidates = collection(db, "calls1", callRef.id, "offerCandidates");
    const answerCandidates = collection(db, "calls1", callRef.id, "answerCandidates");

    pc.onicecandidate = (event) => {
        if (event.candidate) {
            addDoc(offerCandidates, event.candidate.toJSON());
        }
        console.log("PA updated candidates in database");
        checkTime();
    };

    const offerDescription = await pc.createOffer().then(console.log("PA created offer"));
    checkTime();
    offerDescription.sdp = offerDescription.sdp.replace('useinbandfec=1', 'useinbandfec=1; stereo=1; maxaveragebitrate=510000');
    await pc.setLocalDescription(offerDescription).then(console.log("PA set local desc"));
    checkTime();

    const offer = {
        sdp: offerDescription.sdp,
        type: offerDescription.type,
    };

    await setDoc(doc(db, 'calls1', callRef.id), { offer }).then(console.log("PA updated offer in database"));
    checkTime();

    // Listen for remote answer
    const q1 = query(doc(db, 'calls1', callRef.id));
    onSnapshot(q1, (snapshot) => {
        const data = snapshot.data();
        if (!pc.currentRemoteDescription && data?.answer) {
            console.log("listen to answer triggered");
            checkTime();
            const answerDescription = new RTCSessionDescription(data.answer);
            pc.setRemoteDescription(answerDescription);
            console.log("PA set remote desc", answerDescription);
            checkTime();
        }
    });

    // When answered, add candidate to peer connection
    const q2 = query(answerCandidates);
    onSnapshot(q2, (snapshot) => {
        snapshot.docChanges().forEach((change) => {
            if (change.type === 'added') {
                console.log("listen to answerCandidates triggered");
                checkTime();
                const candi = new RTCIceCandidate(change.doc.data());
                pc.addIceCandidate(candi);
                console.log("PA added ICE Candi: ", candi);
                // webrtc stats  
                // pc.getStats(null).then(stats => {
                //     stats.forEach(report => {
                //         console.log(report);
                //     });
                // });
                // pc.getStats().then(reports => reports.forEach(report => { if (report.type.includes('candidate-pair')) console.log(report.currentRoundTripTime) }));
                checkTime();
            }
        });
    });

    hangupButton.disabled = false;
}

export {
    rtcInit, rtcDataChannelInit
    , createCall, createAnswer
};
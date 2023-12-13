// audioUtil.js


// Step 6. Codec changes
async function fxFunctions(
    localStream, fxValues, receiveHandle, sendChannel,
    comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
    comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
    comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
    gain_bypass, gain, pan_bypass, pan) {


    // Step 7. Apply FX
    // Init FXs
    var audioCtx = new AudioContext();
    var source = audioCtx.createMediaStreamSource(localStream);
    var dest = audioCtx.createMediaStreamDestination();
    const processedStream = dest.stream;

    var Filter = audioCtx.createBiquadFilter();
    Filter.type = eq_type.value;
    Filter.frequency.value = eq_freq.value;
    Filter.gain.value = gain.value;

    var Panner = audioCtx.createStereoPanner();
    Panner.pan.value = pan.value;

    var Gainner = audioCtx.createGain();
    Gainner.gain.value = gain.value;

    var Compressor = audioCtx.createDynamicsCompressor();
    Compressor.release.value = comp_release.value;

    var Convolver = audioCtx.createConvolver();
    setRevParam(rev_type.value).then("rev set success to: ", rev_type.value);

    source.connect(Compressor);
    Compressor.connect(Filter);
    Filter.connect(Convolver);
    Convolver.connect(Gainner);
    Gainner.connect(Panner);
    Panner.connect(dest);

    // param listener
    comp_threshold.on('change', function (v) {
        Compressor.threshold.value = comp_threshold.value;
        fxValues.comp.threshold = comp_threshold.value;
        fxValues.whatChanged = 'comp.threshold';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            try {
                sendChannel.send(buffer);
            } catch (error) {
                console.error(error);
            }
            console.log("threshold: ", fxValues.comp.threshold);
        }
    });

    comp_release.on('change', function (v) {
        Compressor.release.value = comp_release.value;
        fxValues.comp.release = comp_release.value;
        fxValues.whatChanged = 'comp.release';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    comp_attack.on('change', function (v) {
        Compressor.attack.value = comp_attack.value;
        fxValues.comp.attack = comp_attack.value;
        fxValues.whatChanged = 'comp.attack';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    comp_knee.on('change', function (v) {
        Compressor.knee.value = comp_knee.value;
        fxValues.comp.knee = comp_knee.value;
        fxValues.whatChanged = 'comp.knee';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    comp_ratio.on('change', function (v) {
        Compressor.ratio.value = comp_ratio.value;
        fxValues.comp.ratio = comp_ratio.value;
        fxValues.whatChanged = 'comp.ratio';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    rev_mix.on('change', function (v) {
        // what changes make to Convolver?
        fxValues.rev.mix = rev_mix.value;
        fxValues.whatChanged = 'rev.mix';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    rev_type.on('change', function (v) {
        setRevParam(rev_type.value).then("rev set success to: ", rev_type.value);
        fxValues.rev.type = rev_type.value;
        fxValues.whatChanged = 'rev.type';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq_type.on('change', function (v) {
        Filter.type = eq_type.value;
        fxValues.eq.type = eq_type.value;
        fxValues.whatChanged = 'eq.type';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq_freq.on('change', function (v) {
        Filter.frequency.value = eq_freq.value;
        fxValues.eq.freq = eq_freq.value;
        fxValues.whatChanged = 'eq.freq';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq1.on('change', function (v) {
        Filter.gain.value = eq1.value;
        fxValues.eq.low = eq1.value;
        fxValues.whatChanged = 'eq.low';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq2.on('change', function (v) {
        Filter.gain.value = eq2.value;
        fxValues.eq.mid = eq2.value;
        fxValues.whatChanged = 'eq.mid';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq3.on('change', function (v) {
        Filter.gain.value = eq3.value;
        fxValues.eq.high = eq3.value;
        fxValues.whatChanged = 'eq.high';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    pan.on('change', function (v) {
        Panner.pan.value = pan.value;
        fxValues.pan.pan = pan.value;
        fxValues.whatChanged = 'pan.pan';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    gain.on('change', function (v) {
        Gainner.gain.value = gain.value;
        fxValues.gain.gain = gain.value;
        fxValues.whatChanged = 'gain.gain';
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });


    // Bypass function
    comp_bypass.on('change', function (v) {
        fxValues.comp.bypass = comp_bypass.state;
        fxValues.whatChanged = 'comp.bypass';
        setBypass();
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    rev_bypass.on('change', function (v) {
        fxValues.rev.bypass = rev_bypass.state;
        fxValues.whatChanged = 'rev.bypass';
        setBypass();
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    eq_bypass.on('change', function (v) {
        fxValues.eq.bypass = eq_bypass.state;
        fxValues.whatChanged = 'eq.bypass';
        setBypass();
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    gain_bypass.on('change', function (v) {
        fxValues.gain.bypass = gain_bypass.state;
        fxValues.whatChanged = 'gain.bypass';
        setBypass();
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    pan_bypass.on('change', function (v) {
        fxValues.pan.bypass = pan_bypass.state;
        fxValues.whatChanged = 'pan.bypass';
        setBypass();
        if (!receiveHandle) {
            const buffer = JSON.stringify(fxValues);
            sendChannel.send(buffer);
        }
    });

    var oscilloscope = new Nexus.Oscilloscope('#oScope');
    oscilloscope.connect(source);

    var spectrogram = new Nexus.Spectrogram('#spec');
    spectrogram.connect(source);

    function setBypass() {
        Compressor.disconnect();
        Filter.disconnect();
        Convolver.disconnect();
        Gainner.disconnect();
        Panner.disconnect();
        let bypassState = [false, fxValues.comp.bypass, fxValues.eq.bypass, fxValues.rev.bypass, fxValues.gain.bypass, fxValues.pan.bypass, false];
        var i, j;
        for (i = 0; i < bypassState.length - 1; i++) {
            if (!bypassState[i]) {
                for (j = i + 1; j < bypassState.length; j++) {
                    if (!bypassState[j]) {
                        connectNode(i, j);
                        break;
                    }
                }
            }
        }
    }

    function connectNode(nodeA, nodeB) {
        let componentsList = [source, Compressor, Filter, Convolver, Gainner, Panner, dest];
        componentsList[nodeA].connect(componentsList[nodeB]);
        // console.log(nodeA, " and ", nodeB, " are connected!");
        // console.log(fxValues.pan.bypass);
    }

    async function setRevParam(param) {
        var requestString = 'https://talker93.github.io/pb/audio/' + param + '.wav';
        var soundSource;
        var ajaxRequest = new XMLHttpRequest();
        ajaxRequest.open('GET', requestString, true);
        ajaxRequest.responseType = 'arraybuffer';
        ajaxRequest.onload = function () {
            var audioData = ajaxRequest.response;
            audioCtx.decodeAudioData(audioData, function (buffer) {
                soundSource = audioCtx.createBufferSource();
                Convolver.buffer = buffer;
            }, function (e) { "Error with decoding audio data" + e.err });
        }
        ajaxRequest.send();
    }

    return processedStream;


};

export { fxFunctions };
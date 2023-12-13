const Nexus = require('nexusui');
import { oscSendAndPrint } from './oscUtil.js';

function uiInit(webcamButton, client, pc) {
    Nexus.colors.accent = "#2596be";
    Nexus.colors.fill = "#fff";
    var muteMonitorToggle = new Nexus.Toggle('#muteMo', { 'state': true });
    muteMonitorToggle.on('change', function (v) {
        document.getElementById('muteMonitor').click();
    });
    var muteSelfToggle = new Nexus.Toggle('#muteMy');
    muteSelfToggle.on('change', function (v) {
        document.getElementById('muteMe').click();
    });
    var audioSourceSelect = new Nexus.Select('audioSourceSelect', {
        'size': [200, 20],
        'options': ['Default - MacBook Pro Microphone', 'iPhone']
    });
    audioSourceSelect.colorize("fill", "#eee");
    var audioOutSelect = new Nexus.Select('audioOutputSelect', {
        'size': [200, 20],
        'options': ['Defalut - MacBook Pro Speakers (Built-in)']
    })
    audioOutSelect.colorize("fill", "#eee");
    var startDevices = new Nexus.TextButton('#startDevices', {
        'size': [150, 50],
        'state': false,
        'text': 'Init',
        'alternateText': 'Activated'
    });
    startDevices.colorize("fill", "#eee");
    startDevices.on('change', function (v) {
        webcamButton.click();
    });
    var comp_threshold = new Nexus.Dial('#comp_threshold', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': -100,
        'max': 0,
        'step': 1,
        'value': -24
    });
    var comp_threshold_val = new Nexus.Number('#comp_threshold_val', {
        'size': [30, 20]
    });
    comp_threshold_val.link(comp_threshold);
    var comp_knee = new Nexus.Dial('#comp_knee', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 0,
        'max': 40,
        'step': 1,
        'value': 30
    });
    var comp_knee_val = new Nexus.Number('#comp_knee_val', {
        'size': [30, 20]
    });
    comp_knee_val.link(comp_knee);
    var comp_ratio = new Nexus.Dial('#comp_ratio', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 1,
        'max': 20,
        'step': 1,
        'value': 12
    });
    var comp_ratio = new Nexus.Dial('#comp_ratio', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 1,
        'max': 20,
        'step': 1,
        'value': 12
    });
    var comp_ratio_val = new Nexus.Number('#comp_ratio_val', {
        'size': [30, 20]
    });
    comp_ratio_val.link(comp_ratio);
    var comp_attack = new Nexus.Dial('#comp_attack', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 0,
        'max': 1,
        'step': 0.01,
        'value': 0.003
    });
    var comp_attack_val = new Nexus.Number('#comp_attack_val', {
        'size': [30, 20]
    });
    comp_attack_val.link(comp_attack);
    var comp_release = new Nexus.Dial('#comp_release', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 0,
        'max': 1,
        'step': 0.05,
        'value': 0.25
    });
    var comp_release_val = new Nexus.Number('#comp_release_val', {
        'size': [30, 20]
    });
    comp_release_val.link(comp_release);
    var comp_bypass = new Nexus.Toggle('#comp_bypass');
    var rev_type = new Nexus.Select('rev_type', {
        'size': [200, 20],
        'options': ['Block Inside', 'Bottle Hall', 'Cement Blocks 1', 'Cement Blocks 2', 'Chateau de Logne, Outside', 'Conic Long Echo Hall', 'Deep Space', 'Derlon Sanctuary', 'Direct Cabinet N1', 'Direct Cabinet N2']
    });
    var rev_mix = new Nexus.Dial('#rev_mix', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 0,
        'max': 20,
        'step': 1,
        'value': 10
    });
    var rev_bypass = new Nexus.Toggle('#rev_bypass');
    var eq_bypass = new Nexus.Toggle('#eq_bypass');
    var eq_type = new Nexus.Select('eq_type', {
        'size': [200, 20],
        'options': ['lowshelf', 'highshelf']
    });
    var eq_freq = new Nexus.Slider('eq_freq', {
        'size': [120, 20],
        'mode': 'relative',
        'min': 20,
        'max': 20000,
        'step': 100,
        'value': 1000
    });
    var eq1 = new Nexus.Dial('#eq1', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': -20,
        'max': 40,
        'step': 0,
        'value': 10
    });
    var eq2 = new Nexus.Dial('#eq2', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': -20,
        'max': 40,
        'step': 0,
        'value': 10
    });
    var eq3 = new Nexus.Dial('#eq3', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': -20,
        'max': 40,
        'step': 0,
        'value': 10
    });
    var gain_bypass = new Nexus.Toggle('#gain_bypass');
    var gain = new Nexus.Dial('#gain', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': 0,
        'max': 20,
        'step': 1,
        'value': 2
    });
    var pan_bypass = new Nexus.Toggle('#pan_bypass');
    var pan = new Nexus.Dial('#pan', {
        'size': [50, 50],
        'interaction': 'radial',
        'mode': 'relative',
        'min': -1,
        'max': 1,
        'step': 0.1,
        'value': 0
    });
    var delayTracking = new Nexus.Slider("#delay_tracking");
    delayTracking.min = 0.0;
    delayTracking.max = 1.0;
    delayTracking.step = 0.01;
    delayTracking.value = 0.5;
    delayTracking.on('change', function (v) {
        oscSendAndPrint(client, "rotaryknob", v);
    });
    // for latency detection and compensation
    var delayTracking2 = new Nexus.Button("#delay_tracking2", {
        'size': [50, 50],
        'mode': 'impulse',
        'state': false
    });
    // this only work when the peer connection is established
    delayTracking2.on('change', function (v) {
        var currRTT300 = Math.random() / 1000;
        let currRTT = 0.0;
        pc.getStats().then(reports => reports.forEach(report => {
            if (report.type.includes('remote-inbound-rtp')) {
                console.log("RTT", report.roundTripTime);
                if (report.roundTripTime > 0)
                    currRTT = report.roundTripTime + currRTT;
            }
        }));
        currRTT300 = currRTT300 * 300;
        oscSendAndPrint(client, "rotaryknob", currRTT300);
        // checkTime();
        // console.log("something");
    });

    return {
        muteMonitorToggle, muteSelfToggle, audioSourceSelect, audioOutSelect, startDevices,
        comp_threshold, comp_threshold_val, comp_knee, comp_knee_val,
        comp_ratio, comp_ratio_val, comp_attack, comp_attack_val, comp_release, comp_release_val,
        comp_bypass, rev_type, rev_mix, rev_bypass, eq_bypass, eq_type, eq_freq, eq1, eq2, eq3,
        gain_bypass, gain, pan_bypass, pan,
        delayTracking, delayTracking2
    };
}

export { uiInit };

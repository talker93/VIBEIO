// audioprocessor.js

class RingBuffer {
    constructor(length) {
        this.buffer = new Float32Array(length);
        this.length = length;
        this.readIndex = 0;
        this.writeIndex = 0;
        this.counter_w = 0;
        this.counter_r = 0;
    }

    isFull() {
        return (this.writeIndex + 1) % this.length === this.readIndex;
    }

    isEmpty() {
        return this.readIndex === this.writeIndex;
    }

    push(item) {
        // reset the index and counter when boundary is met
        if (this.writeIndex >= this.length) {
            this.writeIndex = 0;
            this.counter_w = 0;
        }
        this.buffer[this.writeIndex++] = item;
        this.counter_w = this.counter_w + 1;
    }

    pop() {
        // return item when data is available
        if (this.readIndex !== this.writeIndex) {
            // back to 0 when boundary met
            if (this.readIndex >= this.length) {
                this.readIndex = 0;
                this.counter_r = 0;
            }
            const item = this.buffer[this.readIndex++];
            // increase read idx only when available data is read
            this.counter_r = this.counter_r + 1;
            return item;
        }
        // return 0 when data is non-ready
        return 0;
    }

    // return the number of items in the buffer
    size() {
        if (this.writeIndex < this.readIndex) {
            return this.writeIndex + this.length - this.readIndex;
        }
        return this.writeIndex - this.readIndex;
    }
}

class AudioProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        // the udp package is 512 samples length or 2048 in byte
        this.bufferSize = 512;
        this.ringBuffer = new RingBuffer(this.bufferSize * 1000); // Four times the buffer for some leeway
        this.port.onmessage = (event) => {
            const data = event.data;
            // console.log(data);
            console.log("Received ", data.length, " samples");
            for (let i = 0; i < data.length; i++) {
                this.ringBuffer.push(data[i]);
            }
        };
    }

    process(inputs, outputs) {
        const output = outputs[0];
        for (let channel = 0; channel < output.length; channel++) {
            const outputChannel = output[channel];
            console.log("channel: ", channel, "available: ", this.ringBuffer.size());
            for (let i = 0; i < outputChannel.length; i++) {
                outputChannel[i] = this.ringBuffer.pop(); // Default to 0 if buffer is empt
            }
        }
        // console.log("write counter: ", this.ringBuffer.counter_w);
        // console.log("read counter: ", this.ringBuffer.counter_r);
        return true;
    }
}

registerProcessor('audio-processor', AudioProcessor);

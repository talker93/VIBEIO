/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SenderAudioProcessor::SenderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // set up the datagramSocket
    socket.bindToPort(0); // Bind to any available local port
}

SenderAudioProcessor::~SenderAudioProcessor()
{
}

//==============================================================================
const juce::String SenderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SenderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SenderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SenderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SenderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SenderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SenderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SenderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SenderAudioProcessor::getProgramName (int index)
{
    return {};
}

void SenderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SenderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    m_sampleRate = sampleRate;
    m_counter = 0;
    
    angleDelta = (2.0 * juce::MathConstants<double>::pi * frequency) / sampleRate;
}

void SenderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SenderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SenderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // After processing, send audio data to Node.js server
    for (int channel = 0; channel < 1; ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        
        // method 1: check if the absolute zeros exists
//        bool containsOnlyZeros = true;
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            if (channelData[sample] != 0.0f)
//            {
//                containsOnlyZeros = false;
//                break;
//            }
//        }
        
        // method 2: compute the rms
        bool containsOnlyZeros = true;
        float rms = 0.0f;
        float sumOfSquares = 0.0f;
        
        // compute the sum of squares
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            sumOfSquares += channelData[sample] * channelData[sample];
        }
        
        // Compute RMS for this channel
        rms = std::sqrt(sumOfSquares / buffer.getNumSamples());

        if (rms > 0.001)
        {
            containsOnlyZeros = false;
        }
        
        
        if (!containsOnlyZeros)
        {
            sendAudioData(channelData, buffer.getNumSamples());
        }
    }
    // write the the same value for each block 512 samples
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        float* channelData = buffer.getWritePointer(channel);
//
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            // Generate a sine wave sample and write it to the buffer
//            channelData[sample] = m_counter * 0.001;
//        }
//    }
//
//    // For signal intergrity test
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        const float* channelData = buffer.getReadPointer(channel);
//        sendAudioData(channelData, buffer.getNumSamples());
//        for (int sample = 0; sample < juce::jmin(10, buffer.getNumSamples()); ++sample)
//        {
//            DBG("Counter: " << m_counter << " Channel: " << channel << " Sample " << sample << ": " << channelData[sample]);
//        }
//    }
    
    m_counter++;
}

//==============================================================================
bool SenderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SenderAudioProcessor::createEditor()
{
    return new SenderAudioProcessorEditor (*this);
}

//==============================================================================
void SenderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SenderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SenderAudioProcessor();
}

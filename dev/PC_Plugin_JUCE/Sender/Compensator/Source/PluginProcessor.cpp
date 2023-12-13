/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompensatorAudioProcessor::CompensatorAudioProcessor()
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
    m_fLenSec = 1.0;
    m_fBuffLenSec = m_fLenSec*10;
}

CompensatorAudioProcessor::~CompensatorAudioProcessor()
{
}

//==============================================================================
const juce::String CompensatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompensatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompensatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompensatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompensatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompensatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompensatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompensatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompensatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompensatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompensatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_iSampleRate = (int) sampleRate;
    
    // compute the total buffer length and create it
    m_iBuffLenSample = (int) (m_fBuffLenSec * m_iSampleRate);
    m_fDelayBuffer = juce::AudioBuffer<float>(getTotalNumInputChannels(), (int) m_iBuffLenSample);
    if (m_iBuffLenSample < 1)
        m_iBuffLenSample = 1;
    m_fDelayBuffer.clear();
    
    // compute the delay length and set read start position
    m_iLenSample = (int) (m_fLenSec * m_iSampleRate);
    m_iReadIdx = (int) ((m_iWriteIdx - m_iLenSample + m_iBuffLenSample) % m_iBuffLenSample);
    
}

void CompensatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompensatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CompensatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    
    // if delay is too low, deactivate the delay fx
    if (fLenSec >= 0.001)
    {
        const int numInChannels = getTotalNumOutputChannels();
        const int numSamples = buffer.getNumSamples();

//      m_fLenSec = *m_pfDelayTime;
        m_fLenSec = fLenSec;
        m_iLenSample = (int) (m_fLenSec * m_iSampleRate);
        m_iReadIdx = (int) ((m_iWriteIdx - m_iLenSample + m_iBuffLenSample) % m_iBuffLenSample);

        int rIdx, wIdx;

        for (int channel = 0; channel < numInChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float* delayData = m_fDelayBuffer.getWritePointer(channel);

            rIdx = m_iReadIdx;
            wIdx = m_iWriteIdx;
            float inData;
            float outData;

            for (int i = 0; i < numSamples; ++i)
            {
                inData = channelData[i];
                outData = delayData[rIdx];

                delayData[wIdx] = inData;

                rIdx++;
                if (rIdx >= m_iBuffLenSample)
                    rIdx = 0;
                wIdx++;
                if (wIdx >= m_iBuffLenSample)
                    wIdx = 0;

                channelData[i] = outData;
//                outStream.writeFloat(outData);
            }
        }

        m_iReadIdx = rIdx;
        m_iWriteIdx = wIdx;
    }
}

//==============================================================================
bool CompensatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompensatorAudioProcessor::createEditor()
{
    return new CompensatorAudioProcessorEditor (*this);
}

//==============================================================================
void CompensatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CompensatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompensatorAudioProcessor();
}

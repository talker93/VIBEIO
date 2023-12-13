/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CompensatorAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        private juce::Slider::Listener,
                                        private juce::OSCReceiver,
                                        private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    CompensatorAudioProcessorEditor (CompensatorAudioProcessor&);
    ~CompensatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CompensatorAudioProcessor& audioProcessor;
    
    juce::Slider delaySlider;
    juce::Label delayLabel;
    
    void sliderValueChanged (juce::Slider* slider) override;
    
    void oscMessageReceived (const juce::OSCMessage& message) override;
    
    void showConnectionErrorMessage (const juce::String& messageText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompensatorAudioProcessorEditor)
};

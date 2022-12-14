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
class ShanPlugin1101AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ShanPlugin1101AudioProcessorEditor (ShanPlugin1101AudioProcessor&);
    ~ShanPlugin1101AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ShanPlugin1101AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShanPlugin1101AudioProcessorEditor)
};

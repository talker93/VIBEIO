/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SenderAudioProcessorEditor::SenderAudioProcessorEditor (SenderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    labelSampleRate.setText(juce::String(processor.getSampleRate()), juce::dontSendNotification);
    labelChannelNum.setText(juce::String(processor.getTotalNumInputChannels()), juce::dontSendNotification);
}

SenderAudioProcessorEditor::~SenderAudioProcessorEditor()
{
}

//==============================================================================
void SenderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World3!", getLocalBounds(), juce::Justification::centred, 1);
    
    addAndMakeVisible(labelSampleRate);
    labelSampleRate.setBounds(10, 10, 100, 30);
    
    addAndMakeVisible(labelChannelNum);
    labelChannelNum.setBounds(10, 40, 100, 30);
}

void SenderAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

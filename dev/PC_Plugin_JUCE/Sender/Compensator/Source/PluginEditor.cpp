/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompensatorAudioProcessorEditor::CompensatorAudioProcessorEditor (CompensatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(&delaySlider);
    delaySlider.setRange(0.0f, 10.0f, 0.1f);
    delaySlider.setValue(1.0f);
    delaySlider.setTextValueSuffix(" s");
    delaySlider.addListener(this);
    
    addAndMakeVisible(delayLabel);
    delayLabel.setText("Delay Time", juce::dontSendNotification);
    delayLabel.attachToComponent(&delaySlider, true);
    
    if (! connect(9001))
        showConnectionErrorMessage("Error: could not connect to UDP port 9001.");
    
    // add the listener to the OSC port
    addListener(this, "/juce/rotaryknob");
    
    setSize (400, 100);
}

CompensatorAudioProcessorEditor::~CompensatorAudioProcessorEditor()
{
}

//==============================================================================
void CompensatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void CompensatorAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;
    delaySlider.setBounds(sliderLeft, 20, getWidth() - sliderLeft - 10, 20);
}


void CompensatorAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &delaySlider)
    {
        audioProcessor.fLenSec = delaySlider.getValue();
    }
}


void CompensatorAudioProcessorEditor::oscMessageReceived (const juce::OSCMessage& message)
{
    if (message.size() == 1 && message[0].isFloat32())
    {
        // any osc API import ranges from 0 ~ 1
        delaySlider.setValue(juce::jlimit(0.0f, 10.0f, message[0].getFloat32()*10));
    }
}

void CompensatorAudioProcessorEditor::showConnectionErrorMessage (const juce::String& messageText)
{
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Connection error", messageText, "OK");
}

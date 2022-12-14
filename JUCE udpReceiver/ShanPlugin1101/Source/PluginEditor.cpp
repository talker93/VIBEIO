/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ShanPlugin1101AudioProcessorEditor::ShanPlugin1101AudioProcessorEditor (ShanPlugin1101AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

ShanPlugin1101AudioProcessorEditor::~ShanPlugin1101AudioProcessorEditor()
{
}

//==============================================================================
void ShanPlugin1101AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    juce::String flag_connection = audioProcessor.flag_connection ? "true" : "false";
    DBG("flag_connectio: " << flag_connection);
    DBG("msg: " << audioProcessor.msg);
    DBG("msg_num: " << audioProcessor.msg_num);
    DBG("printing is right");
    g.drawFittedText (flag_connection, getLocalBounds(), juce::Justification::centred, 1);
}

void ShanPlugin1101AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

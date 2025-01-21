/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define PLUGIN_WIDTH 800
#define PLUGIN_HEIGHT 600

//==============================================================================
Juce_sandboxAudioProcessorEditor::Juce_sandboxAudioProcessorEditor (Juce_sandboxAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(main_cmp);
    setSize (PLUGIN_WIDTH, PLUGIN_HEIGHT);
}

Juce_sandboxAudioProcessorEditor::~Juce_sandboxAudioProcessorEditor()
{
}

//==============================================================================
void Juce_sandboxAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Juce_sandboxAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    main_cmp.setBounds(3, 3, PLUGIN_WIDTH-3, PLUGIN_HEIGHT-3);
}

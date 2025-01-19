/*
  ==============================================================================

    RedButt.cpp
    Created: 5 Jan 2025 7:58:22am
    Author:  jcbsk

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RedButt.h"

//==============================================================================
RedButt::RedButt()
{
    setButtonText("Click Me!");
    setLookAndFeel(&lnf);
}

RedButt::~RedButt()
{
    setLookAndFeel(nullptr); // Reset to avoid dangling pointer
}

void RedButt::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    getLookAndFeel().drawButtonBackground(g, *this, findColour(juce::TextButton::buttonColourId),
                                          isMouseOver(), isMouseButtonDown());


    // g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("RedButt", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void RedButt::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

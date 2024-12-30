/*
  ==============================================================================

    MainCmp.cpp
    Created: 28 Dec 2024 8:39:02pm
    Author:  jcbsk

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainCmp.h"

//==============================================================================
MainCmp::MainCmp()
{
    addAndMakeVisible(myButton);
    myButton.setButtonText("Click Me!");
    myButton.addListener(this); // Add listener
    // NOTE: Don't use setSize() for child components, their size is always managed by parents in their resized() fct
}

MainCmp::~MainCmp()
{
    myButton.removeListener(this);
}

void MainCmp::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::pink);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("MainCmp", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void MainCmp::resized()
{
    constexpr int buttonWidth = 200;
    constexpr int buttonHeight = 50;

    int x = (getWidth() - buttonWidth) / 2;             // Center horizontally
    int y = (getHeight() * 3) / 4 - buttonHeight / 2;   // 3/4 vertically
    myButton.setBounds(x, y, buttonWidth, buttonHeight);
}

// Button::Listener overrides
void MainCmp::buttonClicked(juce::Button* button)
{
    if (button == &myButton)
    {
        // FIXME: this overrides the Lookandfeel hover effects and such, use my own lookandfeel instead
        button->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    }
}

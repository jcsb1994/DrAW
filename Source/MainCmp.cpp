/*
  ==============================================================================

    MainCmp.cpp
    Created: 28 Dec 2024 8:39:02pm
    Author:  jcbsk

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainCmp.h"

#include <windows.h> // For Windows API (console manipulation)


//==============================================================================
MainCmp::MainCmp()
{
    addAndMakeVisible(redButton);
    redButton.addListener(this); // Add listener
    // NOTE: Don't use setSize() for child components, their size is always managed by parents in their resized() fct


    if (AllocConsole()) // Open a console window
    {
        freopen("CONOUT$", "w", stdout); // Redirect stdout to the console
        freopen("CONOUT$", "w", stderr); // Redirect stderr to the console
    }
}

MainCmp::~MainCmp()
{
    redButton.removeListener(this);

    FreeConsole(); // Close the console when the application quits
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
    redButton.setBounds(x, y, buttonWidth, buttonHeight);
}

// Button::Listener overrides
void MainCmp::buttonClicked(juce::Button* button)
{
    if (button == &redButton)
    {
        std::cout << "red butt" << std::endl;
    }
}

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
    addAndMakeVisible(graph);

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
    /* Setting the bounds of our subcomponents. At first, we may put harcoded values, but it is best
        to pass fractions of the main component's size */
    auto bounds = getLocalBounds();

    /* Interactive graph - placed at the top half of the GUI */
    auto graph_rectangle = bounds.removeFromTop(bounds.getHeight() / 2);
    graph.setBounds(graph_rectangle); // passes X,Y,W,H

    /* Dummy button placed at center of bottom half */
    int redButt_w = bounds.getWidth() * 0.2f;
    int redButt_h = bounds.getHeight() * 0.1f;
    auto redButt_rect = bounds.withSizeKeepingCentre(redButt_w, redButt_h);
    redButton.setBounds(redButt_rect);

}

// Button::Listener overrides
void MainCmp::buttonClicked(juce::Button* button)
{
    if (button == &redButton)
    {
        std::cout << "red butt" << std::endl;
    }
}

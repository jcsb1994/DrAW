/*
  ==============================================================================

    MainCmp.h
    Created: 28 Dec 2024 8:39:02pm
    Author:  jcbsk

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
// user
#include "buttons/RedButt.h"
#include "graph/graphCmp.h"

//==============================================================================
/*
*/
class MainCmp  : public juce::Component, juce::Button::Listener
{
public:
    MainCmp();
    ~MainCmp() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Button::Listener overrides
    void buttonClicked(juce::Button* button) override;

private:

    // Child components
    RedButt redButton;
    FrequencyGraph graph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainCmp)
};

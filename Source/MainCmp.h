/*
  ==============================================================================

    MainCmp.h
    Created: 28 Dec 2024 8:39:02pm
    Author:  jcbsk

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class MainCmp  : public juce::Component
{
public:
    MainCmp();
    ~MainCmp() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainCmp)
};

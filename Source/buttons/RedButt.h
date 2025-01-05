/*
  ==============================================================================

    RedButt.h
    Created: 5 Jan 2025 7:58:22am
    Author:  jcbsk

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class RedButt  : public juce::Component
{
public:
    RedButt();
    ~RedButt() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RedButt)
};

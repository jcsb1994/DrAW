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

class RedButtLnF : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Define colors based on the button state
        juce::Colour fillColour;
        if (isButtonDown)
            fillColour = juce::Colours::red; // Red when pressed
        else if (isMouseOverButton)
            fillColour = juce::Colours::lightcoral; // Light red when hovered
        else
            fillColour = backgroundColour; // Default color

        // Fill the button background
        g.setColour(fillColour);
        g.fillRoundedRectangle(bounds, 6.0f); // Rounded corners

        // Optional: Draw an outline
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        std::cout << ".";
    }
};



class RedButt  : public juce::TextButton
{
public:
    RedButt();
    ~RedButt() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    RedButtLnF lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RedButt)
};

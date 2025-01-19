/* Component that manages a graph with an interactive line */

#include <JuceHeader.h>

class FrequencyGraph : public juce::Component
{
public:
    FrequencyGraph()
    {
        // Initialize dots: frequency (Hz), amplitude (dB)
        dots = { {100.0f, 0.0f}, {1000.0f, 0.0f}, {10000.0f, 0.0f} };
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto graphBounds = bounds.reduced(40);

        // Draw background
        g.fillAll(juce::Colours::black);

        // Draw axes
        g.setColour(juce::Colours::white);
        g.drawRect(graphBounds);

        // Draw X-axis (logarithmic frequency scale)
        for (float freq = 100.0f; freq <= 10000.0f; freq *= 10.0f)
        {
            float x = frequencyToX(freq, graphBounds);
            g.drawVerticalLine((int)x, graphBounds.getY(), graphBounds.getBottom());
            g.drawText(juce::String(freq, 0) + " Hz", (int)x - 20, graphBounds.getBottom() + 5, 40, 20, juce::Justification::centred);
        }

        // Draw Y-axis (linear amplitude scale)
        for (int dB = -24; dB <= 24; dB += 6)
        {
            float y = amplitudeToY((float)dB, graphBounds);
            g.drawHorizontalLine((int)y, graphBounds.getX(), graphBounds.getRight());
            g.drawText(juce::String(dB) + " dB", graphBounds.getX() - 35, (int)y - 10, 30, 20, juce::Justification::centredRight);
        }

        // Draw line and dots
        g.setColour(juce::Colours::cyan);
        for (size_t i = 0; i < dots.size(); ++i)
        {
            float x = frequencyToX(dots[i].first, graphBounds);
            float y = amplitudeToY(dots[i].second, graphBounds);

            if (i > 0)
            {
                float prevX = frequencyToX(dots[i - 1].first, graphBounds);
                float prevY = amplitudeToY(dots[i - 1].second, graphBounds);
                g.drawLine(prevX, prevY, x, y, 2.0f);
            }

            g.fillEllipse(x - 5, y - 5, 10, 10); // Draw dot
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        // Check if the mouse clicked near a dot
        auto graphBounds = getLocalBounds().reduced(40);

        for (size_t i = 0; i < dots.size(); ++i)
        {
            float x = frequencyToX(dots[i].first, graphBounds);
            float y = amplitudeToY(dots[i].second, graphBounds);

            if (juce::Rectangle<float>(x - 5, y - 5, 10, 10).contains(event.position))
            {
                draggedDotIndex = (int)i;
                return;
            }
        }
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggedDotIndex >= 0)
        {
            auto graphBounds = getLocalBounds().reduced(40);

            // Convert mouse position to frequency and amplitude
            float freq = xToFrequency(event.position.x, graphBounds);
            float amp = yToAmplitude(event.position.y, graphBounds);

            // Clamp values to valid ranges
            freq = juce::jlimit(100.0f, 10000.0f, freq);
            amp = juce::jlimit(-24.0f, 24.0f, amp);

            dots[draggedDotIndex] = { freq, amp };
            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        draggedDotIndex = -1; // Reset dragged dot
    }

private:
    std::vector<std::pair<float, float>> dots; // Dots: frequency (Hz), amplitude (dB)
    int draggedDotIndex = -1;

    // Map frequency (log scale) to X position
    float frequencyToX(float freq, juce::Rectangle<int> bounds) const
    {
        return bounds.getX() + bounds.getWidth() * std::log10(freq / 100.0f) / std::log10(10000.0f / 100.0f);
    }

    // Map amplitude to Y position
    float amplitudeToY(float amp, juce::Rectangle<int> bounds) const
    {
        return bounds.getBottom() - (amp + 24.0f) * bounds.getHeight() / 48.0f;
    }

    // Map X position to frequency
    float xToFrequency(float x, juce::Rectangle<int> bounds) const
    {
        return 100.0f * std::pow(10.0f, (x - bounds.getX()) / bounds.getWidth() * std::log10(10000.0f / 100.0f));
    }

    // Map Y position to amplitude
    float yToAmplitude(float y, juce::Rectangle<int> bounds) const
    {
        return 24.0f - (y - bounds.getY()) * 48.0f / bounds.getHeight();
    }
};

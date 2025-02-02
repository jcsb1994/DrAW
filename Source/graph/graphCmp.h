/* Component that manages a graph with an interactive line */

#include <JuceHeader.h>
#include <algorithm>

class FrequencyGraph : public juce::Component
{
public:
    FrequencyGraph()
    {
        // Initialize dots: frequency (Hz), amplitude (dB)

        dots = {
            {0.0f, 0.0f},       // (0 Hz, 0 dB)
            {20000.0f, 0.0f}    // (20000 Hz, 0 dB)
        };
    }

    void resized() override
    {
        // Recreate static graph image when the component is resized
        createStaticGraph();
    }

    void paint(juce::Graphics& g) override
    {

        // Draw the cached static graph

        g.drawImageAt(staticGraph, 0, 0);

        // Draw dynamic elements (dots and lines)

        g.setColour(juce::Colours::cyan);
        auto graphBounds = getGraphBounds();
        std::cout << dots.size();
        for (size_t i = 0; i < dots.size(); i++)
        {
            float x = frequencyToX(dots[i].first, graphBounds);
            float y = amplitudeToY(dots[i].second, graphBounds);

            if (i > 0) {
                float prevX = frequencyToX(dots[i - 1].first, graphBounds);
                float prevY = amplitudeToY(dots[i - 1].second, graphBounds);
                g.drawLine(prevX, prevY, x, y, 2.0f);
            }

            g.fillEllipse(x - 5, y - 5, 10, 10); // Draw dot
        }
    }


    void createStaticGraph()
    {
        auto bounds = getLocalBounds();
        auto graphBounds = getGraphBounds();

        staticGraph = juce::Image(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);
        juce::Graphics g(staticGraph);

        // Draw background
        g.fillAll(juce::Colours::black);

        // Draw axes
        g.setColour(juce::Colours::white);
        g.drawRect(graphBounds);

        // Draw X-axis (logarithmic frequency scale)
        for (float freq = 100.0f; freq <= 10000.0f; freq *= 10.0f)
        {
            float x = frequencyToX(freq, graphBounds);

            // Draw main vertical line
            g.drawVerticalLine((int)x, graphBounds.getY(), graphBounds.getBottom());
            g.drawText(juce::String(freq, 0) + " Hz", (int)x - 20, graphBounds.getBottom() + 5, 40, 20, juce::Justification::centred);

            // Add dimmer vertical lines for intermediate frequencies
            g.setColour(juce::Colours::grey.withAlpha(0.6f)); // Dimmer lines
            for (int i = 2; i < 10; ++i)
            {
                float subFreq = freq * i;
                if (subFreq > 20000.0f) break; // Prevent exceeding 20 kHz

                float subX = frequencyToX(subFreq, graphBounds);
                g.drawVerticalLine((int)subX, graphBounds.getY(), graphBounds.getBottom());
            }

            // Reset colour for main lines
            g.setColour(juce::Colours::white);
        }


        // Draw Y-axis (linear amplitude scale)
        for (int dB = -24; dB <= 24; dB += 6)
        {
            float y = amplitudeToY((float)dB, graphBounds);
            g.drawHorizontalLine((int)y, graphBounds.getX(), graphBounds.getRight());
            g.drawText(juce::String(dB) + " dB", graphBounds.getX() - 35, (int)y - 10, 30, 20, juce::Justification::centredRight);
        }
    }

    juce::Rectangle<int> getGraphBounds() const
    {
        return getLocalBounds().reduced(40); // Trims the component bounds on all sides, très utile pour encadrés
    }


    void mouseDown(const juce::MouseEvent& event) override
    {
        auto graphBounds = getGraphBounds();
        float mouseX = event.position.x;
        float mouseY = event.position.y;

        std::cout << "Click: " << mouseX << ", " << mouseY << "\n";

        // Check if we clicked on an existing dot
        int clickedDotIndex = getClickedDotIndex(mouseX, mouseY, graphBounds);
        if (clickedDotIndex != -1)
        {
            // Start dragging this dot
            draggedDotIndex = clickedDotIndex;
            return;
        }

        // Otherwise, split the closest line
        float freq = xToFrequency(mouseX, graphBounds);
        float amp = yToAmplitude(mouseY, graphBounds);


        // Find the position to insert based on the x value
        auto it = std::lower_bound(dots.begin(), dots.end(), freq,
            [](const std::pair<int, int>& a, int value) {
                return a.first < value; // Compare only the x (first) value
            });
        // Calculate index
        size_t index = std::distance(dots.begin(), it);


        // size_t closestIndex = findClosestLineSegment(freq, amp, graphBounds);
        dots.insert(dots.begin() + index + 1, { freq, amp });
        std::cout << "at " << (index + 1);

        repaint();
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (draggedDotIndex >= 0)
        {
            auto graphBounds = getGraphBounds();

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

    const std::pair<float, float> graph_f_bounds{ 100.0f, 20000.0f };

    std::vector<std::pair<float, float>> dots; // Dots: frequency (Hz), amplitude (dB)
    int draggedDotIndex = -1;

    juce::Image staticGraph;

    // Map frequency (log scale) to X position
    float frequencyToX(float freq, juce::Rectangle<int> bounds) const
    {
        float log_ratio = std::log10(graph_f_bounds.second / graph_f_bounds.first);
        return bounds.getX() + bounds.getWidth() * std::log10(freq / graph_f_bounds.first) / log_ratio;
    }

    // Map amplitude to Y position
    float amplitudeToY(float amp, juce::Rectangle<int> bounds) const
    {
        return bounds.getBottom() - (amp + 24.0f) * bounds.getHeight() / 48.0f;
    }

    // Map X position to frequency
    float xToFrequency(float x, juce::Rectangle<int> bounds) const
    {
        float log_ratio = std::log10(graph_f_bounds.second / graph_f_bounds.first);
        float x_offset = x - bounds.getX();
        float result =  graph_f_bounds.first * std::pow(10.0f, x_offset / bounds.getWidth() * log_ratio);
        std:: cout << "X @ offset " << x_offset << " to frquency:\n\tLog10(20k/100)=" << log_ratio << "\n\tFinal freq=" << result;
        return result;
    }

    // Map Y position to amplitude
    float yToAmplitude(float y, juce::Rectangle<int> bounds) const
    {
        return 24.0f - (y - bounds.getY()) * 48.0f / bounds.getHeight();
    }

    void debug_dot(uint8_t index, float x, float y, float freq) const
    {
        std::cout << "dot " << index << ": " <<  "(" << x << ", " << y << ")" << " " << freq << "Hz\n";
    }

    int getClickedDotIndex(float mouseX, float mouseY, const juce::Rectangle<int>& bounds) const
    {
        for (size_t i = 0; i < dots.size(); ++i)
        {
            float x = frequencyToX(dots[i].first, bounds);
            float y = amplitudeToY(dots[i].second, bounds);
            debug_dot(i, x, y, dots[i].first);
            // Check if the mouse click is within the dot's radius
            if (std::hypot(mouseX - x, mouseY - y) <= 5.0f)
                return static_cast<int>(i);
        }
        return -1; // No dot clicked
    }

    size_t findClosestLineSegment(float freq, float amp, const juce::Rectangle<int>& bounds) const
    {
        size_t closestIndex = 0;
        float closestDistance = std::numeric_limits<float>::max();

        for (size_t i = 0; i < dots.size() - 1; ++i)
        {
            auto x1 = frequencyToX(dots[i].first, bounds);
            auto y1 = amplitudeToY(dots[i].second, bounds);
            auto x2 = frequencyToX(dots[i + 1].first, bounds);
            auto y2 = amplitudeToY(dots[i + 1].second, bounds);

            float distance = pointToLineSegmentDistance({ x1, y1 }, { x2, y2 }, { freq, amp });
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestIndex = i;
            }
        }

        return closestIndex;
    }

    float pointToLineSegmentDistance(juce::Point<float> p1, juce::Point<float> p2, juce::Point<float> p) const
    {
        auto d = p2 - p1;
        float lenSquared = d.x * d.x + d.y * d.y; // Squared length of the line segment

        if (lenSquared == 0.0f)
            return p.getDistanceFrom(p1); // If the line segment is a point, return distance to p1

        // Project the point onto the line segment and clamp to the segment
        auto t = ((p.x - p1.x) * d.x + (p.y - p1.y) * d.y) / lenSquared;
        t = juce::jlimit(0.0f, 1.0f, t);

        // Compute the projection point
        auto projection = p1 + d * t;

        // Return the distance from the point to the projection
        return p.getDistanceFrom(projection);
    }

};

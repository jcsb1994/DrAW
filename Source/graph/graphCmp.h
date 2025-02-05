/* Component that manages a graph with an interactive line */

#include <JuceHeader.h>
#include <algorithm>

class FrequencyGraph : public juce::Component
{
public:
    FrequencyGraph()
    {
        // Initialize _dots: frequency (Hz), amplitude (dB)

        _dots = {
            {0.0f, 0.0f},       // (0 Hz, 0 dB)
            {20000.0f, 0.0f}    // (20000 Hz, 0 dB)
        };
    }

    void resized() override;
    void paint(juce::Graphics& g) override;

    void createStaticGraph();

    juce::Rectangle<int> getGraphBounds() const;

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseDrag(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent&) override;

private:

    // Drawing
    const std::pair<float, float> _freq_bounds{ 100.0f, 20000.0f }; // TODO: 0 - 20k
    const std::pair<float, float> _plot_x_bounds{ 0.05f, 100.0f }; // Leave 5% for Y axis
    const float _log_ratio = std::log10(_freq_bounds.second / _freq_bounds.first); // ~2.3
    juce::Image _staticGraph;

    std::vector<std::pair<float, float>> _dots; // Dots: frequency (Hz), amplitude (dB)
    int _dragged_dot_idx = -1;


    // Map frequency (log scale) to X position
    float frequencyToX(float freq, juce::Rectangle<int> bounds) const
    {
        if (freq == 0) {
            return 0; // log10(0) is invalid
        }
        return bounds.getX() + bounds.getWidth() * std::log10(freq / _freq_bounds.first) / _log_ratio;
    }

    // Map amplitude to Y position
    float amplitudeToY(float amp, juce::Rectangle<int> bounds) const
    {
        return bounds.getBottom() - (amp + 24.0f) * bounds.getHeight() / 48.0f;
    }

    // Map X position to frequency
    float xToFrequency(float x, juce::Rectangle<int> bounds) const
    {
        float x_offset = x - bounds.getX();
        float result =  _freq_bounds.first * std::pow(10.0f, x_offset / bounds.getWidth() * _log_ratio);
        std::cout << "X @ offset " << x_offset << " to frquency:\n\tLog10(20k/100)=" << _log_ratio << "\n\tFinal freq=" << result << "\n";
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
        for (size_t i = 0; i < _dots.size(); ++i)
        {
            float x = frequencyToX(_dots[i].first, bounds);
            float y = amplitudeToY(_dots[i].second, bounds);
            debug_dot(i, x, y, _dots[i].first);
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

        for (size_t i = 0; i < _dots.size() - 1; ++i)
        {
            auto x1 = frequencyToX(_dots[i].first, bounds);
            auto y1 = amplitudeToY(_dots[i].second, bounds);
            auto x2 = frequencyToX(_dots[i + 1].first, bounds);
            auto y2 = amplitudeToY(_dots[i + 1].second, bounds);

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

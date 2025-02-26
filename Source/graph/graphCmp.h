/* Component that manages a graph with an interactive line */

#include <JuceHeader.h>
#include <algorithm>

#define debug(title, val) std::cout << title << val << "\n"





class DotLnF : public juce::LookAndFeel_V4 {
    static constexpr int dotRadius = 5;
public:
    void drawDot(juce::Graphics& g, float x, float y, bool isSelected) {
        g.setColour(isSelected ? juce::Colours::blue : juce::Colours::cyan);
        g.fillEllipse(x - dotRadius, y - dotRadius, (dotRadius*2), (dotRadius*2));
        g.setColour(juce::Colours::black);
        g.drawEllipse(x - dotRadius, y - dotRadius, (dotRadius*2), (dotRadius*2), 2);
    }
    void drawLineCenter(juce::Graphics& g, float x, float y, bool isSelected) {
        g.setColour(juce::Colours::black);
        g.fillEllipse(x - dotRadius, y - dotRadius, (dotRadius*2), (dotRadius*2));
        g.setColour(isSelected ? juce::Colours::blue : juce::Colours::cyan);
        g.drawEllipse(x - dotRadius, y - dotRadius, (dotRadius*2), (dotRadius*2), 2);
    }
    void drawLine(juce::Graphics& g, juce::Path& path) {
        g.setColour(juce::Colours::cyan);
        g.strokePath(path, juce::PathStrokeType(3.0f));
    }

};



struct FreqDot
{
    juce::Point<float> pt;
    // path and mode
    enum class lineMode { bezier, stair, sine };
    lineMode linemode;
    /*! \note
    bezier: sets ctrl point
    stair,sine: nb of cycles
    TODO: add square */
    float control;

    FreqDot(float freq, float amp) : pt(freq, amp), linemode(lineMode::bezier) { control = 0; }
};







struct CurvedLine {
    juce::Point<float> center;  // Midpoint of the line
    juce::Point<float> control; // Control point for bending the curve

    CurvedLine(juce::Point<float> start, juce::Point<float> end)
        : center((start + end) / 2.0f), control((start + end) / 2.0f) {
        } // Start with straight line

    // Quadratic Bézier curve formula
    juce::Point<float> getPointOnCurve(float t, juce::Point<float> start, juce::Point<float> end) const {
        float x = (1 - t) * (1 - t) * start.x + 2 * (1 - t) * t * control.x + t * t * end.x;
        float y = (1 - t) * (1 - t) * start.y + 2 * (1 - t) * t * control.y + t * t * end.y;
        return {x, y};
    }

    void updateCenter()
    {
        // Must drag line center along with dot so curve ratio doesnt change when sliding dots
    }

};


class FrequencyGraph : public juce::Component
{
public:
    FrequencyGraph()
    {
        // Initialize _dots: frequency (Hz), amplitude (dB)


        addDot(_freq_bounds.first, 0.0f);
        addDot(_freq_bounds.second, 0.0f);

        // Must wait for resizing to paint, XY is unknown
    }

    // Display
    void resized() override;
    void paint(juce::Graphics& g) override;
    void createStaticGraph();

    // Clickable graph region
    juce::Rectangle<int>    getGraphBounds() const;
    bool                    isWithinGraphBounds(float x, float y) const;
    bool                    isWithinGraphBounds(float x, float y, juce::Rectangle<int> graphBounds) const;


    // Click
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    enum class graphElement { dot, line };
    // Drawing
    const std::pair<float, float> _freq_bounds{ 10.0f, 20000.0f };
    const std::pair<float, float> _amp_bounds{ -24.0f, 24.0f }; //TODO: check if should be - to 0
    const float _amp_range = _amp_bounds.second - _amp_bounds.first;
    const float _log_ratio = std::log10(_freq_bounds.second / _freq_bounds.first); // ~2.3
    juce::Image _staticGraph;

    std::vector<FreqDot> _dots2;
    DotLnF _lnf; // Manages visuals for the dots set
    // std::vector<int> _selected_idxs; // manages selecting more t`
    std::pair<std::vector<int>, graphElement> _clicked_items{{}, graphElement::dot};


    // Map frequency (log scale) to X position
    float frequencyToX(float freq, juce::Rectangle<int> bounds) const
    {
        if (freq == 0) {
            return 0; // log10(0) is invalid
        }
        float logResult = std::log10(freq / _freq_bounds.first);
        int w = bounds.getWidth();
        int x = bounds.getX();
        return x + w * logResult / _log_ratio;
    }

    // Map amplitude to Y position
    float amplitudeToY(float amp, juce::Rectangle<int> bounds) const
    {
        return bounds.getBottom() - (amp + _amp_bounds.second) * bounds.getHeight() / _amp_range;
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
        return _amp_bounds.second - (y - bounds.getY()) * _amp_range / bounds.getHeight();
    }

    void debug_dot(uint8_t index, float x, float y, float freq) const
    {
        std::cout << "dot " << index << ": " <<  "(" << x << ", " << y << ")" << " " << freq << "Hz\n";
    }

    void debug_curves()
    {
        for (int i = 0; i < _dots2.size(); i++) {
            std::cout << "line " << i << " ctrl/center:\n";
        }

    }

    inline bool isDotClicked(float mouseX, float mouseY, float ptX, float ptY)
    {
        // Check if the mouse click is within the dot's radius
        return std::hypot(mouseX - ptX, mouseY - ptY) <= 5.0f;
    }

    bool getLineCenterPoint(int dotIdx, const juce::Rectangle<int>& bounds, juce::Point<float>& linePt)
    {
        if (dotIdx == 0 || dotIdx >= _dots2.size()) {
            return false;
        }

        float lastX = frequencyToX(_dots2[dotIdx-1].pt.x, bounds);
        float lastY = amplitudeToY(_dots2[dotIdx-1].pt.y, bounds);
        float x = frequencyToX(_dots2[dotIdx].pt.x, bounds);
        float y = amplitudeToY(_dots2[dotIdx].pt.y, bounds);
        linePt.x = (x + lastX) / 2;
        linePt.y = (y + lastY) / 2;
        return true;
    }

    std::pair<int, graphElement> getClickedItem(float mouseX, float mouseY, const juce::Rectangle<int>& bounds)
    {
        for (size_t i = 0; i < _dots2.size(); ++i)
        {
            float x = frequencyToX(_dots2[i].pt.x, bounds);
            float y = amplitudeToY(_dots2[i].pt.y, bounds);

            // Check if we are clicking a dot
            if (isDotClicked(mouseX, mouseY, x, y)) {
                return std::pair<int, graphElement>(i, graphElement::dot);
            }

            // Check if we are clicking a center of a line
            if (i > 0) {
                float lastX = frequencyToX(_dots2[i-1].pt.x, bounds);
                float lastY = amplitudeToY(_dots2[i-1].pt.y, bounds);
                float lineCenterX = (x + lastX) / 2;
                float lineCenterY = (y + lastY) / 2;

                if (isDotClicked(mouseX, mouseY, lineCenterX, lineCenterY)) {
                    return std::pair<int, graphElement>(i, graphElement::line);
                }
            }
        }

        // std::pair<int, graphElement> rc{-1, graphElement::dot};
        return std::pair<int, graphElement>{-1, graphElement::dot};
    }

    int getClickedDotIndex(float mouseX, float mouseY, const juce::Rectangle<int>& bounds) const
    {
        for (size_t i = 0; i < _dots2.size(); ++i)
        {
            float x = frequencyToX(_dots2[i].pt.x, bounds);
            float y = amplitudeToY(_dots2[i].pt.y, bounds);
            // debug_dot(i, x, y, _dots[i].first);
            // Check if the mouse click is within the dot's radius
            if (std::hypot(mouseX - x, mouseY - y) <= 5.0f)
                return static_cast<int>(i);
        }
        return -1; // No dot clicked
    }


    void addDot(float x, float y)
    {
        juce::Point<float> point(x, y);
        addDot(point);
    }

    void addDot(juce::Point<float>& point)
    {
        // Find the correct position in _dots to maintain sorted order by X
        auto it = std::lower_bound(_dots2.begin(), _dots2.end(), point,
            [](const FreqDot& dot, const juce::Point<float>& value) {
                return dot.pt.x < value.x;
            });

        // Insert new FreqDot at the found position
        _dots2.insert(it, FreqDot(point.x, point.y));
        printDot("New", point);
    }

    /*! \brief Move dot at specified idx to new freq/amp coordinates */
    void moveDot(int idx, float freq, float amp)
    {
        amp = juce::jlimit(_amp_bounds.first, _amp_bounds.second, amp);
        // TODO: CLEANUP
        if (_dots2.size() > 2) {
            // Clamp freq to valid ranges (Graph bounds, or adjacent dots)
            float leftBound = (idx > 0) ? _dots2[idx-1].pt.x : _freq_bounds.first;
            float rightBound = (idx < _dots2.size() - 1) ? _dots2[idx+1].pt.x : _freq_bounds.second;
            if (idx == 0) {
                rightBound = _freq_bounds.first;
            } else if (idx == _dots2.size() - 1) {
                leftBound = _freq_bounds.second;
            }
            freq = juce::jlimit(leftBound, rightBound, freq);
            _dots2[idx].pt.setXY(freq, amp);

        } else { // When only 2 dots, freq cannot change
            _dots2[idx].pt.setY(amp);
        }
    }

    void applyLineCtrl(int idx, int mouseX, int mouseY, const juce::Rectangle<int>& bounds)
    {
        juce::Point<float> linePt(0, 0);
        juce::Point<float> mousePos(mouseX, mouseY);
        getLineCenterPoint(idx, bounds, linePt);

        juce::Point<float> distanceVector = mousePos - linePt;
        // float distance = distanceVector.getDistanceFromOrigin();

        _dots2[idx].control = distanceVector.y; // Do not use absolute value, can be negative!
        std::cout << "ctrl:" << _dots2[idx].control << "\n";
    }

    void printDot(juce::String name, juce::Point<float> point)
    {
        std::cout << name << " dot: (" << point.x << "," << point.y << ")\n";
    }
    void printDot(juce::String name, float x, float y)
    {
        std::cout << name << " dot: (" << x << "," << y << ")\n";
    }


};

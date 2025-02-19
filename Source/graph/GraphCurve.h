#include <JuceHeader.h>



class DotLnF : public juce::LookAndFeel_V4 {
public:
    void drawDot(juce::Graphics& g, juce::Point<float> position, bool isSelected) {
        g.setColour(isSelected ? juce::Colours::red : juce::Colours::white);
        g.fillEllipse(position.x - 5, position.y - 5, 10, 10);
        g.setColour(juce::Colours::black);
        g.drawEllipse(position.x - 5, position.y - 5, 10, 10, 2);
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

    FreqDot(float freq, float amp) : pt(freq, amp), linemode(lineMode::bezier) {}
};




class FreqBoundMap
{
private:
    juce::Rectangle<float> _xyBounds; // Bound to change @ resize

    // Note: These could also have been a rectangle
    const std::pair<float, float> _freqBounds;
    const std::pair<float, float> _ampBounds;
    const float _ampWidth;
    const float _freqLogRange; // logarithmic range of frequencies
public:
    FreqBoundMap(const std::pair<float, float> freqBounds, const std::pair<float, float> ampBounds) :
        _ampBounds(ampBounds), _freqBounds(freqBounds),
        _ampWidth(ampBounds.second - ampBounds.first),
        _freqLogRange(std::log10(_freqBounds.second / _freqBounds.first))
    {
        std::cout << "Bound map: "  << minAmplitude() << maxAmplitude() << minFrequency() << maxFrequency() << minY() << maxY() << minX() << maxX();
    }

    inline float minAmplitude() const { return _ampBounds.first; }
    inline float maxAmplitude() const { return _ampBounds.second; }
    inline float minFrequency() const { return _freqBounds.first; }
    inline float maxFrequency() const { return _freqBounds.second; }

    inline float minY() const { return _xyBounds.getY() - _xyBounds.getHeight(); }
    inline float maxY() const { return _xyBounds.getY(); }
    inline float minX() const { return _xyBounds.getX(); }
    inline float maxX() const { return _xyBounds.getX() + _xyBounds.getWidth(); }



    // Map frequency (log scale) to X position
    float frequencyToX(float freq, juce::Rectangle<int> bounds) const
    {
        if (freq == 0) {
            return 0; // log10(0) is invalid
        }
        float logResult = std::log10(freq / minFrequency());
        int w = bounds.getWidth();
        int x = bounds.getX();
        return x + w * logResult / _freqLogRange;
    }

    // Map amplitude to Y position
    float amplitudeToY(float amp, juce::Rectangle<int> bounds) const
    {
        return bounds.getBottom() - (amp + maxAmplitude()) * bounds.getHeight() / _ampWidth;
    }

    // Map X position to frequency
    float xToFrequency(float x, juce::Rectangle<int> bounds) const
    {
        float x_offset = x - bounds.getX();
        float result =  minFrequency() * std::pow(10.0f, x_offset / bounds.getWidth() * _freqLogRange);
        std::cout << "X @ offset " << x_offset << " to frquency:\n\tLog10(20k/100)=" << _freqLogRange << "\n\tFinal freq=" << result << "\n";
        return result;
    }

    // Map Y position to amplitude
    float yToAmplitude(float y, juce::Rectangle<int> bounds) const
    {
        return maxAmplitude() - (y - bounds.getY()) * _ampWidth / bounds.getHeight();
    }

};







/*! \brief Doesnt know about X/Y/freq/amp conversions, about freq and amp max */
class FreqCurve
{
public:
    FreqCurve(FreqBoundMap& boundMap) : _boundMap(boundMap) {}
    ~FreqCurve() {}

    void drawCurve(juce::Graphics& g)
    {
        auto selected_it = _selected_idxs.begin();
        auto selected_end = _selected_idxs.end();
        juce::Path path;

        for (int i = 0; i < _dots.size(); i++) {
            bool selected = (selected_it != selected_end && i == *selected_it);
            _lnf.drawDot(g, _dots[i].pt, selected);

            if (selected) {
                selected_it++; // Move to the next selected index
            }

            if (i > 0) {
                path.startNewSubPath(_dots[i-1].pt);
                float ctrlX = _dots[i].pt.x - _dots[i-1].pt.x;
                float ctrlY = (_dots[i].pt.y - _dots[i-1].pt.y) + _dots[i].control;
                juce::Point<float> ctrlPoint(ctrlX, ctrlY);
                // FIXME: control doit etre perpendiculaire a slope, pas vertical

                path.quadraticTo(ctrlPoint, _dots[i].pt); // TODO: change fct depending linemode
            }
        }
    }


    /*! \brief Return a const ref to the curve points (read-only) */
    const std::vector<FreqDot>& getDots() const { return _dots; }


    void addDot(juce::Point<float> point)
    {
        // Find the correct position in _dots to maintain sorted order by X
        auto it = std::lower_bound(_dots.begin(), _dots.end(), point,
            [](const FreqDot& dot, const juce::Point<float>& value) {
                return dot.pt.x < value.x;
            });

        // Insert new FreqDot at the found position
        _dots.insert(it, FreqDot(point.x, point.y));
        std::cout << "Added dot (" << point.x << "," << point.y << ") to curve\n";
    }

    // bool selectPoint(float selectedFreq, float selectedAmp)
    // {
    //     for (size_t i = 0; i < _dots.size(); ++i) {

    //         if (std::hypot(selectedFreq - _dots[i].pt.x, selectedAmp - _dots[i].pt.y) <= 5.0f) {
    //             return static_cast<int>(i);
    //         }
    //     }
    //     return -1; // No dot clicked
    // }
    // 2 functions for selection, since we know X increases for each dot,
    // when dragging, we can check 1st non selected dot to see if Y is in box
    // in caller: when ctrl is down and dragging, make rect with origin of drag and call this
    void selectBox(juce::Rectangle<float> selectionBox)
    {
        // One day optimize with keeping already selected until selection is released
        // Means checking which direction the rect increased

        for (int i = 0; i < _dots.size(); i++) {

            juce::Point<float> *point = &_dots[i].pt;

            if (point->x - selectionBox.getX() <= selectionBox.getWidth() &&
                point->y - selectionBox.getY() <= selectionBox.getHeight()) {
                    _selected_idxs.emplace_back(i);
            }
        }
    }
    void deselect()
    {
        _selected_idxs.clear();
    }
    // delete() // selected
    // delete(Rect) // in box
    // drag(x, y)

private:

    std::vector<FreqDot> _dots; // Vector of dots (juce points)
    DotLnF _lnf; // Manages visuaks for the dots set
    FreqBoundMap& _boundMap;
    std::vector<int> _selected_idxs;
};
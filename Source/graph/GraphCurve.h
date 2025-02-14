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

enum  {

};

struct GraphDot
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

    GraphDot(float x, float y) : pt(x, y), linemode(lineMode::bezier) {}
};


/*! \brief Doesnt know about X/Y/freq/amp conversions, about freq and amp max */
class GraphCurve
{
public:
    GraphCurve(/* args */) {}
    ~GraphCurve() {}

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
                path.quadraticTo(ctrlPoint, _dots[i].pt); // TODO: change fct depending linemode
            }
        }
    }

    void addDot(juce::Point<float> point)
    {
        // Find the correct position in _dots to maintain sorted order by X
        auto it = std::lower_bound(_dots.begin(), _dots.end(), point,
            [](const GraphDot& dot, const juce::Point<float>& value) {
                return dot.pt.x < value.x;
            });

        // Insert new GraphDot at the found position
        _dots.insert(it, GraphDot(point.x, point.y));
    }

    // 2 functions for selection, since we know X increases for each dot,
    // when dragging, we can check 1st non selected dot to see if Y is in box
    // in caller: when ctrl is down and dragging, make rect with origin of drag and call this
    void select(juce::Rectangle<float> selectionBox)
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

private:

    std::vector<GraphDot> _dots; // Vector of dots (juce points)
    DotLnF _lnf; // Manages visuaks for the dots set
    std::vector<int> _selected_idxs;
};
#include "graphCmp.h"


void FrequencyGraph::resized()
{
    // Recreate static graph image when the component is resized
    createStaticGraph();
}

void FrequencyGraph::paint(juce::Graphics& g)
{

    // Draw the cached static graph

    g.drawImageAt(_staticGraph, 0, 0);

    // Draw dynamic elements (_dots and lines)

    g.setColour(juce::Colours::cyan);
    auto graphBounds = getGraphBounds();

    juce::Path path;
    for (size_t i = 0; i < _dots.size(); i++)
    {
        float x = frequencyToX(_dots[i].first, graphBounds);
        float y = amplitudeToY(_dots[i].second, graphBounds);
        std::cout << x << "Hz for left of line\n";

        if (i < _dots.size() - 1) {

            juce::Point<float> start(frequencyToX(_dots[i].first, getGraphBounds()),
                                    amplitudeToY(_dots[i].second, getGraphBounds()));
            juce::Point<float> end(frequencyToX(_dots[i + 1].first, getGraphBounds()),
                                amplitudeToY(_dots[i + 1].second, getGraphBounds()));

            juce::Rectangle<float> rect(_curvedLines[i].center.getX() - 5, _curvedLines[i].center.getY() - 5, 10, 10);
            g.drawEllipse(rect, 3);
            path.startNewSubPath(start);
            path.quadraticTo(_curvedLines[i].control, end);
        }

        g.fillEllipse(x - 5, y - 5, 10, 10); // Draw dot
    }

        //     float prevX = frequencyToX(_dots[i - 1].first, graphBounds);
        //     float prevY = amplitudeToY(_dots[i - 1].second, graphBounds);
        //     g.drawLine(prevX, prevY, x, y, 2.0f);
        // }
    g.strokePath(path, juce::PathStrokeType(2.0f));


}
//==========================

void FrequencyGraph::createStaticGraph()
{
    auto bounds = getLocalBounds();
    auto graphBounds = getGraphBounds();

    _staticGraph = juce::Image(juce::Image::RGB, bounds.getWidth(), bounds.getHeight(), true);
    juce::Graphics g(_staticGraph);

    // Draw background
    g.fillAll(juce::Colours::black);

    // Draw axes
    g.setColour(juce::Colours::white);
    g.drawRect(graphBounds);


    for (float freq = 10.0f; freq <= 10000.0f; freq *= 10.0f)
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
            if (subFreq > _freq_bounds.second) break; // Prevent exceeding max freq

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

juce::Rectangle<int> FrequencyGraph::getGraphBounds() const
{

    auto local = getLocalBounds();
    auto w = local.getWidth() * 0.05f;
    auto h = local.getHeight() * 0.05f;

    local = local.reduced(w, h);

    local.removeFromBottom(h); // Remove twice from bottom

    return local;
}



void FrequencyGraph::updateCurvedLines() {
    _curvedLines.clear();
    for (size_t i = 1; i < _dots.size(); ++i) {
        juce::Point<float> start(frequencyToX(_dots[i - 1].first, getGraphBounds()),
                                 amplitudeToY(_dots[i - 1].second, getGraphBounds()));
        juce::Point<float> end(frequencyToX(_dots[i].first, getGraphBounds()),
                               amplitudeToY(_dots[i].second, getGraphBounds()));

        _curvedLines.emplace_back(start, end);
    }
}
void FrequencyGraph::mouseDown(const juce::MouseEvent& event)
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
        _dragged_dot_idx = clickedDotIndex;
        return;
    } else {
        for (auto& line : _curvedLines) {
            if (event.getPosition().toFloat().getDistanceFrom(line.center) < 10.0f) {
                _draggingLine = &line;
                return;
            }
        }
    }
    // Otherwise, split the closest line
    float freq = xToFrequency(mouseX, graphBounds);
    float amp = yToAmplitude(mouseY, graphBounds);

    // TODO: check bounds in xy
    if (freq < _freq_bounds.first ||
        freq > _freq_bounds.second ||
        amp < -24.0f ||
        amp > 24.0f ) {
            return;
        }

    // Find the position to insert based on the first value (freq)
    // Since we work with a vector of pairs, we need a comparator fct (as a lambda)
    auto it = std::lower_bound(_dots.begin(), _dots.end(), freq,
        [](const std::pair<float, float>& dot, float value) {
            return dot.first < value;
        }
    );

    size_t index;

    if (it == _dots.end()) {
        index = _dots.size() - 1;
    } else {
        index = std::distance(_dots.begin(), it);
    }

    // Calculate index

    _dots.insert(_dots.begin() + index, { freq, amp });
    _dragged_dot_idx = index;

    // Generate new lines

    updateCurvedLines();

    repaint();



    // Debug
    for (size_t i = 0; i < _dots.size(); i++) {
        std::cout << _dots[i].first << "Hz, ";
    }
    std::cout << (index + 1) << "th dot added\n";

}



void FrequencyGraph::mouseDrag(const juce::MouseEvent& event)
{
    if (_dragged_dot_idx >= 0)
    {
        auto graphBounds = getGraphBounds();

        // Convert mouse position to frequency and amplitude
        float freq = xToFrequency(event.position.x, graphBounds);
        float amp = yToAmplitude(event.position.y, graphBounds);

        // Clamp values to valid ranges (Graph bounds, or adjacent dots)
        float leftBound = (_dragged_dot_idx > 0) ? _dots[_dragged_dot_idx-1].first : _freq_bounds.first;
        float rightBound = (_dragged_dot_idx < _dots.size() - 1) ? _dots[_dragged_dot_idx+1].first : _freq_bounds.second;
        freq = juce::jlimit(leftBound, rightBound, freq);
        amp = juce::jlimit(-24.0f, 24.0f, amp);


        float deltaFreq =  freq - _dots[_dragged_dot_idx].first;
        float deltaAmp =  amp - _dots[_dragged_dot_idx].second;
        // update adjacent line centers
        if (_dragged_dot_idx > 0) {
            // update left line
            _curvedLines[_dragged_dot_idx-1].center.addXY(deltaFreq/2, deltaAmp/2);

        }
        if (_dragged_dot_idx < _dots.size() - 1) {
            // update right line
            _curvedLines[_dragged_dot_idx].center.addXY(deltaFreq/2, deltaAmp/2);
        }
        updateCurvedLines();

        _dots[_dragged_dot_idx] = { freq, amp };

    } else if (_draggingLine) {
        _draggingLine->control.y = event.position.y; // Adjust curvature by dragging
        // Update center pos TODO:
    }
    repaint();
}

void FrequencyGraph::mouseUp(const juce::MouseEvent&)
{
    _dragged_dot_idx = -1; // Reset dragged dot
    _draggingLine = nullptr;
}

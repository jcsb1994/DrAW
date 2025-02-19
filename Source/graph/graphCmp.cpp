#include "graphCmp.h"


void FrequencyGraph::resized()
{
    // Recreate static graph image when the component is resized
    createStaticGraph();

    // Remap curve XY

}

void FrequencyGraph::paint(juce::Graphics& g)
{

    // Draw the cached static graph

    g.drawImageAt(_staticGraph, 0, 0);

    // Draw dynamic elements (_dots and lines)

    g.setColour(juce::Colours::cyan);
    auto graphBounds = getGraphBounds();

    juce::Path path;

    std::cout << "lines at start\n";
    // debug_curves();
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

            juce::Rectangle<float> rect(_curvedLines[i].center.getX() - 5, _curvedLines[i].control.getY() - 5, 10, 10);
            g.drawEllipse(rect, 3);
            path.startNewSubPath(start);
            std::cout << i << " loop ctrl " << _curvedLines[i].control.y << "\n";
            path.quadraticTo(_curvedLines[i].control, end);
        }

        g.fillEllipse(x - 5, y - 5, 10, 10); // Draw dot
    }
    std::cout << "lines at end\n";
    debug_curves();
    g.strokePath(path, juce::PathStrokeType(2.0f));
}
//==========================

void FrequencyGraph::createStaticGraph()
{
    auto bounds = getLocalBounds();
    auto graphBounds = getGraphBounds();

    if (_curvedLines.size() == 0) {
        addCurvedLine(0); // TODO:  not at the right place, should be somewhere else in init
    }

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
    for (float dB = _amp_bounds.first; dB <= _amp_bounds.second; dB += 6)
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


void FrequencyGraph::addCurvedLine(unsigned int index) {
    auto graphBounds = getGraphBounds();

    // Points for the line: dot at idx + next
    juce::Point<float> start(frequencyToX(_dots[index].first, graphBounds),
    amplitudeToY(_dots[index].second, graphBounds));
    juce::Point<float> end(frequencyToX(_dots[index+1].first, graphBounds),
    amplitudeToY(_dots[index+1].second, graphBounds));


    if (_curvedLines.size() > index) {
        juce::Point<float> prev(frequencyToX(_dots[index-1].first, graphBounds),
        amplitudeToY(_dots[index-1].second, graphBounds));
        _curvedLines.erase(_curvedLines.begin() + index);
        CurvedLine modifOldLine(prev, start);
        _curvedLines.insert(_curvedLines.begin() + index, modifOldLine);
    }

    CurvedLine newline(start, end);
    _curvedLines.insert(_curvedLines.begin() + index, newline);

    //     float newCenter = _curvedLines[index-1].center.x
    //     _curvedLines[index].control
    // }
}
void FrequencyGraph::updateCurvedLines() {
    // _curvedLines.clear();
    // auto graphBounds = getGraphBounds();
    // juce::Point<float> start(frequencyToX(_dots[i - 1].first, graphBounds),
    //                          amplitudeToY(_dots[i - 1].second, graphBounds));
    // juce::Point<float> end(frequencyToX(_dots[i].first, graphBounds),
    //                        amplitudeToY(_dots[i].second, graphBounds));
    // for (size_t i = 1; i < _dots.size(); ++i) {

    //     _curvedLines.emplace_back(start, end);
    // }
}
void FrequencyGraph::mouseDown(const juce::MouseEvent& event)
{
    auto graphBounds = getGraphBounds();
    float mouseX = event.position.x;
    float mouseY = event.position.y;

    std::cout << "Click: " << mouseX << ", " << mouseY << "\n";

    // Check if we clicked on an existing dot
    int clickedDotIndex = getClickedDotIndex(mouseX, mouseY, graphBounds);
    int clickedCurveIndex;

    if (clickedDotIndex != -1) {
        // Start dragging this dot
        _dragged_dot_idx = clickedDotIndex;
    } else {
        clickedCurveIndex = getClickedLineIndex(mouseX, mouseY, graphBounds);
        if (clickedCurveIndex != -1) {
            _dragged_line_idx = clickedCurveIndex;
        }
    }

    // Otherwise, split the closest line
    juce::Point<float> newDot;
    newDot.x = xToFrequency(mouseX, graphBounds);
    newDot.y = yToAmplitude(mouseY, graphBounds);

    // TODO: check bounds in xy
    if (newDot.x < _freq_bounds.first ||
        newDot.x > _freq_bounds.second ||
        newDot.y < _amp_bounds.first ||
        newDot.y > _amp_bounds.second ) {
            return;
        }

    _curve.addDot(newDot);

    repaint();

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
        amp = juce::jlimit(_amp_bounds.first, _amp_bounds.second, amp);


        float deltaFreq =  freq - _dots[_dragged_dot_idx].first;
        float deltaAmp =  amp - _dots[_dragged_dot_idx].second;
        // update adjacent line centers
        if (_dragged_dot_idx > 0) {
            // update left line
            // re-compute center
            _curvedLines[_dragged_dot_idx-1].center.x = _dots[_dragged_dot_idx].first - _dots[_dragged_dot_idx-1].first;
            _curvedLines[_dragged_dot_idx-1].center.y = _dots[_dragged_dot_idx].second - _dots[_dragged_dot_idx-1].second;
            // _curvedLines[_dragged_dot_idx-1].center.addXY(deltaFreq/2, deltaAmp/2);

        }
        if (_dragged_dot_idx < _dots.size() - 1) {
            _curvedLines[_dragged_dot_idx].center.x = _dots[_dragged_dot_idx+1].first - _dots[_dragged_dot_idx].first;
            _curvedLines[_dragged_dot_idx].center.y = _dots[_dragged_dot_idx+1].second - _dots[_dragged_dot_idx].second;
            // update right line
            // re-compute center
            // _curvedLines[_dragged_dot_idx].center.addXY(deltaFreq/2, deltaAmp/2);
        }

        _dots[_dragged_dot_idx] = { freq, amp };

    } else if (_draggingLine) {
        // // clamp line ctrl pt
        auto graphBounds = getGraphBounds();

        // Convert mouse position to frequency and amplitude
        float amp = yToAmplitude(event.position.y, graphBounds);

        if (amp > _amp_bounds.second) {
            amp =  _amp_bounds.second;
        } else if (amp < _amp_bounds.first) {
            amp =  _amp_bounds.first;
        }
        _draggingLine->control.y = amplitudeToY( amp, graphBounds);
        // auto graphBounds = getGraphBounds();
        // _draggingLine->control.y = event.position.y;
        std::cout << "amp " << amp << " event.position.y; "  << event.position.y << "\n";
        // Update center pos TODO:
    }
    repaint();
}

void FrequencyGraph::mouseUp(const juce::MouseEvent&)
{
    _dragged_dot_idx = -1; // Reset dragged dot
    _draggingLine = nullptr;
}

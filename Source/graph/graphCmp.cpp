#include "graphCmp.h"


void FrequencyGraph::resized()
{
    // Recreate static graph image when the component is resized
    createStaticGraph();

}

// Compute highest point of quadratic Bézier curve
juce::Point<float> getHighestPoint(juce::Point<float> p0, juce::Point<float> p1, juce::Point<float> p2)
{
    float denominator = p0.x - 2.0f * p1.x + p2.x;
    if (denominator == 0.0f) return p0; // Avoid division by zero (degenerate case)

    float t_vertex = (p0.x - p1.x) / denominator;
    t_vertex = juce::jlimit(0.0f, 1.0f, t_vertex); // Ensure t is in range [0, 1]

    // Bézier equation to get the point at t_vertex
    float x_high = (1 - t_vertex) * (1 - t_vertex) * p0.x +
                   2 * (1 - t_vertex) * t_vertex * p1.x +
                   t_vertex * t_vertex * p2.x;

    float y_high = (1 - t_vertex) * (1 - t_vertex) * p0.y +
                   2 * (1 - t_vertex) * t_vertex * p1.y +
                   t_vertex * t_vertex * p2.y;

    return {x_high, y_high}; // Highest point
}


void FrequencyGraph::paint(juce::Graphics& g)
{

    // Draw the cached static graph

    g.drawImageAt(_staticGraph, 0, 0);

    // Draw dynamic elements (_dots and lines)

    auto selected_it = _clicked_items.first.begin();
    auto selected_end = _clicked_items.first.end();
    juce::Path path;
    auto bounds = getGraphBounds(); // Note: getlocalbounds doesnt work.. why?

    for (int i = 0; i < _dots2.size(); i++) {
        bool selected = (selected_it != selected_end && i == *selected_it);
        if (selected) {
            selected_it++; // Move to the next selected index
        }

        float x = frequencyToX(_dots2[i].pt.x, bounds);
        float y = amplitudeToY(_dots2[i].pt.y, bounds);
        _lnf.drawDot(g, x, y, selected && (_clicked_items.second == graphElement::dot));

        if (i > 0) {
            float lastX = frequencyToX(_dots2[i-1].pt.x, bounds);
            float lastY = amplitudeToY(_dots2[i-1].pt.y, bounds);
            juce::Point<float> start(lastX, lastY);
            juce::Point<float> end(x, y);

            path.startNewSubPath(start);
            float centerX = (x + lastX) / 2;
            float centerY = ((y + lastY) / 2);

            juce::Point<float> ctrlPoint(centerX, centerY + _dots2[i].control);
            ctrlPoint.y = juce::jlimit((float)bounds.getTopLeft().y,(float)bounds.getBottom(), ctrlPoint.y);
            _lnf.drawLineCenter(g, centerX, centerY, selected && (_clicked_items.second == graphElement::line));
            path.quadraticTo(ctrlPoint, end); // TODO: change fct depending linemode
            juce::Point<float> highestPoint = getHighestPoint(start, ctrlPoint, end);
            std::cout << "Highest point: " << highestPoint.toString() << ctrlPoint.toString();

            // std::cout << "Line " << i << "\n";
            // printDot("Left", start);
            // printDot("Ctrl", ctrlPoint);
            // printDot("Right", end);
            // std::cout << "----\n";
            // FIXME: control doit etre perpendiculaire a slope, pas vertical
            // path.lineTo(_dots2[i].pt);
        }
    }
    _lnf.drawLine(g, path);
}

//==========================

void FrequencyGraph::createStaticGraph()
{
    auto bounds = getLocalBounds();
    auto graphBounds = getGraphBounds();

    // if (_curvedLines.size() == 0) {
    //     addCurvedLine(0); // TODO:  not at the right place, should be somewhere else in init
    // }

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

bool FrequencyGraph::isWithinGraphBounds(float x, float y) const
{
    auto bounds = getGraphBounds();

    return isWithinGraphBounds(x, y, bounds);
}

bool FrequencyGraph::isWithinGraphBounds(float x, float y, juce::Rectangle<int> graphBounds) const
{
    auto bounds = getGraphBounds();

    return !(x < graphBounds.getX() ||
           x > graphBounds.getX() + graphBounds.getWidth() ||
           y < graphBounds.getY() ||
           y > graphBounds.getY() + graphBounds.getHeight());
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


// void FrequencyGraph::addCurvedLine(unsigned int index) {
//     auto graphBounds = getGraphBounds();

//     // Points for the line: dot at idx + next
//     juce::Point<float> start(frequencyToX(_dots[index].first, graphBounds),
//     amplitudeToY(_dots[index].second, graphBounds));
//     juce::Point<float> end(frequencyToX(_dots[index+1].first, graphBounds),
//     amplitudeToY(_dots[index+1].second, graphBounds));


//     if (_curvedLines.size() > index) {
//         juce::Point<float> prev(frequencyToX(_dots[index-1].first, graphBounds),
//         amplitudeToY(_dots[index-1].second, graphBounds));
//         _curvedLines.erase(_curvedLines.begin() + index);
//         CurvedLine modifOldLine(prev, start);
//         _curvedLines.insert(_curvedLines.begin() + index, modifOldLine);
//     }

//     CurvedLine newline(start, end);
//     _curvedLines.insert(_curvedLines.begin() + index, newline);

//     //     float newCenter = _curvedLines[index-1].center.x
//     //     _curvedLines[index].control
//     // }
// }
// void FrequencyGraph::updateCurvedLines() {
//     // _curvedLines.clear();
//     // auto graphBounds = getGraphBounds();
//     // juce::Point<float> start(frequencyToX(_dots[i - 1].first, graphBounds),
//     //                          amplitudeToY(_dots[i - 1].second, graphBounds));
//     // juce::Point<float> end(frequencyToX(_dots[i].first, graphBounds),
//     //                        amplitudeToY(_dots[i].second, graphBounds));
//     // for (size_t i = 1; i < _dots.size(); ++i) {

//     //     _curvedLines.emplace_back(start, end);
//     // }
// }
void FrequencyGraph::mouseDown(const juce::MouseEvent& event)
{
    /*  1. if click is on dot, start dragging it
        2. if on curve */
    float mouseX = event.position.x;
    float mouseY = event.position.y;

    auto graphBounds = getGraphBounds();

    if (!isWithinGraphBounds(mouseX, mouseY, graphBounds)) {
        std::cout << "click out of graph\n";
        return; }


    std::cout << "Click: " << mouseX << ", " << mouseY << "\n";


    // Check if we clicked on an existing dot
    std::pair<int, graphElement> clickedItem = getClickedItem(mouseX, mouseY, graphBounds);

    if (clickedItem.first != -1) {
        // Save the clicked item
        _clicked_items.first.push_back(clickedItem.first);
        _clicked_items.second = clickedItem.second;

    } else { // Box selection begins

        // TODO: do drag, in up, check how much dragging, create dot if not a lot

        // Split the closest line
        float freq = xToFrequency(mouseX, graphBounds);
        float amp = yToAmplitude(mouseY, graphBounds);

        // TODO: check bounds in xy
        if (freq < _freq_bounds.first ||
            freq > _freq_bounds.second ||
            amp < _amp_bounds.first ||
            amp > _amp_bounds.second ) {
                return;
            }

        addDot(freq, amp);

        repaint();

    }



    // // Debug
    // for (size_t i = 0; i < _dots.size(); i++) {
    //     std::cout << _dots[i].first << "Hz, ";
    // }
    // std::cout << (index + 1) << "th dot added\n";

}



void FrequencyGraph::mouseDrag(const juce::MouseEvent& event)
{
    if (_clicked_items.first.size() > 0) {

        auto graphBounds = getGraphBounds();

        // Convert mouse position to frequency and amplitude
        float freq = xToFrequency(event.position.x, graphBounds);
        float amp = yToAmplitude(event.position.y, graphBounds);

        int dotIdx = _clicked_items.first[0]; // TODO: loop

        if (_clicked_items.second == graphElement::dot) {
            moveDot(dotIdx, freq, amp);
        } else if (_clicked_items.second == graphElement::line) {
            applyLineCtrl(dotIdx, event.position.x, event.position.y, graphBounds);
        }
    }

    repaint();
}

void FrequencyGraph::mouseUp(const juce::MouseEvent&)
{
    _clicked_items.first = {}; // No need to reset the graphElement

    repaint();
}

#include "graphCmp.h"


void FrequencyGraph::resized()
{
    // Recreate static graph image when the component is resized
    createStaticGraph();

}


#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <complex>

constexpr int fftSize = 1024;          // FFT size
constexpr double sampleRate = 44100.0;  // CD sample rate


int8_t generateWavIFFT(const std::vector<float>& freqs, const std::vector<float>& amps, const juce::String& outputPath)
{
    /*
    Since we create a repeating soundwave, we only need one chunk of samples. So 1024 samples that would in a forward
    FFT represent a single chunk or like 20ms of sound, is enough for our inverse FFT. We look at the user UI graph,
    and interpret it as a single FFT chunk of 1024 samples.

    So this function expects freqs and amps vectors to be len 512, because the function fills out the 512 last indexes with
    the mirror bins containing the negative frequencies as is needed by IFFTs.
    freqs should contain evenly spaced freqencies, (20kHz - 10Hz) / 512.


    We take the list of freqs and amps, and conglomerate them in freq bins. In an IFFT, each freq bin is
    a complex number of a given freq, which is the space between each bin. The freq space between each bin is
    sample Rate / FFT size = 48000/1024 ≈ 46.875 Hz. Bin 0 is always 0Hz, bin 1 is 46Hz, bin 2 is 92Hz, ... etc
    until bin 512 (fftSize / 2, which is the nyquist freq)
    */

    const int numBins = (fftSize / 2) + 1; // Number of real bins (0Hz..Nyquist)
    const float nyquistFreq = sampleRate / 2.0f;

    if (freqs.size() != numBins || freqs.size() != amps.size()) {
        return -1;
    }

    juce::dsp::FFT fft(static_cast<int>(std::log2(fftSize)));

    std::vector<std::complex<float>> freqDomain(fftSize);
    std::vector<std::complex<float>> outBuffer(fftSize);

    // 1. Fill bins 0..512 (DC to Nyquist)
    for (size_t k = 0; k < numBins; ++k) {
        freqDomain[k] = std::complex<float>(amps[k], 0.0f);
    }

    // 2. Fill bins 513..1023 (mirror)
    for (size_t k = numBins; k < fftSize; ++k) {
        freqDomain[k] = std::conj(freqDomain[fftSize - k]);
    }

    // 3. Perform the IFFT
    fft.perform(freqDomain.data(), outBuffer.data(), true);


    // Perform the IFFT

    /* It is allowed to pass a vector to a pointer fct parameter, but you must use vect.data() */
    fft.perform(freqDomain.data(), outBuffer.data(), true);

    // Normalize the time-domain signal
    const float maxSample = *std::max_element(freqDomain.begin(), freqDomain.end(),
                                              [](float a, float b) { return std::abs(a) < std::abs(b); });

    if (maxSample > 0.0f)
    {
        for (auto& sample : freqDomain)
            sample /= maxSample;  // Normalize to -1.0f to +1.0f range
    }
    // Write to WAV
    juce::AudioBuffer<float> buffer(1, fftSize);  // Mono channel
    auto* channelData = buffer.getWritePointer(0);
    std::copy(freqDomain.begin(), freqDomain.begin() + fftSize, channelData);

    juce::WavAudioFormat format;

    // Use unique_ptr for the stream
    std::unique_ptr<juce::OutputStream> outStream(juce::File(outputPath).createOutputStream());

    if (outStream)
    {
        // Create the writer and transfer ownership of the stream
        juce::AudioFormatWriter* writer = format.createWriterFor(outStream.get(), sampleRate, 1, 16, {}, 0);

        if (writer)
        {
            // Release the stream from the unique_ptr to avoid double deletion
            outStream.release();  // Prevent double free

            writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());

            delete writer;  // Safely delete the writer
        }
    }

}




void FrequencyGraph::genFreqPath()
{
    std::vector<float> freqData;
    std::vector<float> ampData;

    auto bounds = getGraphBounds();

    // const int numSamples = (fftSize / 2); // 512

    // float pathLength = _freqPath.getLength();

    // // Find the point at 1% of the total length

    // constexpr float freq_step = (1.0f / numSamples);

    // int skipped_steps = 0;
    // for (int i = 1; i <= numSamples; ++i) {

    //     juce::Point<float> pt = _freqPath.getPointAlongPath(pathLength * (i * freq_step));

    //     float freq = xToFrequency(pt.x, bounds);
    //     if (!freqData.empty() && (freq == freqData.back())) {
    //         std::cout << "vline ";
    //         skipped_steps++; // TODO: reduce step size along path, check end of path for vertical too
    //         continue;
    //     }




    //     float amp = yToAmplitude(pt.y, bounds);
    //     freqData.push_back(freq);
    //     ampData.push_back(amp);
    //     // std::cout << "\npt :" << i << " " << freq_step << " " << pt.toString() << " " << freq << " " << amp;
    // }

    auto binSpacing = 44100 / 1024;
    for (uint8_t i = 0; i < 512; i++) {
        freqData.push_back(binSpacing * i);
        ampData.push_back(0);
    }

    ampData[2] = 0.5;
    ampData[3] = 0.5;

    generateWavIFFT(freqData, ampData, "C:\\Users\\jcbsk\\Desktop\\test.wav");
    std::cout << "wav generated";
}


void FrequencyGraph::paint(juce::Graphics& g)
{

    // Draw the cached static graph

    g.drawImageAt(_staticGraph, 0, 0);

    // Draw dynamic elements (_dots and lines)

    auto selected_it = _clicked_items.first.begin();
    auto selected_end = _clicked_items.first.end();
    _freqPath.clear();

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

            _freqPath.startNewSubPath(start);
            float centerX = (x + lastX) / 2;
            float centerY = ((y + lastY) / 2);

            juce::Point<float> ctrlPoint(centerX, centerY + _dots2[i].control);
            ctrlPoint.y = juce::jlimit((float)bounds.getTopLeft().y,(float)bounds.getBottom(), ctrlPoint.y);
            _lnf.drawLineCenter(g, centerX, centerY, selected && (_clicked_items.second == graphElement::line));
            _freqPath.quadraticTo(ctrlPoint, end); // TODO: change fct depending linemode
            // juce::Point<float> highestPoint = getHighestPoint(start, ctrlPoint, end);
            // std::cout << "Highest point: " << highestPoint.toString() << ctrlPoint.toString();

        }
    }
    // TODO: generate _freqPath not in paint()
    _lnf.drawLine(g, _freqPath);
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

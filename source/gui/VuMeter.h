// Chioccia — VU meter with mechanical ballistics (original code)
#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

namespace chioccia::ui
{
/** Moving-coil style meter. The value source returns {mode, value}: mode false = gain reduction (dB,
    positive number), mode true = output level (dBFS RMS). 0 VU = −18 dBFS. */
class VuMeter final : public juce::Component, private juce::Timer
{
public:
    struct Reading { bool showsOutput = false; float valueDb = 0.0f; };
    explicit VuMeter (std::function<Reading()> source);
    ~VuMeter() override { stopTimer(); }

    void paint (juce::Graphics&) override;

    /** For tests / snapshots: jump the needle to a VU value without ballistics. */
    void setNeedleImmediately (float vu) { position = vu; velocity = 0.0f; repaint(); }
    float getNeedleVu() const noexcept { return position; }

private:
    void timerCallback() override;
    float vuToAngle (float vu) const;

    std::function<Reading()> source;
    float position = 0.0f, velocity = 0.0f;   // needle position in VU dB
    bool showsOutput = false;
    float lastValue = 0.0f;
};
} // namespace chioccia::ui

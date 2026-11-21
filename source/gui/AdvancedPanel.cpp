// Chioccia — advanced panel (original code)
#include "AdvancedPanel.h"

#include "ChiocciaLookAndFeel.h"
#include "PluginProcessor.h"

namespace chioccia::ui
{
using namespace juce;

AdvancedPanel::AdvancedPanel (ChiocciaProcessor& p) : processor (p)
{
    auto& s = processor.getState();
    const std::array<std::pair<const char*, const char*>, 6> knobDefs { {
        { params::id::inputTrim,  "INPUT TRIM" },
        { params::id::wetTrim,    "WET TRIM" },
        { params::id::stereoLink, "STEREO LINK" },
        { params::id::colour,     "COLOUR" },
        { params::id::scHpfFreq,  "DETECTOR HPF" },
        { params::id::noise,      "NOISE & HUM" } } };
    const std::array<const char*, 6> knobTips {
        "Input level into the whole plug-in (dry and wet).",
        "Level of the processed (wet) signal before it is summed with the dry signal.",
        "100 % = both channels always get the same gain (no image shift).",
        "Amount of tube/transformer colouration on the wet path only. 0 % = clean.",
        "High-pass filter in the detector only: bass triggers less compression. The audio is not filtered.",
        "Optional hardware-style noise floor and mains hum on the wet path. Off (0 %) by default." };

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];
        k.title = knobDefs[i].second;
        k.slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (Slider::NoTextBox, true, 0, 0);
        k.slider.getProperties().set (style::key, "smallKnob");
        k.slider.setTooltip (knobTips[i]);
        k.slider.setTitle (k.title);
        addAndMakeVisible (k.slider);
        k.attachment = std::make_unique<SliderAttachment> (s, knobDefs[i].first, k.slider);
    }

    const std::array<std::pair<const char*, const char*>, 5> comboDefs { {
        { params::id::quality,    "QUALITY" },
        { params::id::mixLaw,     "MIX LAW" },
        { params::id::listen,     "LISTEN" },
        { params::id::diagnostic, "DIAGNOSTIC" },
        { params::id::humFreq,    "HUM" } } };
    const std::array<const char*, 5> comboTips {
        "Oversampling of the colouration stage. Latency is identical in every mode (no host re-sync).",
        "Linear crossfade: (1 - mix) * dry + mix * wet.  Dry constant: dry + mix * wet (parallel-bus style).",
        "Audition dry only or wet only to check the blend.",
        "Null Residual outputs dry - wet with an identity wet path: silence means perfect time alignment.",
        "Mains hum frequency used by NOISE & HUM." };

    for (size_t i = 0; i < combos.size(); ++i)
    {
        auto& c = combos[i];
        c.title = comboDefs[i].second;
        if (auto* choice = dynamic_cast<AudioParameterChoice*> (s.getParameter (comboDefs[i].first)))
            c.box.addItemList (choice->choices, 1);
        c.box.setTooltip (comboTips[i]);
        c.box.setTitle (c.title);
        addAndMakeVisible (c.box);
        c.attachment = std::make_unique<ComboAttachment> (s, comboDefs[i].first, c.box);
    }

    for (auto* b : { &hpfOn, &compare })
    {
        b->getProperties().set (style::key, "pill");
        addAndMakeVisible (*b);
    }
    hpfOn.setTooltip ("Enable the detector high-pass filter.");
    compare.setTooltip ("Monitoring aid: slowly matches the output loudness to the input so you can judge the "
                        "sound without the louder-is-better bias. Off by default; never use it to print.");
    hpfOnAttachment = std::make_unique<ButtonAttachment> (s, params::id::scHpfOn, hpfOn);
    compareAttachment = std::make_unique<ButtonAttachment> (s, params::id::compare, compare);

    close.setTooltip ("Close the advanced panel");
    close.onClick = [this] { if (onClose) onClose(); };
    addAndMakeVisible (close);

    timerCallback();
    startTimerHz (8);
}

void AdvancedPanel::timerCallback()
{
    const auto& e = processor.getEngine();
    const double sr = processor.getSampleRate() > 0.0 ? processor.getSampleRate() : 48000.0;
    const int lat = processor.getLatencySamples();
    String t = "LATENCY " + String (lat) + " SMP (" + String (1000.0 * lat / sr, 2) + " MS)";
    if (processor.getState().getRawParameterValue (params::id::compare)->load() > 0.5f)
        t << "   COMPARE GAIN " << (e.getCompareGainDb() >= 0.0f ? "+" : "") << String (e.getCompareGainDb(), 1) << " DB";
    if (e.getActiveDiagnostic() != dsp::Diagnostic::off)
        t << "   DIAGNOSTIC ACTIVE";
    if (t != info) { info = t; repaint(); }
}

void AdvancedPanel::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const float s = r.getWidth() / 548.0f;
    g.setColour (Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (r.translated (0.0f, 4.0f * s), 14.0f * s);
    g.setGradientFill (ColourGradient (colours::chocolate.brighter (0.15f), 0.0f, r.getY(), colours::chocolate.darker (0.4f), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 14.0f * s);
    g.setColour (colours::gold);
    g.drawRoundedRectangle (r.reduced (1.5f * s), 14.0f * s, 2.0f * s);

    g.setColour (colours::goldLight);
    g.setFont (ChiocciaLookAndFeel::font (20.0f * s));
    g.drawText ("ADVANCED", Rectangle<float> (16.0f * s, 8.0f * s, 300.0f * s, 24.0f * s), Justification::centredLeft, false);

    g.setColour (colours::cream);
    g.setFont (ChiocciaLookAndFeel::font (12.0f * s));
    for (auto& k : knobs)
        g.drawText (k.title, k.slider.getBounds().toFloat().withY (k.slider.getY() - 15.0f * s).withHeight (14.0f * s), Justification::centred, false);
    for (auto& c : combos)
        g.drawText (c.title, c.box.getBounds().toFloat().withY (c.box.getY() - 15.0f * s).withHeight (14.0f * s), Justification::centred, false);

    g.setColour (colours::goldLight.withAlpha (0.9f));
    g.setFont (ChiocciaLookAndFeel::font (12.0f * s));
    g.drawText (info, Rectangle<float> (16.0f * s, r.getHeight() - 26.0f * s, r.getWidth() - 32.0f * s, 18.0f * s), Justification::centredRight, false);
}

void AdvancedPanel::resized()
{
    const float s = (float) getWidth() / 548.0f;
    auto R = [s] (float x, float y, float w, float h) { return Rectangle<float> (x * s, y * s, w * s, h * s).toNearestInt(); };

    close.setBounds (R (510.0f, 8.0f, 28.0f, 24.0f));
    for (size_t i = 0; i < knobs.size(); ++i)
        knobs[i].slider.setBounds (R (14.0f + 88.0f * (float) i, 52.0f, 80.0f, 80.0f));
    hpfOn.setBounds (R (14.0f + 88.0f * 4.0f + 2.0f, 136.0f, 76.0f, 20.0f));
    for (size_t i = 0; i < combos.size(); ++i)
        combos[i].box.setBounds (R (14.0f + 104.0f * (float) i, 180.0f, 98.0f, 26.0f));
    compare.setBounds (R (14.0f, 222.0f, 200.0f, 24.0f));
}
} // namespace chioccia::ui

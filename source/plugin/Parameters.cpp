// Chioccia — parameter definitions (original code)
#include "Parameters.h"

namespace chioccia::params
{
namespace
{
using juce::AudioParameterBool;
using juce::AudioParameterChoice;
using juce::AudioParameterFloat;
using juce::NormalisableRange;
using juce::String;

juce::ParameterID pid (const char* name) { return { name, kParameterVersion }; }

std::unique_ptr<AudioParameterFloat> makeFloat (const char* name, const String& label, NormalisableRange<float> range,
                                                float def, const String& unit,
                                                std::function<String (float, int)> toText,
                                                std::function<float (const String&)> fromText = {})
{
    auto attrs = juce::AudioParameterFloatAttributes().withLabel (unit).withStringFromValueFunction (std::move (toText));
    if (fromText) attrs = attrs.withValueFromStringFunction (std::move (fromText));
    return std::make_unique<AudioParameterFloat> (pid (name), label, range, def, attrs);
}

String dbText (float v, int)      { return (v > 0.0f ? "+" : "") + String (v, 1) + " dB"; }
String pctText (float v, int)     { return String (v, 0) + " %"; }
float  parseLeadingNumber (const String& s) { return s.retainCharacters ("+-.0123456789").getFloatValue(); }
} // namespace

const juce::StringArray& allIds()
{
    static const juce::StringArray ids {
        id::peakReduction, id::gain, id::compLimit, id::mix, id::outputTrim, id::vuMode, id::bypass,
        id::inputTrim, id::wetTrim, id::stereoLink, id::scHpfOn, id::scHpfFreq, id::colour, id::quality,
        id::compare, id::noise, id::humFreq, id::mixLaw, id::listen, id::diagnostic };
    return ids;
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const NormalisableRange<float> knob100 { 0.0f, 100.0f, 0.1f };
    const NormalisableRange<float> trim { -24.0f, 24.0f, 0.1f };

    // --- main panel -------------------------------------------------------------------------------
    layout.add (makeFloat (id::peakReduction, "Peak Reduction", knob100, 30.0f, "",
        [] (float v, int) { return String (v, 1) + " (SC " + String (dsp::mapping::peakReductionToDriveDb (v), 1) + " dB)"; },
        parseLeadingNumber));
    layout.add (makeFloat (id::gain, "Gain", knob100, 50.0f, "",
        [] (float v, int) { return String (v, 1) + " (" + dbText (dsp::mapping::gainKnobToDb (v), 0) + ")"; },
        parseLeadingNumber));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::compLimit), "Compress/Limit", juce::StringArray { "Compress", "Limit" }, 0));
    layout.add (makeFloat (id::mix, "Mix", knob100, 100.0f, "%", pctText, parseLeadingNumber));
    layout.add (makeFloat (id::outputTrim, "Output Trim", trim, 0.0f, "dB", dbText, parseLeadingNumber));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::vuMode), "VU Meter", juce::StringArray { "Gain Reduction", "Output" }, 0,
        juce::AudioParameterChoiceAttributes().withAutomatable (false)));
    layout.add (std::make_unique<AudioParameterBool> (pid (id::bypass), "Bypass", false));

    // --- advanced panel ---------------------------------------------------------------------------
    layout.add (makeFloat (id::inputTrim, "Input Trim", trim, 0.0f, "dB", dbText, parseLeadingNumber));
    layout.add (makeFloat (id::wetTrim, "Wet Trim", trim, 0.0f, "dB", dbText, parseLeadingNumber));
    layout.add (makeFloat (id::stereoLink, "Stereo Link", knob100, 100.0f, "%", pctText, parseLeadingNumber));
    layout.add (std::make_unique<AudioParameterBool> (pid (id::scHpfOn), "Detector HPF", false));
    NormalisableRange<float> hpfRange { 20.0f, 500.0f, 1.0f };
    hpfRange.setSkewForCentre (100.0f);
    layout.add (makeFloat (id::scHpfFreq, "Detector HPF Freq", hpfRange, 90.0f, "Hz",
        [] (float v, int) { return String (juce::roundToInt (v)) + " Hz"; }, parseLeadingNumber));
    layout.add (makeFloat (id::colour, "Colour", knob100, 50.0f, "%", pctText, parseLeadingNumber));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::quality), "Quality",
        juce::StringArray { "Eco (1x)", "High (2x)", "Ultra (4x)" }, 1));
    layout.add (std::make_unique<AudioParameterBool> (pid (id::compare), "Level-Matched Compare", false));
    layout.add (makeFloat (id::noise, "Noise & Hum", knob100, 0.0f, "%", pctText, parseLeadingNumber));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::humFreq), "Hum Frequency", juce::StringArray { "50 Hz", "60 Hz" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::mixLaw), "Mix Law",
        juce::StringArray { "Linear Crossfade", "Dry Constant" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::listen), "Listen",
        juce::StringArray { "Mix", "Dry Only", "Wet Only" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (pid (id::diagnostic), "Diagnostic",
        juce::StringArray { "Off", "Null Residual", "Wet Identity" }, 0,
        juce::AudioParameterChoiceAttributes().withAutomatable (false)));
    return layout;
}

std::atomic<float>* ParameterReader::get (juce::AudioProcessorValueTreeState& s, const char* name)
{
    auto* p = s.getRawParameterValue (name);
    jassert (p != nullptr);
    return p;
}

ParameterReader::ParameterReader (juce::AudioProcessorValueTreeState& s)
    : peakReduction (get (s, id::peakReduction)), gain (get (s, id::gain)), compLimit (get (s, id::compLimit)),
      mix (get (s, id::mix)), outputTrim (get (s, id::outputTrim)), vuMode (get (s, id::vuMode)), bypass (get (s, id::bypass)),
      inputTrim (get (s, id::inputTrim)), wetTrim (get (s, id::wetTrim)), stereoLink (get (s, id::stereoLink)),
      scHpfOn (get (s, id::scHpfOn)), scHpfFreq (get (s, id::scHpfFreq)), colour (get (s, id::colour)),
      quality (get (s, id::quality)), compare (get (s, id::compare)), noise (get (s, id::noise)),
      humFreq (get (s, id::humFreq)), mixLaw (get (s, id::mixLaw)), listen (get (s, id::listen)),
      diagnostic (get (s, id::diagnostic))
{
}

dsp::EngineParameters ParameterReader::read() const noexcept
{
    auto v = [] (const std::atomic<float>* a) { return a->load (std::memory_order_relaxed); };
    auto idx = [&v] (const std::atomic<float>* a) { return juce::roundToInt (v (a)); };

    dsp::EngineParameters p;
    p.peakReduction       = v (peakReduction);
    p.gain                = v (gain);
    p.limitMode           = idx (compLimit) == 1;
    p.mixPercent          = v (mix);
    p.outputTrimDb        = v (outputTrim);
    p.inputTrimDb         = v (inputTrim);
    p.wetTrimDb           = v (wetTrim);
    p.stereoLinkPercent   = v (stereoLink);
    p.detectorHpfOn       = v (scHpfOn) > 0.5f;
    p.detectorHpfHz       = v (scHpfFreq);
    p.colourPercent       = v (colour);
    p.quality             = (dsp::Quality) juce::jlimit (0, 2, idx (quality));
    p.levelMatchedCompare = v (compare) > 0.5f;
    p.noisePercent        = v (noise);
    p.humHz               = idx (humFreq) == 1 ? 60 : 50;
    p.mixLaw              = idx (mixLaw) == 1 ? dsp::MixLaw::dryConstant : dsp::MixLaw::linearCrossfade;
    p.listen              = (dsp::Listen) juce::jlimit (0, 2, idx (listen));
    p.diagnostic          = (dsp::Diagnostic) juce::jlimit (0, 2, idx (diagnostic));
    p.bypass              = v (bypass) > 0.5f;
    return p;
}
} // namespace chioccia::params

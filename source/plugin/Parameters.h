// Chioccia — parameter definitions (original code)
//
// Parameter IDs are part of the saved-session format and of host automation: NEVER rename them.
// Add new parameters with a new ID and a higher ParameterID version hint (see kParameterVersion).
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "EngineParameters.h"

namespace chioccia::params
{
inline constexpr int kParameterVersion = 1;   // version hint used by all parameters of release 0.1

namespace id
{
    // Main panel
    inline constexpr const char* peakReduction = "peakReduction";
    inline constexpr const char* gain          = "gain";
    inline constexpr const char* compLimit     = "compLimit";
    inline constexpr const char* mix           = "mix";
    inline constexpr const char* outputTrim    = "outputTrim";
    inline constexpr const char* vuMode        = "vuMode";
    inline constexpr const char* bypass        = "bypass";
    // Advanced panel
    inline constexpr const char* inputTrim     = "inputTrim";
    inline constexpr const char* wetTrim       = "wetTrim";
    inline constexpr const char* stereoLink    = "stereoLink";
    inline constexpr const char* scHpfOn       = "scHpfOn";
    inline constexpr const char* scHpfFreq     = "scHpfFreq";
    inline constexpr const char* colour        = "colour";
    inline constexpr const char* quality       = "quality";
    inline constexpr const char* compare       = "compare";
    inline constexpr const char* noise         = "noise";
    inline constexpr const char* humFreq       = "humFreq";
    inline constexpr const char* mixLaw        = "mixLaw";
    inline constexpr const char* listen        = "listen";
    inline constexpr const char* diagnostic    = "diagnostic";
} // namespace id

/** All IDs, in a fixed order. A test checks this list so accidental renames are caught. */
const juce::StringArray& allIds();

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

/** Lock-free view of the parameter values for the audio thread. */
class ParameterReader
{
public:
    explicit ParameterReader (juce::AudioProcessorValueTreeState& apvts);
    dsp::EngineParameters read() const noexcept;
    bool vuShowsOutput() const noexcept { return vuMode->load (std::memory_order_relaxed) > 0.5f; }

private:
    std::atomic<float>* get (juce::AudioProcessorValueTreeState& s, const char* pid);
    std::atomic<float> *peakReduction, *gain, *compLimit, *mix, *outputTrim, *vuMode, *bypass,
                       *inputTrim, *wetTrim, *stereoLink, *scHpfOn, *scHpfFreq, *colour, *quality,
                       *compare, *noise, *humFreq, *mixLaw, *listen, *diagnostic;
};
} // namespace chioccia::params

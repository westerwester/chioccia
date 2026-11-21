// Chioccia — plain parameter set passed from the host layer to the DSP engine (original code)
#pragma once

#include <algorithm>

namespace chioccia::dsp
{
enum class Quality  : int { x1 = 0, x2 = 1, x4 = 2 };
enum class MixLaw   : int { linearCrossfade = 0, dryConstant = 1 };
enum class Listen   : int { mix = 0, dryOnly = 1, wetOnly = 2 };
enum class Diagnostic : int { off = 0, nullResidual = 1, wetIdentity = 2 };

struct EngineParameters
{
    float peakReduction   = 30.0f;   // 0..100
    float gain            = 50.0f;   // 0..100 (50 = 0 dB)
    bool  limitMode       = false;
    float mixPercent      = 100.0f;  // 0..100
    float outputTrimDb    = 0.0f;    // -24..+24
    float inputTrimDb     = 0.0f;    // -24..+24
    float wetTrimDb       = 0.0f;    // -24..+24
    float stereoLinkPercent = 100.0f;// 0..100
    bool  detectorHpfOn   = false;
    float detectorHpfHz   = 90.0f;   // 20..500
    float colourPercent   = 50.0f;   // 0..100
    Quality quality       = Quality::x2;
    bool  levelMatchedCompare = false;
    float noisePercent    = 0.0f;    // 0..100 (0 = off)
    int   humHz           = 50;      // 50 or 60
    MixLaw mixLaw         = MixLaw::linearCrossfade;
    Listen listen         = Listen::mix;
    Diagnostic diagnostic = Diagnostic::off;
    bool  bypass          = false;
};

/** Control laws (documented in docs/02-parametri.md). Shared by engine, GUI read-outs and tests. */
namespace mapping
{
    inline float peakReductionToDriveDb (float pr) noexcept  { return -10.0f + 0.6f * pr; }
    /** Below PR = 2 the side-chain is faded to zero, so PR = 0 means "no compression at all". */
    inline float peakReductionEnable (float pr) noexcept     { return std::clamp (pr * 0.5f, 0.0f, 1.0f); }
    inline float gainKnobToDb (float g) noexcept             { return (g - 50.0f) * 0.6f; }
    inline float colourToAmount (float c) noexcept           { return std::clamp (c / 20.0f, 0.0f, 1.0f); }
    inline float colourToDriveDb (float c) noexcept          { return (c - 50.0f) * 0.24f; }
    constexpr float kExponentCompress = 3.5f;
    constexpr float kExponentLimit    = 12.0f;
    constexpr float kNoiseFullScaleDb = -80.0f;  // RMS of the noise at 100 %
    constexpr float kHumFullScaleDb   = -88.0f;  // RMS of the hum fundamental at 100 %
}
} // namespace chioccia::dsp

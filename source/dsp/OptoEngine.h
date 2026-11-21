// Chioccia — complete processing engine: dry/wet paths, latency compensation, mixing, metering.
// Original code, independent of JUCE so it can be unit-tested and measured on its own.
//
//  in ─► sanitize ─► Input Trim ─┬───────────────► dry delay (L) ─────────────────────┐
//                                │                                                     │
//                                └► × cell gain ─► × Gain ─► [OS ► colour ► OS] ─►     ├► mix law ─► listen ─► Output Trim
//                                     ▲                       + pad (L − L_mode)      │     ▲
//                                     │ feedback (post-cell)  + noise/hum ─► Wet Trim ┘     │
//                                 detector ◄─────────────────────────────────────────       │
//  raw in ─► bypass delay (L) ──────────────────────────────────────────────── bypass xfade ┘
//
// L (reported latency) is constant for a given build: the 4x path latency. Lower quality modes are
// padded with pure delay, so changing quality never changes the latency reported to the host.
#pragma once

#include <array>
#include <atomic>
#include <vector>

#include "ColourStage.h"
#include "DelayLine.h"
#include "Detector.h"
#include "EngineParameters.h"
#include "HalfBandOversampler.h"
#include "OpticalCell.h"
#include "Smoothers.h"

namespace chioccia::dsp
{
class OptoEngine
{
public:
    static constexpr int kMaxChannels = 2;
    static constexpr int kNumQualities = 3;
    static constexpr int kStage1Taps = 131;   // 4k+3, passband ≈ 0.45 fs, ≥ 100 dB stopband
    static constexpr int kStage2Taps = 27;    // 4k+3, second (2x→4x) stage
    static constexpr double kStopbandDb = 100.0;
    static constexpr int kPrimeLength = 256;  // history used to warm up a path on quality change

    OptoEngine();

    /** Allocates everything. Not real-time safe. numChannels: 1 or 2. */
    void prepare (double sampleRate, int numChannels);
    /** Clears all audio state (delays, filters, cell) without reallocating. */
    void reset() noexcept;

    /** Reads plain parameter values and sets smoother targets. Real-time safe. */
    void setParameters (const EngineParameters& p) noexcept;

    /** In-place processing of numChannels (≤ prepared channels) × numSamples. Real-time safe. */
    void process (float* const* channels, int numChannels, int numSamples) noexcept;

    /** Constant latency of the plug-in (samples at the host rate). */
    int getLatencySamples() const noexcept { return totalLatency; }
    int getPathLatency (Quality q) const noexcept { return pathLatency[(size_t) q]; }

    // --- metering (written by the audio thread, read by any thread) ---------------------------------
    float getGainReductionDb() const noexcept { return meterGrDb.load (std::memory_order_relaxed); }
    float getOutputRmsDb() const noexcept     { return meterOutDb.load (std::memory_order_relaxed); }
    float getInputRmsDb() const noexcept      { return meterInDb.load (std::memory_order_relaxed); }
    float getCompareGainDb() const noexcept   { return meterCompareDb.load (std::memory_order_relaxed); }
    Quality getActiveQuality() const noexcept { return (Quality) activeQuality; }
    Diagnostic getActiveDiagnostic() const noexcept { return (Diagnostic) activeDiagnostic; }

    /** Static configs, exposed so tests and the measurement tool can inspect / tune them. */
    OpticalCellConfig cellConfig;
    DetectorConfig detectorConfig;
    ColourConfig colourConfig;

private:
    struct WetPath
    {
        Oversampler os;
        ColourStage colour;
        IntegerDelay pad;
    };

    struct Channel
    {
        Detector detector;
        OpticalCell cell;
        IntegerDelay dryDelay, bypassDelay;
        std::array<WetPath, kNumQualities> paths;
        std::array<float, kPrimeLength> history {};
        XorShift32 noise;
        double feedback = 0.0;
    };

    float runPath (Channel& ch, int q, float u, float amount, float colourDrive, bool identity) noexcept;
    void startQualityChange (int newQuality) noexcept;
    void applyParameters (const EngineParameters& p, bool immediate) noexcept;

    EngineParameters lastParams;
    double fs = 48000.0;
    int numChannelsPrepared = 2;
    std::array<Channel, kMaxChannels> chans;
    std::array<int, kNumQualities> pathLatency { 0, 0, 0 };
    int totalLatency = 0;
    int historyPos = 0;

    // Smoothed controls
    DbGainSmoother sInputTrim, sMakeup, sWetTrim, sOutputTrim, sDrive, sColourDrive;
    LinearSmoother sDriveEnable, sMix, sLink, sExponent, sColourAmount, sLawA, sHpfLog2,
                   sListenMix, sListenDry, sListenWet, sBypass, sCompare, sNoise, sDiagGain;
    int hpfUpdateCounter = 0;

    // Discrete states
    int activeQuality = 1, fadingQuality = -1, xfadeRemaining = 0, xfadeLength = 1;
    int requestedQuality = 1;
    int activeDiagnostic = 0, requestedDiagnostic = 0;
    int humHz = 50;
    double humPhase = 0.0;

    // Level-matched compare
    double compareInPow = 0.0, compareOutPow = 0.0, compareGain = 1.0;
    double aComparePow = 0.0, aCompareGain = 0.0;

    std::atomic<float> meterGrDb { 0.0f }, meterOutDb { -144.0f }, meterInDb { -144.0f }, meterCompareDb { 0.0f };
};
} // namespace chioccia::dsp

// Chioccia — side-chain detector and EL-panel driver (original code)
//
// Signal: post-cell audio (feedback topology w.r.t. the attenuator, taken *before* the Gain stage,
// so the Gain control never changes the amount of compression).
//   HPF (DC removal at 10 Hz, or user detector HPF) -> fixed HF emphasis shelf
//   -> drive (Peak Reduction) -> EL panel light = max(0, |v| - v0)^p
//   -> EL persistence (fast rise, short decay)
// p sets the knee/ratio family: Compress ~ 4:1 asymptotic, Limit ~ 10:1 (see docs/03-modello-dsp.md).
#pragma once

#include "Biquad.h"

namespace chioccia::dsp
{
struct DetectorConfig
{
    double emphasisFreqHz   = 2500.0;  // fixed side-chain HF emphasis (approximation)
    double emphasisGainDb   = 2.0;
    double elThreshold      = 0.02;    // light onset, relative to drive-normalised level
    double elRiseTau        = 0.0002;  // s
    double elFallTau        = 0.0020;  // s
    double dcBlockHz        = 10.0;    // used when the user HPF is off
    double maxLight         = 1.0e4;   // EL / photocell saturation (caps gain reduction near 70 dB)
};

class Detector
{
public:
    void prepare (double sampleRate, const DetectorConfig& cfg) noexcept;
    void reset() noexcept;

    /** Sets the side-chain high-pass corner (Hz). Cheap enough to call at control rate. */
    void setHighPass (double hz) noexcept;

    /** One sample: side-chain input s, drive (linear), EL exponent p. Returns light (≥ 0). */
    inline double process (double s, double drive, double exponent) noexcept
    {
        const double f = emphasis.process (hpf.process (s));
        const double v = std::abs (f) * drive - cfg.elThreshold;
        const double lightNow = v > 0.0 ? std::min (std::pow (v, exponent), cfg.maxLight) : 0.0;
        el += (lightNow - el) * (lightNow > el ? aRise : aFall);
        el = flushTiny (el);
        return el;
    }

private:
    DetectorConfig cfg;
    double fs = 48000.0, currentHpfHz = -1.0;
    Biquad hpf, emphasis;
    double el = 0.0, aRise = 1.0, aFall = 1.0;
};
} // namespace chioccia::dsp

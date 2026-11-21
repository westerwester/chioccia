// Chioccia — behavioural model of an electro-optical attenuator (EL panel + CdS photoresistor)
// Original code. This is NOT a circuit-level emulation of any specific unit: it is a behavioural
// model whose constants were tuned against publicly documented behaviour of classic opto levelers
// (≈10 ms average attack, ≈60 ms to 50 % release, 1–15 s to full release depending on history).
//
// State variables (all in "light" units, ≥ 0):
//   c  fast photo-carriers  -> directly sets the cell conductance (and therefore the attenuation)
//   m  memory (trap) state  -> charges while compressing, sets the floor of the fast release
//   d  deep-trap state      -> charges over many seconds, slows the release of m ("history")
//
// Attenuation of the shunt divider:  g = 1 / (1 + K * c^gamma)
#pragma once

#include "DspCommon.h"

namespace chioccia::dsp
{
struct OpticalCellConfig
{
    // Attack runs in the log-light domain. Its time constant is scaled by (1 + attackLoopNorm * p * gamma)
    // so that Compress and Limit (different loop gains in the feedback topology) end up with similar
    // closed-loop attack times (measured ≈ 10 ms at moderate GR, see docs/misure).
    double attackTauSmall   = 0.014;   // s, attack for small light increases
    double attackTauLarge   = 0.006;   // s, attack for large light steps (program-dependent)
    double attackStepRef    = 3.0;     // log-light step at which attack is half-way between the two
    double attackLogFloor   = 1.0e-4;  // light floor used for the log domain (≈ 0.003 dB GR)
    double attackLoopNorm   = 0.12;
    double releaseFastTau   = 0.040;   // s, first release stage
    double releaseFloor     = 0.22;    // fraction of the memory state the fast release falls to
    double memoryChargeTau  = 1.5;     // s
    double memoryReleaseTau = 0.45;    // s, with empty deep traps
    double memoryReleaseTauDeep = 2.5; // s, added when deep traps are full
    double deepChargeTau    = 6.0;     // s
    double deepReleaseTau   = 20.0;    // s
    double ldrGamma         = 0.85;    // CdS sensitivity exponent
    double ldrK             = 1.0;     // divider scaling
    double memoryCap        = 50.0;    // trap saturation (≈ 30 dB GR): bounds recovery after overloads
};

class OpticalCell
{
public:
    void prepare (double sampleRate, const OpticalCellConfig& cfg) noexcept;
    void reset() noexcept { c = m = d = 0.0; gain = 1.0; }

    /** Advances the cell by one sample. light ≥ 0; elExponent is the detector exponent p (used to
        normalise the attack for the loop gain). Returns linear gain (0, 1]. */
    double process (double light, double elExponent) noexcept;

    double getGain() const noexcept { return gain; }
    double getFastState() const noexcept { return c; }
    double getMemoryState() const noexcept { return m; }
    double getDeepState() const noexcept { return d; }

private:
    OpticalCellConfig cfg;
    double dt = 1.0 / 48000.0;
    double aRelFast = 0.0, aMemCharge = 0.0, aDeepCharge = 0.0, aDeepRelease = 0.0;
    double c = 0.0, m = 0.0, d = 0.0, gain = 1.0;
};
} // namespace chioccia::dsp

// Chioccia — behavioural opto-cell model (original code)
#include "OpticalCell.h"

namespace chioccia::dsp
{
void OpticalCell::prepare (double sampleRate, const OpticalCellConfig& config) noexcept
{
    cfg = config;
    dt = 1.0 / sampleRate;
    aRelFast     = onePoleCoeff (cfg.releaseFastTau, sampleRate);
    aMemCharge   = onePoleCoeff (cfg.memoryChargeTau, sampleRate);
    aDeepCharge  = onePoleCoeff (cfg.deepChargeTau, sampleRate);
    aDeepRelease = onePoleCoeff (cfg.deepReleaseTau, sampleRate);
    reset();
}

double OpticalCell::process (double light, double elExponent) noexcept
{
    constexpr double eps = 1.0e-12;
    light = std::isfinite (light) ? std::max (0.0, light) : 0.0;

    // --- fast carriers --------------------------------------------------------------------------
    if (light > c)
    {
        // Program-dependent attack in the log domain: the larger the light step relative to the
        // present state, the faster the photoresistor responds.
        const double lc = std::log (c + cfg.attackLogFloor);
        const double step = std::log (light + cfg.attackLogFloor) - lc;     // > 0
        const double t = step / (step + cfg.attackStepRef);                  // 0 .. 1
        const double loopNorm = 1.0 + cfg.attackLoopNorm * std::max (1.0, elExponent) * cfg.ldrGamma;
        const double tau = (cfg.attackTauSmall + (cfg.attackTauLarge - cfg.attackTauSmall) * t) * loopNorm;
        c = std::exp (lc + step * (1.0 - std::exp (-dt / tau))) - cfg.attackLogFloor;
        c = std::max (0.0, c);
    }
    else
    {
        // Two-stage release: fast fall towards a floor set by the trap (memory) state.
        const double floorLevel = std::max (light, cfg.releaseFloor * m);
        c += (floorLevel - c) * aRelFast;
    }

    // --- memory (trap) state ----------------------------------------------------------------------
    if (c > m)
    {
        m = std::min (m + (c - m) * aMemCharge, cfg.memoryCap);
    }
    else
    {
        const double deepRatio = std::clamp (d / (m + eps), 0.0, 1.0);
        const double tau = cfg.memoryReleaseTau + cfg.memoryReleaseTauDeep * deepRatio;
        m += (c - m) * (1.0 - std::exp (-dt / tau));
    }

    // --- deep traps (long-term history) -----------------------------------------------------------
    d += (m - d) * (m > d ? aDeepCharge : aDeepRelease);

    c = flushTiny (c); m = flushTiny (m); d = flushTiny (d);

    if (! (std::isfinite (c) && std::isfinite (m) && std::isfinite (d)))
        reset();

    gain = 1.0 / (1.0 + cfg.ldrK * std::pow (c, cfg.ldrGamma));
    return gain;
}
} // namespace chioccia::dsp

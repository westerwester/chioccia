// Chioccia — side-chain detector (original code)
#include "Detector.h"

namespace chioccia::dsp
{
void Detector::prepare (double sampleRate, const DetectorConfig& config) noexcept
{
    cfg = config;
    fs = sampleRate;
    aRise = onePoleCoeff (cfg.elRiseTau, fs);
    aFall = onePoleCoeff (cfg.elFallTau, fs);
    emphasis.setCoeffs (BiquadCoeffs::highShelf (fs, cfg.emphasisFreqHz, cfg.emphasisGainDb, 0.7));
    currentHpfHz = -1.0;
    setHighPass (cfg.dcBlockHz);
    reset();
}

void Detector::reset() noexcept
{
    hpf.reset();
    emphasis.reset();
    el = 0.0;
}

void Detector::setHighPass (double hz) noexcept
{
    hz = std::clamp (hz, 5.0, 1000.0);
    if (std::abs (hz - currentHpfHz) < 1.0e-3) return;
    currentHpfHz = hz;
    hpf.setCoeffs (BiquadCoeffs::highPass (fs, hz, 0.7071067811865476));
}
} // namespace chioccia::dsp

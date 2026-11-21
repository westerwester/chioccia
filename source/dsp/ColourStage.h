// Chioccia — non-linear amplifier / transformer colouration (original code)
//
// Runs at the oversampled rate. Only the *non-linear residual* (shaped − linear) is DC-blocked and
// added back, so with Colour = 0 the stage is an exact identity and the linear part of the wet path
// keeps the same phase as the dry path.
//
//   tube stage:        asymmetric tanh curve (even + odd harmonics), knee above 0 dBFS at nominal colour
//   transformer stage: symmetric saturation of the low-frequency content (mostly 3rd harmonic,
//                      increasing as frequency decreases)
// Colour 0..1 : 0 = clean, 0.5 = nominal, 1 = +12 dB more drive into both curves (compensated).
#pragma once

#include "DspCommon.h"

namespace chioccia::dsp
{
struct ColourConfig
{
    double tubeHeadroom      = 1.6;    // linear level of the tube-curve knee (≈ +4 dBFS at Colour 50 %)
    double tubeBias          = 0.038;  // asymmetry (even harmonics)
    double xfmrHeadroom      = 1.2;    // low-frequency saturation knee
    double xfmrCornerHz      = 90.0;   // low-pass that extracts the "flux-relevant" low band
    double xfmrAmount        = 0.5;
    double dcBlockHz         = 8.0;    // high-pass applied to the residual only
};

class ColourStage
{
public:
    void prepare (double sampleRate, const ColourConfig& cfg) noexcept;
    void reset() noexcept { lf = 0.0; dc = 0.0; }

    /** Copies filter states (rate-independent meaning) from another instance, e.g. when switching
        oversampling factor, to avoid DC / LF steps. */
    void copyStateFrom (const ColourStage& other) noexcept { lf = other.lf; dc = other.dc; }

    /** amount: 0..1 crossfade of the residual (0 = identity). drive: linear drive into the curves. */
    inline float process (float input, float amount, float drive) noexcept
    {
        const double u = input;
        lf += (u - lf) * aLf;

        double residual = 0.0;
        if (amount > 0.0f)
        {
            // Tube: normalised asymmetric tanh, unity slope at the origin.
            const double x = u * invTubeHeadroom * drive;
            const double shaped = (std::tanh (x + cfg.tubeBias) - tanhBias) * invSlope;
            residual = (shaped - x) * cfg.tubeHeadroom / drive;

            // Transformer: symmetric saturation of the low band.
            const double z = lf * invXfmrHeadroom * drive;
            residual += cfg.xfmrAmount * (std::tanh (z) - z) * cfg.xfmrHeadroom / drive;

            residual *= amount;
        }

        dc += (residual - dc) * aDc;
        lf = flushTiny (lf);
        dc = flushTiny (dc);
        return (float) (u + residual - dc);
    }

private:
    ColourConfig cfg;
    double aLf = 0.0, aDc = 0.0;
    double invTubeHeadroom = 2.0, invXfmrHeadroom = 1.0, tanhBias = 0.0, invSlope = 1.0;
    double lf = 0.0, dc = 0.0;
};
} // namespace chioccia::dsp

// Chioccia — biquad (TDF-II, double precision) with RBJ-cookbook designs (original code)
#pragma once

#include "DspCommon.h"

namespace chioccia::dsp
{
struct BiquadCoeffs
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;

    static BiquadCoeffs highPass (double fs, double f0, double q) noexcept
    {
        f0 = std::clamp (f0, 1.0, 0.45 * fs);
        const double w0 = 2.0 * kPi * f0 / fs, cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q);
        const double a0 = 1.0 + alpha;
        BiquadCoeffs c;
        c.b0 = (1.0 + cw) * 0.5 / a0; c.b1 = -(1.0 + cw) / a0; c.b2 = c.b0;
        c.a1 = -2.0 * cw / a0;        c.a2 = (1.0 - alpha) / a0;
        return c;
    }

    static BiquadCoeffs highShelf (double fs, double f0, double gainDb, double slope = 1.0) noexcept
    {
        f0 = std::clamp (f0, 10.0, 0.45 * fs);
        const double A = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * f0 / fs, cw = std::cos (w0), sw = std::sin (w0);
        const double alpha = sw / 2.0 * std::sqrt ((A + 1.0 / A) * (1.0 / slope - 1.0) + 2.0);
        const double sA = 2.0 * std::sqrt (A) * alpha;
        const double a0 = (A + 1.0) - (A - 1.0) * cw + sA;
        BiquadCoeffs c;
        c.b0 = A * ((A + 1.0) + (A - 1.0) * cw + sA) / a0;
        c.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw) / a0;
        c.b2 = A * ((A + 1.0) + (A - 1.0) * cw - sA) / a0;
        c.a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cw) / a0;
        c.a2 = ((A + 1.0) - (A - 1.0) * cw - sA) / a0;
        return c;
    }
};

class Biquad
{
public:
    void setCoeffs (const BiquadCoeffs& c) noexcept { k = c; }
    void reset() noexcept { z1 = z2 = 0.0; }

    inline double process (double x) noexcept
    {
        const double y = k.b0 * x + z1;
        z1 = flushTiny (k.b1 * x - k.a1 * y + z2);
        z2 = flushTiny (k.b2 * x - k.a2 * y);
        return y;
    }

private:
    BiquadCoeffs k;
    double z1 = 0.0, z2 = 0.0;
};
} // namespace chioccia::dsp

// Chioccia — linear-phase half-band polyphase oversampling (original code)
#include "HalfBandOversampler.h"

#include <stdexcept>

namespace chioccia::dsp
{
namespace
{
double besselI0 (double x)
{
    // Power series; converges quickly for the beta values used here.
    double sum = 1.0, term = 1.0;
    const double q = x * x * 0.25;
    for (int k = 1; k < 200; ++k)
    {
        term *= q / ((double) k * (double) k);
        sum += term;
        if (term < sum * 1.0e-17) break;
    }
    return sum;
}

double kaiserBeta (double attenuationDb)
{
    if (attenuationDb > 50.0) return 0.1102 * (attenuationDb - 8.7);
    if (attenuationDb >= 21.0) return 0.5842 * std::pow (attenuationDb - 21.0, 0.4) + 0.07886 * (attenuationDb - 21.0);
    return 0.0;
}
} // namespace

std::vector<double> designHalfBand (int numTaps, double stopbandAttenuationDb)
{
    if (numTaps < 3 || (numTaps - 3) % 4 != 0)
        throw std::invalid_argument ("half-band length must be 4k + 3");

    const int c = (numTaps - 1) / 2;
    const double beta = kaiserBeta (stopbandAttenuationDb);
    const double i0Beta = besselI0 (beta);

    std::vector<double> h ((size_t) numTaps, 0.0);
    for (int n = 0; n < numTaps; ++n)
    {
        const int d = n - c;
        double ideal;
        if (d == 0)          ideal = 0.5;
        else if (d % 2 == 0) ideal = 0.0;                                  // exact half-band zeros
        else                 ideal = std::sin (kPi * d * 0.5) / (kPi * d);  // 0.5 * sinc(d / 2)

        const double r = (double) d / (double) c;
        const double w = besselI0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / i0Beta;
        h[(size_t) n] = ideal * w;
    }

    // Normalise DC gain to exactly 1 while keeping the centre tap at 0.5 (only odd taps scaled).
    double oddSum = 0.0;
    for (int n = 0; n < numTaps; ++n)
        if (n != c && ((n - c) % 2 != 0)) oddSum += h[(size_t) n];
    const double scale = 0.5 / oddSum;
    for (int n = 0; n < numTaps; ++n)
        if (n != c && ((n - c) % 2 != 0)) h[(size_t) n] *= scale;

    return h;
}

void HalfBandStage::prepare (const std::vector<double>& h)
{
    const int numTaps = (int) h.size();
    const int c = (numTaps - 1) / 2;
    k = (c - 1) / 2;
    numEven = (numTaps + 1) / 2;

    upCoeffs.resize ((size_t) numEven);
    downCoeffs.resize ((size_t) numEven);
    for (int j = 0; j < numEven; ++j)
    {
        upCoeffs[(size_t) j]   = (float) (2.0 * h[(size_t) (2 * j)]);
        downCoeffs[(size_t) j] = (float) h[(size_t) (2 * j)];
    }

    upHist.prepare (numEven);
    downA.prepare (numEven);
    downB.prepare (k + 2);
}

void Oversampler::prepare (int newFactor, const std::vector<double>& stage1, const std::vector<double>& stage2)
{
    factor = (newFactor >= 4 ? 4 : (newFactor >= 2 ? 2 : 1));
    s1.prepare (stage1);
    s2.prepare (stage2);

    if (factor == 1)      latency = 0;
    else if (factor == 2) latency = s1.getRoundTripLatency();
    else                  latency = s1.getRoundTripLatency() + (s2.getRoundTripLatency() + 1) / 2;

    reset();
}

void Oversampler::reset() noexcept
{
    s1.reset();
    s2.reset();
    pad2x = 0.0f;
}
} // namespace chioccia::dsp

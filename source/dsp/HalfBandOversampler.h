// Chioccia — linear-phase half-band polyphase oversampling (original code)
//
// Why linear phase: the wet path is summed with a delayed copy of the dry signal. A linear-phase
// FIR delays every frequency by the same amount, so after integer delay compensation the two paths
// are time-aligned across the passband (no comb filtering from oversampling filters).
//
// Half-band FIR, length N = 4k + 3, centre c = (N - 1) / 2 = 2k + 1 (odd).
// h[c] = 0.5 and h[c + d] = 0 for every even d != 0.
//   Upsampler (gain 2):   y[2n]   = 2 * sum_j h[2j] x[n - j]       (j = 0 .. (N - 1) / 2)
//                         y[2n+1] = x[n - k]                       (centre tap, pure delay)
//   Downsampler:          y[n]    = sum_j h[2j] a[n - j] + 0.5 b[n - k - 1]
//                         (a = even-phase input samples, b = odd-phase input samples)
// Up + down latency at the lower rate: c samples (integer).
#pragma once

#include <vector>
#include "DspCommon.h"

namespace chioccia::dsp
{
/** Kaiser-windowed half-band low-pass. numTaps must be 4k + 3. Not real-time safe (allocates). */
std::vector<double> designHalfBand (int numTaps, double stopbandAttenuationDb);

/** History buffer that returns a contiguous view of the last N samples (newest first). */
class FirHistory
{
public:
    void prepare (int length) { len = std::max (1, length); buf.assign ((size_t) (2 * len), 0.0f); pos = 0; }
    void reset() noexcept     { std::fill (buf.begin(), buf.end(), 0.0f); pos = 0; }

    inline void push (float x) noexcept
    {
        pos = (pos == 0 ? len - 1 : pos - 1);
        buf[(size_t) pos] = x;
        buf[(size_t) (pos + len)] = x;
    }

    /** data()[j] is the sample pushed j calls ago. */
    inline const float* data() const noexcept { return buf.data() + pos; }

private:
    std::vector<float> buf { 0.0f, 0.0f };
    int len = 1, pos = 0;
};

class HalfBandStage
{
public:
    void prepare (const std::vector<double>& h);
    void reset() noexcept { upHist.reset(); downA.reset(); downB.reset(); }

    /** Latency of up + down through this stage, in samples at the lower rate. */
    int getRoundTripLatency() const noexcept { return 2 * k + 1; }

    inline void up (float x, float& outEven, float& outOdd) noexcept
    {
        upHist.push (x);
        const float* d = upHist.data();
        float acc = 0.0f;
        for (int j = 0; j < numEven; ++j) acc += upCoeffs[(size_t) j] * d[j];
        outEven = acc;
        outOdd  = d[k];
    }

    inline float down (float even, float odd) noexcept
    {
        downA.push (even);
        downB.push (odd);
        const float* a = downA.data();
        float acc = 0.0f;
        for (int j = 0; j < numEven; ++j) acc += downCoeffs[(size_t) j] * a[j];
        return acc + 0.5f * downB.data()[k + 1];
    }

private:
    std::vector<float> upCoeffs, downCoeffs;
    FirHistory upHist, downA, downB;
    int numEven = 1, k = 0;
};

/** 1x / 2x / 4x oversampler with an integer, constant latency at the base rate. */
class Oversampler
{
public:
    /** factor must be 1, 2 or 4. Allocates; call from prepare only. */
    void prepare (int factor, const std::vector<double>& stage1, const std::vector<double>& stage2);
    void reset() noexcept;

    int getFactor() const noexcept  { return factor; }
    int getLatency() const noexcept { return latency; }

    /** Runs one base-rate sample through up-sampling, fn (called factor times), down-sampling. */
    template <typename Fn>
    inline float process (float x, Fn&& fn) noexcept
    {
        if (factor == 1)
            return fn (x);

        float e, o;
        s1.up (x, e, o);

        if (factor == 2)
            return s1.down (fn (e), fn (o));

        float a0, a1, a2, a3;
        s2.up (e, a0, a1);
        s2.up (o, a2, a3);
        a0 = fn (a0); a1 = fn (a1); a2 = fn (a2); a3 = fn (a3);
        const float d0 = s2.down (a0, a1);
        const float d1 = s2.down (a2, a3);
        // One-sample delay at 2x makes the total latency an integer number of base samples.
        const float out = s1.down (pad2x, d0);
        pad2x = d1;
        return out;
    }

private:
    HalfBandStage s1, s2;
    int factor = 1, latency = 0;
    float pad2x = 0.0f;
};
} // namespace chioccia::dsp

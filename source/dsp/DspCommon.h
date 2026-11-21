// Chioccia — opto leveling amplifier (original code)
// SPDX-License-Identifier: see LICENSE (project) — independent of any third-party product.
//
// Small helpers shared by the JUCE-independent DSP core.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace chioccia::dsp
{
constexpr double kPi = 3.14159265358979323846;

/** Digital reference level used across the project: 0 VU (+4 dBu) == -18 dBFS (sine RMS). */
constexpr float kZeroVuDbfs = -18.0f;

inline float dbToGain (float db) noexcept           { return std::pow (10.0f, db * 0.05f); }
inline double dbToGain (double db) noexcept         { return std::pow (10.0, db * 0.05); }
inline float gainToDb (float g, float floorDb = -144.0f) noexcept
{
    return g > 0.0f ? std::max (floorDb, 20.0f * std::log10 (g)) : floorDb;
}

/** Replaces NaN / Inf with 0. Never compiled with -ffast-math (see CMakeLists). */
inline float sanitize (float x) noexcept            { return std::isfinite (x) ? x : 0.0f; }

/** Input protection: NaN/Inf -> 0 and a hard bound at +60 dBFS (1000.0). This is not a musical
    limiter: no legitimate audio reaches it; it only keeps internal states finite and recoverable. */
constexpr float kInputSafetyBound = 1000.0f;
inline float sanitizeInput (float x) noexcept
{
    return std::isfinite (x) ? std::clamp (x, -kInputSafetyBound, kInputSafetyBound) : 0.0f;
}

/** Explicit denormal flushing for long-decaying state variables (portable, no FPU flags). */
inline double flushTiny (double x) noexcept         { return (x > -1.0e-30 && x < 1.0e-30) ? 0.0 : x; }
inline float flushTiny (float x) noexcept           { return (x > -1.0e-25f && x < 1.0e-25f) ? 0.0f : x; }

/** One-pole coefficient for a time constant tau (seconds) at rate fs: y += a * (x - y). */
inline double onePoleCoeff (double tauSeconds, double fs) noexcept
{
    if (tauSeconds <= 0.0) return 1.0;
    return 1.0 - std::exp (-1.0 / (tauSeconds * fs));
}

/** Small, deterministic, allocation-free PRNG (xorshift32). */
struct XorShift32
{
    std::uint32_t state = 0x9E3779B9u;
    void seed (std::uint32_t s) noexcept { state = s != 0 ? s : 0x9E3779B9u; }
    inline float nextBipolar() noexcept
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return (float) ((double) state * (2.0 / 4294967295.0) - 1.0);
    }
};
} // namespace chioccia::dsp

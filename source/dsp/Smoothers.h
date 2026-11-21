// Chioccia — parameter smoothing (original code)
#pragma once

#include "DspCommon.h"

namespace chioccia::dsp
{
/** Linear ramp towards a target over a fixed time. Sample-accurate, allocation free. */
class LinearSmoother
{
public:
    void prepare (double sampleRate, double rampSeconds) noexcept
    {
        rampSteps = std::max (1, (int) std::lround (rampSeconds * sampleRate));
        countdown = 0;
        current = target;
    }

    void setCurrentAndTarget (float v) noexcept { current = target = v; countdown = 0; step = 0.0f; }

    void setTarget (float v) noexcept
    {
        if (! (v < target || v > target)) return;   // unchanged target: keep the running ramp
        target = v;
        countdown = rampSteps;
        step = (target - current) / (float) rampSteps;
    }

    inline float getNext() noexcept
    {
        if (countdown <= 0) return target;
        if (--countdown == 0) current = target;
        else current += step;
        return current;
    }

    float getCurrent() const noexcept  { return countdown > 0 ? current : target; }
    float getTarget() const noexcept   { return target; }
    bool isSmoothing() const noexcept  { return countdown > 0; }

private:
    float current = 0.0f, target = 0.0f, step = 0.0f;
    int countdown = 0, rampSteps = 1;
};

/** Smooths a value expressed in dB and returns linear gain; exp() is only evaluated while ramping. */
class DbGainSmoother
{
public:
    void prepare (double sampleRate, double rampSeconds) noexcept { db.prepare (sampleRate, rampSeconds); refresh(); }
    void setCurrentAndTargetDb (float v) noexcept               { db.setCurrentAndTarget (v); refresh(); }
    void setTargetDb (float v) noexcept                         { db.setTarget (v); }

    inline float getNextGain() noexcept
    {
        if (! db.isSmoothing()) return cachedGain;
        const float v = db.getNext();
        cachedGain = dbToGain (v);
        if (! db.isSmoothing()) refresh();
        return cachedGain;
    }

    float getTargetDb() const noexcept { return db.getTarget(); }
    /** Last value returned by getNextGain() (or the target gain when idle). */
    float getCurrentGain() const noexcept { return cachedGain; }
    bool isSmoothing() const noexcept { return db.isSmoothing(); }

private:
    void refresh() noexcept { cachedGain = dbToGain (db.getTarget()); }
    LinearSmoother db;
    float cachedGain = 1.0f;
};
} // namespace chioccia::dsp

// Chioccia — integer-sample delay line (original code)
#pragma once

#include <vector>
#include "DspCommon.h"

namespace chioccia::dsp
{
/** Pure integer delay. Memory is allocated in prepare() only; process() is real-time safe. */
class IntegerDelay
{
public:
    void prepare (int maxDelaySamples)
    {
        size = 1;
        while (size < maxDelaySamples + 1) size <<= 1;
        buffer.assign ((size_t) size, 0.0f);
        mask = size - 1;
        writePos = 0;
        delay = std::min (delay, maxDelaySamples);
    }

    void setDelay (int samples) noexcept { delay = std::clamp (samples, 0, mask); }
    int getDelay() const noexcept        { return delay; }

    void reset() noexcept { std::fill (buffer.begin(), buffer.end(), 0.0f); writePos = 0; }

    inline float process (float x) noexcept
    {
        if (delay == 0) return x;
        buffer[(size_t) writePos] = x;
        const float y = buffer[(size_t) ((writePos - delay) & mask)];
        writePos = (writePos + 1) & mask;
        return y;
    }

private:
    std::vector<float> buffer { 0.0f };
    int size = 1, mask = 0, writePos = 0, delay = 0;
};
} // namespace chioccia::dsp

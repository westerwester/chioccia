// Chioccia — colouration stage (original code)
#include "ColourStage.h"

namespace chioccia::dsp
{
void ColourStage::prepare (double sampleRate, const ColourConfig& config) noexcept
{
    cfg = config;
    aLf = 1.0 - std::exp (-2.0 * kPi * cfg.xfmrCornerHz / sampleRate);
    aDc = 1.0 - std::exp (-2.0 * kPi * cfg.dcBlockHz / sampleRate);
    invTubeHeadroom = 1.0 / cfg.tubeHeadroom;
    invXfmrHeadroom = 1.0 / cfg.xfmrHeadroom;
    tanhBias = std::tanh (cfg.tubeBias);
    invSlope = 1.0 / (1.0 - tanhBias * tanhBias);
    reset();
}
} // namespace chioccia::dsp

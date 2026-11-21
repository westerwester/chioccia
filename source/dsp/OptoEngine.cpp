// Chioccia — processing engine (original code)
#include "OptoEngine.h"

namespace chioccia::dsp
{
namespace
{
constexpr int kQualityFactors[OptoEngine::kNumQualities] = { 1, 2, 4 };
} // namespace

OptoEngine::OptoEngine()
{
    prepare (48000.0, 2);
}

void OptoEngine::prepare (double sampleRate, int numChannels)
{
    fs = sampleRate > 1000.0 ? sampleRate : 48000.0;
    numChannelsPrepared = std::clamp (numChannels, 1, kMaxChannels);

    const auto h1 = designHalfBand (kStage1Taps, kStopbandDb);
    const auto h2 = designHalfBand (kStage2Taps, kStopbandDb);

    for (int q = 0; q < kNumQualities; ++q)
    {
        Oversampler probe;
        probe.prepare (kQualityFactors[q], h1, h2);
        pathLatency[(size_t) q] = probe.getLatency();
    }
    totalLatency = *std::max_element (pathLatency.begin(), pathLatency.end());

    for (int c = 0; c < kMaxChannels; ++c)
    {
        auto& ch = chans[(size_t) c];
        for (int q = 0; q < kNumQualities; ++q)
        {
            auto& p = ch.paths[(size_t) q];
            p.os.prepare (kQualityFactors[q], h1, h2);
            p.colour.prepare (fs * kQualityFactors[q], colourConfig);
            p.pad.prepare (totalLatency);
            p.pad.setDelay (totalLatency - pathLatency[(size_t) q]);
        }
        ch.dryDelay.prepare (totalLatency);
        ch.dryDelay.setDelay (totalLatency);
        ch.bypassDelay.prepare (totalLatency);
        ch.bypassDelay.setDelay (totalLatency);
        ch.detector.prepare (fs, detectorConfig);
        ch.cell.prepare (fs, cellConfig);
        ch.noise.seed (0x1234567u + 0x9E3779B9u * (std::uint32_t) (c + 1));
    }

    sInputTrim.prepare (fs, 0.02);   sMakeup.prepare (fs, 0.02);
    sWetTrim.prepare (fs, 0.02);     sOutputTrim.prepare (fs, 0.02);
    sDrive.prepare (fs, 0.03);       sColourDrive.prepare (fs, 0.05);
    sDriveEnable.prepare (fs, 0.03); sMix.prepare (fs, 0.03);
    sLink.prepare (fs, 0.05);        sExponent.prepare (fs, 0.05);
    sColourAmount.prepare (fs, 0.05); sLawA.prepare (fs, 0.03);
    sHpfLog2.prepare (fs, 0.05);
    sListenMix.prepare (fs, 0.02);   sListenDry.prepare (fs, 0.02);  sListenWet.prepare (fs, 0.02);
    sBypass.prepare (fs, 0.02);      sCompare.prepare (fs, 0.05);
    sNoise.prepare (fs, 0.05);       sDiagGain.prepare (fs, 0.005);

    xfadeLength  = std::max (1, (int) std::lround (0.02 * fs));
    aComparePow  = onePoleCoeff (1.5, fs);
    aCompareGain = onePoleCoeff (0.3, fs);

    applyParameters (lastParams, true);
    reset();
}

void OptoEngine::reset() noexcept
{
    for (auto& ch : chans)
    {
        for (auto& p : ch.paths) { p.os.reset(); p.colour.reset(); p.pad.reset(); }
        ch.dryDelay.reset();
        ch.bypassDelay.reset();
        ch.detector.reset();
        ch.cell.reset();
        ch.history.fill (0.0f);
        ch.feedback = 0.0;
    }
    historyPos = 0;
    humPhase = 0.0;
    compareInPow = compareOutPow = 0.0;
    compareGain = 1.0;
    xfadeRemaining = 0;
    fadingQuality = -1;
    activeQuality = requestedQuality;
    activeDiagnostic = requestedDiagnostic;
    sDiagGain.setCurrentAndTarget (1.0f);
    meterGrDb = 0.0f; meterOutDb = -144.0f; meterInDb = -144.0f; meterCompareDb = 0.0f;
}

void OptoEngine::setParameters (const EngineParameters& p) noexcept
{
    lastParams = p;
    applyParameters (p, false);
}

void OptoEngine::applyParameters (const EngineParameters& p, bool immediate) noexcept
{
    auto setDb = [immediate] (DbGainSmoother& s, float v) { immediate ? s.setCurrentAndTargetDb (v) : s.setTargetDb (v); };
    auto setLin = [immediate] (LinearSmoother& s, float v) { immediate ? s.setCurrentAndTarget (v) : s.setTarget (v); };

    const float pr = std::clamp (p.peakReduction, 0.0f, 100.0f);
    setDb (sInputTrim, std::clamp (p.inputTrimDb, -24.0f, 24.0f));
    setDb (sMakeup, mapping::gainKnobToDb (std::clamp (p.gain, 0.0f, 100.0f)));
    setDb (sWetTrim, std::clamp (p.wetTrimDb, -24.0f, 24.0f));
    setDb (sOutputTrim, std::clamp (p.outputTrimDb, -24.0f, 24.0f));
    setDb (sDrive, mapping::peakReductionToDriveDb (pr));
    setLin (sDriveEnable, mapping::peakReductionEnable (pr));

    const float colour = std::clamp (p.colourPercent, 0.0f, 100.0f);
    setDb (sColourDrive, mapping::colourToDriveDb (colour));
    setLin (sColourAmount, mapping::colourToAmount (colour));

    setLin (sMix, std::clamp (p.mixPercent, 0.0f, 100.0f) * 0.01f);
    setLin (sLink, std::clamp (p.stereoLinkPercent, 0.0f, 100.0f) * 0.01f);
    setLin (sExponent, p.limitMode ? mapping::kExponentLimit : mapping::kExponentCompress);
    setLin (sLawA, p.mixLaw == MixLaw::linearCrossfade ? 1.0f : 0.0f);

    const float hpfHz = p.detectorHpfOn ? std::clamp (p.detectorHpfHz, 20.0f, 500.0f) : (float) detectorConfig.dcBlockHz;
    setLin (sHpfLog2, std::log2 (hpfHz));

    setLin (sListenMix, p.listen == Listen::mix ? 1.0f : 0.0f);
    setLin (sListenDry, p.listen == Listen::dryOnly ? 1.0f : 0.0f);
    setLin (sListenWet, p.listen == Listen::wetOnly ? 1.0f : 0.0f);
    setLin (sBypass, p.bypass ? 1.0f : 0.0f);
    setLin (sCompare, p.levelMatchedCompare ? 1.0f : 0.0f);
    setLin (sNoise, std::clamp (p.noisePercent, 0.0f, 100.0f) * 0.01f);
    humHz = p.humHz == 60 ? 60 : 50;

    requestedQuality = std::clamp ((int) p.quality, 0, kNumQualities - 1);
    requestedDiagnostic = std::clamp ((int) p.diagnostic, 0, 2);

    if (immediate)
    {
        activeQuality = requestedQuality;
        activeDiagnostic = requestedDiagnostic;
        for (auto& ch : chans)
            ch.detector.setHighPass (std::exp2 (sHpfLog2.getTarget()));
    }
}

float OptoEngine::runPath (Channel& ch, int q, float u, float amount, float colourDrive, bool identity) noexcept
{
    auto& p = ch.paths[(size_t) q];
    float v;
    if (identity)
    {
        v = p.os.process (u, [] (float s) noexcept { return s; });
    }
    else
    {
        auto& colour = p.colour;
        v = p.os.process (u, [&colour, amount, colourDrive] (float s) noexcept { return colour.process (s, amount, colourDrive); });
    }
    return p.pad.process (v);
}

void OptoEngine::startQualityChange (int newQuality) noexcept
{
    fadingQuality = activeQuality;
    activeQuality = newQuality;
    xfadeRemaining = xfadeLength;

    const bool identity = activeDiagnostic != 0;
    const float amount = sColourAmount.getCurrent();
    const float colourDrive = sColourDrive.getCurrentGain();

    for (int c = 0; c < numChannelsPrepared; ++c)
    {
        auto& ch = chans[(size_t) c];
        auto& incoming = ch.paths[(size_t) newQuality];
        incoming.os.reset();
        incoming.pad.reset();
        incoming.colour.copyStateFrom (ch.paths[(size_t) fadingQuality].colour);

        // Warm-up: replay recent wet-path input so filters and the padding delay hold the same
        // history as the outgoing path. The crossfade then mixes two coherent signals.
        for (int i = 0; i < kPrimeLength; ++i)
            (void) runPath (ch, newQuality, ch.history[(size_t) ((historyPos + i) % kPrimeLength)], amount, colourDrive, identity);
    }
}

void OptoEngine::process (float* const* channels, int numChannels, int numSamples) noexcept
{
    if (numSamples <= 0 || channels == nullptr) return;
    const int nch = std::clamp (std::min (numChannels, numChannelsPrepared), 0, kMaxChannels);
    if (nch == 0) return;

    if (requestedQuality != activeQuality && xfadeRemaining == 0)
        startQualityChange (requestedQuality);

    sDiagGain.setTarget (requestedDiagnostic != activeDiagnostic ? 0.0f : 1.0f);

    const float noiseScale = dbToGain (mapping::kNoiseFullScaleDb) * 1.7320508f;   // uniform → RMS
    const float humScale   = dbToGain (mapping::kHumFullScaleDb) * 1.4142136f;     // sine → RMS
    const double humInc    = (double) humHz / fs;

    float minGain = 1.0f;
    double sumIn = 0.0, sumOut = 0.0;

    for (int n = 0; n < numSamples; ++n)
    {
        const float inTrim   = sInputTrim.getNextGain();
        const float makeup   = sMakeup.getNextGain();
        const float wetTrim  = sWetTrim.getNextGain();
        const float outTrim  = sOutputTrim.getNextGain();
        const double drive   = (double) sDrive.getNextGain() * (double) sDriveEnable.getNext();
        const double expo    = sExponent.getNext();
        const float mix      = sMix.getNext();
        const float link     = sLink.getNext();
        const float amount   = sColourAmount.getNext();
        const float colDrive = sColourDrive.getNextGain();
        const float lawA     = sLawA.getNext();
        const float wMix     = sListenMix.getNext();
        const float wDry     = sListenDry.getNext();
        const float wWet     = sListenWet.getNext();
        const float bypassW  = sBypass.getNext();
        const float compareW = sCompare.getNext();
        const float noiseAmt = sNoise.getNext();
        const float diagGain = sDiagGain.getNext();

        if (sHpfLog2.isSmoothing())
        {
            const float v = sHpfLog2.getNext();
            if (++hpfUpdateCounter >= 16 || ! sHpfLog2.isSmoothing())
            {
                hpfUpdateCounter = 0;
                for (int c = 0; c < nch; ++c) chans[(size_t) c].detector.setHighPass (std::exp2 (v));
            }
        }

        if (activeDiagnostic != requestedDiagnostic && diagGain <= 0.0f)
        {
            activeDiagnostic = requestedDiagnostic;
            sDiagGain.setTarget (1.0f);
        }
        const bool identity = activeDiagnostic != 0;

        // --- detector + optical cell, per channel ----------------------------------------------------
        float raw[kMaxChannels] {}, x[kMaxChannels] {}, g[kMaxChannels] {};
        for (int c = 0; c < nch; ++c)
        {
            auto& ch = chans[(size_t) c];
            raw[c] = sanitizeInput (channels[c][n]);
            x[c] = raw[c] * inTrim;
            const double light = ch.detector.process (ch.feedback, drive, expo);
            g[c] = (float) ch.cell.process (light, expo);
        }

        // --- stereo link in the gain domain: at 100 % both channels get exactly the same gain -------
        if (nch == 2)
        {
            const float gMin = std::min (g[0], g[1]);
            if (link >= 1.0f) { g[0] = g[1] = gMin; }
            else { g[0] += link * (gMin - g[0]); g[1] += link * (gMin - g[1]); }
        }

        const double hum = noiseAmt > 0.0f
            ? std::sin (2.0 * kPi * humPhase) + 0.5 * std::sin (6.0 * kPi * humPhase) : 0.0;

        double inPow = 0.0, outPowPreCompare = 0.0;
        for (int c = 0; c < nch; ++c)
        {
            auto& ch = chans[(size_t) c];
            minGain = std::min (minGain, g[c]);

            const float y = x[c] * g[c];
            ch.feedback = y;
            const float u = identity ? x[c] : y * makeup;
            ch.history[(size_t) historyPos] = u;

            float wet = runPath (ch, activeQuality, u, amount, colDrive, identity);
            if (xfadeRemaining > 0)
            {
                const float wOld = (float) xfadeRemaining / (float) xfadeLength;
                const float old = runPath (ch, fadingQuality, u, amount, colDrive, identity);
                wet += wOld * (old - wet);
            }

            if (! identity)
            {
                if (noiseAmt > 0.0f)
                    wet += noiseAmt * (noiseScale * ch.noise.nextBipolar() + humScale * (float) hum);
                wet *= wetTrim;
            }

            const float dry = ch.dryDelay.process (x[c]);
            float out;
            if (activeDiagnostic == (int) Diagnostic::nullResidual)      out = dry - wet;
            else if (activeDiagnostic == (int) Diagnostic::wetIdentity)  out = wet;
            else
            {
                const float mixed = dry * (1.0f - lawA * mix) + mix * wet;
                out = (wMix * mixed + wDry * dry + wWet * wet) * outTrim;
            }
            out *= diagGain;

            inPow += (double) raw[c] * raw[c];
            outPowPreCompare += (double) out * out;
            out *= 1.0f + compareW * ((float) compareGain - 1.0f);

            const float bypassed = ch.bypassDelay.process (raw[c]);
            if (bypassW >= 1.0f)     out = bypassed;               // exact pass-through when fully bypassed
            else if (bypassW > 0.0f) out = out * (1.0f - bypassW) + bypassW * bypassed;

            out = sanitize (out);
            channels[c][n] = out;
            sumIn += (double) raw[c] * raw[c];
            sumOut += (double) out * out;
        }

        // --- level-matched compare (explicit, opt-in monitoring aid) ----------------------------------
        inPow /= nch;
        outPowPreCompare /= nch;
        compareInPow  = flushTiny (compareInPow + (inPow - compareInPow) * aComparePow);
        compareOutPow = flushTiny (compareOutPow + (outPowPreCompare - compareOutPow) * aComparePow);
        if (compareInPow > 1.0e-7 && compareOutPow > 1.0e-12)
        {
            const double target = std::clamp (std::sqrt (compareInPow / compareOutPow), 1.0 / 15.85, 15.85);
            compareGain += (target - compareGain) * aCompareGain;
        }

        historyPos = (historyPos + 1) % kPrimeLength;
        if (noiseAmt > 0.0f) { humPhase += humInc; if (humPhase >= 1.0) humPhase -= 1.0; }
        if (xfadeRemaining > 0 && --xfadeRemaining == 0) fadingQuality = -1;
    }

    const double denom = (double) numSamples * nch;
    meterGrDb.store (-gainToDb (minGain), std::memory_order_relaxed);
    meterInDb.store ((float) (10.0 * std::log10 (sumIn / denom + 1.0e-30)), std::memory_order_relaxed);
    meterOutDb.store ((float) (10.0 * std::log10 (sumOut / denom + 1.0e-30)), std::memory_order_relaxed);
    meterCompareDb.store (gainToDb ((float) compareGain), std::memory_order_relaxed);
}
} // namespace chioccia::dsp

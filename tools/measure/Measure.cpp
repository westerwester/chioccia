// Chioccia — measurement tool for the DSP model (original code)
//
// Produces CSV files and a Markdown summary with *measured* properties of this implementation:
// static curves, attack/release, THD, frequency response, detector frequency dependence, aliasing.
// These numbers describe Chioccia only. They are NOT a comparison with any hardware or third-party
// plug-in (see docs/06-validazione.md for the comparison procedure).
//
// Usage: chioccia_measure <output-dir> [sampleRate]
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "../../tests/core/TestUtils.h"

using namespace chioccia;
using namespace chioccia::testutil;
using dsp::EngineParameters;
using dsp::OptoEngine;

namespace
{
double gFs = 48000.0;
std::ofstream md;

EngineParameters base()
{
    EngineParameters p;
    p.gain = 50.0f; p.colourPercent = 0.0f; p.mixPercent = 100.0f; p.quality = dsp::Quality::x2;
    return p;
}

Buffer process (EngineParameters p, const Buffer& in, int blockSize = 256)
{
    OptoEngine e;
    e.setParameters (p);
    e.prepare (gFs, 1);
    return run (e, in, {}, blockSize).l;
}

/** Per-sample gain reduction (dB) trace, obtained with block size 1. */
std::vector<float> grTrace (EngineParameters p, const Buffer& in)
{
    OptoEngine e;
    e.setParameters (p);
    e.prepare (gFs, 1);
    std::vector<float> gr (in.size());
    Buffer tmp (in);
    for (size_t i = 0; i < in.size(); ++i)
    {
        float* ptr = tmp.data() + i;
        e.process (&ptr, 1, 1);
        gr[i] = e.getGainReductionDb();
    }
    return gr;
}

std::string f2 (double v, int prec = 2)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision (prec) << v;
    return os.str();
}

// ------------------------------------------------------------------------------------------------
void staticCurves (const std::string& dir)
{
    std::ofstream csv (dir + "/static_curves.csv");
    csv << "mode,pr,in_dbfs_rms,out_dbfs_rms,gr_db\n";
    md << "\n## Curve statiche (sinusoide 1 kHz, Gain 50, Colour 0, Mix 100 %)\n\n"
       << "GR in dB misurata sull'RMS dell'ultimo secondo di 3 s di segnale. 0 VU = −18 dBFS RMS.\n\n"
       << "| Modalità | PR | GR @ −30 | GR @ −18 (0 VU) | GR @ −12 | GR @ −6 | GR @ 0 dBFS | rapporto effettivo (ultimi 6 dB) |\n"
       << "|---|---|---|---|---|---|---|---|\n";
    for (bool limit : { false, true })
    {
        for (float pr : { 20.0f, 40.0f, 50.0f, 60.0f, 80.0f, 100.0f })
        {
            std::map<int, double> gr;
            std::map<int, double> outLvl;
            for (int lvl = -40; lvl <= 0; lvl += 2)
            {
                auto p = base();
                p.peakReduction = pr; p.limitMode = limit;
                auto in = sine (gFs, 1000.0, rmsDbToSinePeak (lvl), (int) (3 * gFs));
                auto out = process (p, in);
                const double o = rmsDb (out, (size_t) (2 * gFs));
                gr[lvl] = lvl - o;
                outLvl[lvl] = o;
                csv << (limit ? "limit" : "compress") << "," << pr << "," << lvl << "," << o << "," << (lvl - o) << "\n";
            }
            const double ratio = 6.0 / std::max (1e-6, outLvl[0] - outLvl[-6]);
            md << "| " << (limit ? "Limit" : "Compress") << " | " << pr << " | " << f2 (gr[-30]) << " | " << f2 (gr[-18]) << " | "
               << f2 (gr[-12]) << " | " << f2 (gr[-6]) << " | " << f2 (gr[0]) << " | " << f2 (ratio, 1) << ":1 |\n";
        }
    }
}

double timeToFraction (const std::vector<float>& gr, size_t from, double startVal, double endVal, double fraction)
{
    const double target = startVal + fraction * (endVal - startVal);
    for (size_t i = from; i < gr.size(); ++i)
    {
        if ((endVal > startVal && gr[i] >= target) || (endVal <= startVal && gr[i] <= target))
            return (double) (i - from) / gFs;
    }
    return -1.0;
}

void attackRelease (const std::string& dir)
{
    md << "\n## Attacco (gradino da −40 dBFS a livello L, sinusoide 1 kHz)\n\n"
       << "Tempo perché la GR (in dB) raggiunga il 63 % e il 90 % del valore finale.\n\n"
       << "| Modalità | PR | L (dBFS RMS) | GR finale | t63 | t90 |\n|---|---|---|---|---|---|\n";
    std::ofstream csv (dir + "/attack.csv");
    csv << "mode,pr,level,t,gr\n";
    for (bool limit : { false, true })
        for (int lvl : { -18, -12, -6 })
        {
            auto p = base();
            p.peakReduction = 60.0f; p.limitMode = limit;
            const int pre = (int) gFs, post = (int) (1.5 * gFs);
            Buffer in = sine (gFs, 1000.0, dbfsToPeak (-40.0), pre + post);
            const double a = rmsDbToSinePeak (lvl);
            for (int i = pre; i < pre + post; ++i) in[(size_t) i] = (float) (a * std::sin (2.0 * kPi * 1000.0 * i / gFs));
            auto gr = grTrace (p, in);
            const double start = gr[(size_t) pre - 1], fin = gr.back();
            const double t63 = timeToFraction (gr, (size_t) pre, start, fin, 0.632);
            const double t90 = timeToFraction (gr, (size_t) pre, start, fin, 0.9);
            md << "| " << (limit ? "Limit" : "Compress") << " | 60 | " << lvl << " | " << f2 (fin) << " dB | "
               << f2 (t63 * 1000.0, 1) << " ms | " << f2 (t90 * 1000.0, 1) << " ms |\n";
            for (size_t i = (size_t) pre - 480; i < (size_t) pre + (size_t) (0.2 * gFs); i += 24)
                csv << (limit ? "limit" : "compress") << "," << 60 << "," << lvl << "," << (double) ((long) i - pre) / gFs << "," << gr[i] << "\n";
        }

    md << "\n## Rilascio in funzione della storia (PR 60, Compress, burst a −8 dBFS RMS poi −80 dBFS)\n\n"
       << "| Durata burst | GR al rilascio | t al 50 % | t al 10 % | t a GR < 0,5 dB |\n|---|---|---|---|---|\n";
    std::ofstream csvR (dir + "/release.csv");
    csvR << "burst_s,t,gr\n";
    for (double burst : { 0.1, 0.5, 2.0, 10.0, 30.0 })
    {
        auto p = base();
        p.peakReduction = 60.0f;
        const int nb = (int) (burst * gFs), tail = (int) (25.0 * gFs);
        Buffer in ((size_t) (nb + tail));
        const double a = rmsDbToSinePeak (-8.0), q = dbfsToPeak (-80.0);
        for (int i = 0; i < nb + tail; ++i) in[(size_t) i] = (float) ((i < nb ? a : q) * std::sin (2.0 * kPi * 1000.0 * i / gFs));
        auto gr = grTrace (p, in);
        const double start = gr[(size_t) nb - 1];
        const double t50 = timeToFraction (gr, (size_t) nb, start, 0.0, 0.5);
        const double t10 = timeToFraction (gr, (size_t) nb, start, 0.0, 0.9);
        double tHalfDb = -1.0;
        for (size_t i = (size_t) nb; i < gr.size(); ++i) if (gr[i] < 0.5f) { tHalfDb = (double) (i - nb) / gFs; break; }
        md << "| " << f2 (burst, 1) << " s | " << f2 (start) << " dB | " << f2 (t50 * 1000.0, 0) << " ms | "
           << (t10 < 0 ? std::string ("> 25 s") : f2 (t10, 2) + " s") << " | " << (tHalfDb < 0 ? std::string ("> 25 s") : f2 (tHalfDb, 2) + " s") << " |\n";
        for (size_t i = (size_t) nb; i < gr.size(); i += 240)
            csvR << burst << "," << (double) (i - nb) / gFs << "," << gr[i] << "\n";
    }
}

double thdPercent (const Buffer& out, double f0, size_t start, size_t len, double* h2 = nullptr, double* h3 = nullptr)
{
    const double a1 = toneAmplitude (out, start, len, gFs, f0);
    double s = 0.0;
    for (int k = 2; k <= 9; ++k)
    {
        if (k * f0 >= gFs * 0.5) break;
        const double ak = toneAmplitude (out, start, len, gFs, k * f0);
        if (k == 2 && h2) *h2 = 20.0 * std::log10 (ak / a1 + 1e-30);
        if (k == 3 && h3) *h3 = 20.0 * std::log10 (ak / a1 + 1e-30);
        s += ak * ak;
    }
    return 100.0 * std::sqrt (s) / a1;
}

void distortion (const std::string& dir)
{
    md << "\n## Distorsione armonica (PR 0 = nessuna compressione, Gain 50, qualità 2x)\n\n"
       << "THD = H2…H9 / H1. Livelli in dBFS di picco.\n\n"
       << "| f | Colour | −30 dBFS | −18 dBFS | −12 dBFS | −6 dBFS | −1 dBFS | H2/H3 a −6 dBFS |\n|---|---|---|---|---|---|---|---|\n";
    std::ofstream csv (dir + "/thd.csv");
    csv << "freq,colour,level_dbfs_peak,thd_percent,h2_db,h3_db\n";
    for (double f : { 1000.0, 100.0, 50.0 })
        for (float colour : { 0.0f, 25.0f, 50.0f, 75.0f, 100.0f })
        {
            md << "| " << f2 (f, 0) << " Hz | " << colour << " % ";
            std::string h23;
            for (double lvl : { -30.0, -18.0, -12.0, -6.0, -1.0 })
            {
                auto p = base();
                p.peakReduction = 0.0f; p.colourPercent = colour;
                const int n = (int) (2 * gFs);
                auto in = sine (gFs, f, dbfsToPeak (lvl), n);
                auto out = process (p, in);
                double h2 = -200, h3 = -200;
                const double thd = thdPercent (out, f, (size_t) gFs, (size_t) gFs, &h2, &h3);
                csv << f << "," << colour << "," << lvl << "," << thd << "," << h2 << "," << h3 << "\n";
                md << "| " << (thd < 0.001 ? std::string ("< 0.001 %") : f2 (thd, 3) + " %") << " ";
                if (lvl == -6.0) h23 = f2 (h2, 0) + " / " + f2 (h3, 0) + " dB";
            }
            md << "| " << h23 << " |\n";
        }

    md << "\n### Distorsione da modulazione del guadagno (PR 60, Colour 0, −12 dBFS RMS)\n\n"
       << "Misura il ripple del controllo a basse frequenze (comportamento tipico dei compressori ottici).\n\n"
       << "| f | Compress THD | Limit THD |\n|---|---|---|\n";
    for (double f : { 50.0, 100.0, 1000.0 })
    {
        md << "| " << f2 (f, 0) << " Hz ";
        for (bool limit : { false, true })
        {
            auto p = base();
            p.peakReduction = 60.0f; p.limitMode = limit;
            auto in = sine (gFs, f, rmsDbToSinePeak (-12.0), (int) (4 * gFs));
            auto out = process (p, in);
            md << "| " << f2 (thdPercent (out, f, (size_t) (3 * gFs), (size_t) gFs), 3) << " % ";
        }
        md << "|\n";
    }
}

void frequencyResponse (const std::string& dir)
{
    md << "\n## Risposta in frequenza del percorso wet (PR 0, −30 dBFS, relativa a 1 kHz)\n\n"
       << "| Qualità | Colour | 20 Hz | 50 Hz | 100 Hz | 10 kHz | 18 kHz | 20 kHz | 21 kHz |\n|---|---|---|---|---|---|---|---|---|\n";
    std::ofstream csv (dir + "/freq_response.csv");
    csv << "quality,colour,freq,gain_db\n";
    const double freqs[] = { 20.0, 50.0, 100.0, 10000.0, 18000.0, 20000.0, 21000.0 };
    for (auto q : { dsp::Quality::x1, dsp::Quality::x2, dsp::Quality::x4 })
        for (float colour : { 0.0f, 100.0f })
        {
            auto measure = [&] (double f)
            {
                auto p = base();
                p.peakReduction = 0.0f; p.colourPercent = colour; p.quality = q;
                auto in = sine (gFs, f, dbfsToPeak (-30.0), (int) (2 * gFs));
                auto out = process (p, in);
                return 20.0 * std::log10 (toneAmplitude (out, (size_t) gFs, (size_t) gFs, gFs, f) / dbfsToPeak (-30.0));
            };
            const double ref = measure (1000.0);
            md << "| x" << (q == dsp::Quality::x1 ? 1 : q == dsp::Quality::x2 ? 2 : 4) << " | " << colour << " % ";
            for (double f : freqs)
            {
                const double g = measure (f) - ref;
                csv << (int) q << "," << colour << "," << f << "," << g << "\n";
                md << "| " << f2 (g, 3) << " ";
            }
            md << "|\n";
        }
}

void detectorFrequency()
{
    md << "\n## Dipendenza in frequenza del detector (PR 60, Compress, −12 dBFS RMS)\n\n"
       << "| f | GR HPF off | GR HPF 100 Hz |\n|---|---|---|\n";
    for (double f : { 40.0, 80.0, 150.0, 500.0, 1000.0, 3000.0, 8000.0 })
    {
        md << "| " << f2 (f, 0) << " Hz ";
        for (bool hpf : { false, true })
        {
            auto p = base();
            p.peakReduction = 60.0f; p.detectorHpfOn = hpf; p.detectorHpfHz = 100.0f;
            auto in = sine (gFs, f, rmsDbToSinePeak (-12.0), (int) (3 * gFs));
            auto out = process (p, in);
            md << "| " << f2 (-12.0 - rmsDb (out, (size_t) (2 * gFs))) << " dB ";
        }
        md << "|\n";
    }
}

void aliasing()
{
    md << "\n## Aliasing (sinusoide 15 kHz a −3 dBFS, Colour 100 %, PR 0)\n\n"
       << "Somma della potenza dei prodotti ripiegati sotto 20 kHz (armoniche 2…12 di 15 kHz), relativa alla fondamentale.\n\n"
       << "| Qualità | aliasing |\n|---|---|\n";
    for (auto q : { dsp::Quality::x1, dsp::Quality::x2, dsp::Quality::x4 })
    {
        auto p = base();
        p.peakReduction = 0.0f; p.colourPercent = 100.0f; p.quality = q;
        const double f0 = 15000.0;
        auto in = sine (gFs, f0, dbfsToPeak (-3.0), (int) (2 * gFs));
        auto out = process (p, in);
        const double a1 = toneAmplitude (out, (size_t) gFs, (size_t) gFs, gFs, f0);
        double s = 0.0;
        for (int k = 2; k <= 12; ++k)
        {
            double f = std::fmod (k * f0, gFs);
            if (f > gFs * 0.5) f = gFs - f;
            if (f < 20000.0 && f > 10.0) { const double a = toneAmplitude (out, (size_t) gFs, (size_t) gFs, gFs, f); s += a * a; }
        }
        md << "| x" << (q == dsp::Quality::x1 ? 1 : q == dsp::Quality::x2 ? 2 : 4) << " | " << f2 (10.0 * std::log10 (s / (a1 * a1) + 1e-30), 1) << " dB |\n";
    }
}
} // namespace

int main (int argc, char** argv)
{
    const std::string dir = argc > 1 ? argv[1] : "misure";
    if (argc > 2) gFs = std::atof (argv[2]);
    std::filesystem::create_directories (dir);
    md.open (dir + "/misure.md");
    md << "# Misure del modello Chioccia (generate automaticamente)\n\n"
       << "Generato da `chioccia_measure` a " << gFs << " Hz. Questi valori descrivono questa implementazione; "
       << "non sono un confronto con hardware o con plug-in di terze parti.\n";
    std::cout << "static curves...\n";   staticCurves (dir);
    std::cout << "attack/release...\n";  attackRelease (dir);
    std::cout << "distortion...\n";      distortion (dir);
    std::cout << "frequency response...\n"; frequencyResponse (dir);
    std::cout << "detector...\n";        detectorFrequency();
    std::cout << "aliasing...\n";        aliasing();
    md.close();
    std::cout << "written " << dir << "/misure.md\n";
    return 0;
}

// Chioccia — VU meter (original code)
#include "VuMeter.h"

#include "ChiocciaLookAndFeel.h"

namespace chioccia::ui
{
using namespace juce;

namespace
{
constexpr float kMinVu = -20.0f, kMaxVu = 3.0f;
constexpr float kArc = 0.78f;   // half-angle of the scale (radians)

/** Non-linear scale like classic VU faces: amplitude-proportional (0 VU at 70 % of the arc). */
float vuToFraction (float vu)
{
    const float a = std::pow (10.0f, jlimit (kMinVu - 2.0f, kMaxVu + 1.0f, vu) / 20.0f);
    const float a0 = std::pow (10.0f, kMinVu / 20.0f), a1 = std::pow (10.0f, kMaxVu / 20.0f);
    return (a - a0) / (a1 - a0);
}
} // namespace

VuMeter::VuMeter (std::function<Reading()> src) : source (std::move (src))
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (60);
}

float VuMeter::vuToAngle (float vu) const
{
    return -kArc + 2.0f * kArc * vuToFraction (vu);
}

void VuMeter::timerCallback()
{
    if (! source) return;
    const auto r = source();
    showsOutput = r.showsOutput;
    lastValue = r.valueDb;
    const float target = jlimit (kMinVu - 2.0f, kMaxVu + 1.0f, r.showsOutput ? r.valueDb + 18.0f : -r.valueDb);

    // Second-order needle (≈ 300 ms to 99 %, slight overshoot) integrated in small steps.
    constexpr float omega = 2.0f * MathConstants<float>::pi * 2.1f, zeta = 0.78f;
    const float dt = 1.0f / 60.0f / 4.0f;
    for (int i = 0; i < 4; ++i)
    {
        const float acc = omega * omega * (target - position) - 2.0f * zeta * omega * velocity;
        velocity += acc * dt;
        position += velocity * dt;
    }
    repaint();
}

void VuMeter::paint (Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float s = b.getWidth() / 208.0f;   // designed at 208 x 122

    // bezel
    g.setColour (Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (b.translated (0.0f, 3.0f * s), 8.0f * s);
    g.setGradientFill (ColourGradient (Colour (0xff4b4f55), b.getX(), b.getY(), Colour (0xff15171a), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 8.0f * s);
    auto face = b.reduced (9.0f * s);
    g.setGradientFill (ColourGradient (Colour (0xfffff6dd), face.getCentreX(), face.getY() + face.getHeight() * 0.3f,
                                       Colour (0xffe9c784), face.getRight(), face.getBottom(), true));
    g.fillRoundedRectangle (face, 4.0f * s);

    const Point<float> pivot (face.getCentreX(), face.getBottom() + face.getHeight() * 0.32f);
    const float radius = face.getHeight() * 1.02f;

    // scale
    auto pointOnArc = [&] (float angle, float r) { return pivot + Point<float> (std::sin (angle), -std::cos (angle)) * r; };
    Path arc, red;
    arc.addCentredArc (pivot.x, pivot.y, radius * 0.86f, radius * 0.86f, 0.0f, vuToAngle (kMinVu), vuToAngle (0.0f), true);
    red.addCentredArc (pivot.x, pivot.y, radius * 0.86f, radius * 0.86f, 0.0f, vuToAngle (0.0f), vuToAngle (kMaxVu), true);
    g.setColour (colours::ink);
    g.strokePath (arc, PathStrokeType (1.6f * s));
    g.setColour (colours::red);
    g.strokePath (red, PathStrokeType (4.0f * s));

    g.setFont (ChiocciaLookAndFeel::font (11.0f * s));
    for (float vu : { -20.0f, -10.0f, -7.0f, -5.0f, -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f })
    {
        const float a = vuToAngle (vu);
        g.setColour (vu > 0.0f ? colours::red : colours::ink);
        g.drawLine (Line<float> (pointOnArc (a, radius * 0.86f), pointOnArc (a, radius * 0.95f)), 1.4f * s);
        const String txt = vu > 0.0f ? "+" + String ((int) vu) : String ((int) std::abs (vu));
        g.drawText (txt, Rectangle<float> (22.0f * s, 12.0f * s).withCentre (pointOnArc (a, radius * 1.03f)), Justification::centred, false);
    }

    g.setColour (colours::ink);
    g.setFont (ChiocciaLookAndFeel::font (17.0f * s));
    g.drawText ("VU", face.withTrimmedTop (face.getHeight() * 0.42f).withHeight (20.0f * s), Justification::centred, false);
    g.setFont (ChiocciaLookAndFeel::font (10.0f * s));
    g.setColour (colours::ink.withAlpha (0.75f));
    g.drawText (showsOutput ? "OUTPUT  (0 VU = -18 dBFS)" : "GAIN REDUCTION",
                face.withTrimmedTop (face.getHeight() * 0.62f).withHeight (14.0f * s), Justification::centred, false);
    g.setFont (ChiocciaLookAndFeel::font (12.0f * s));
    g.setColour (colours::goldDark);
    g.drawText ("CHIOCCIA", face.withTrimmedTop (face.getHeight() * 0.77f).withHeight (16.0f * s), Justification::centred, false);

    // numeric read-out (small, bottom-right)
    g.setFont (ChiocciaLookAndFeel::font (10.0f * s));
    g.setColour (colours::ink.withAlpha (0.8f));
    const float shown = showsOutput ? lastValue : (lastValue > 0.05f ? -lastValue : 0.0f);
    const String readout = String (shown, 1) + (showsOutput ? " dBFS" : " dB");
    g.drawText (readout, face.reduced (5.0f * s).removeFromBottom (12.0f * s), Justification::bottomRight, false);

    // needle
    g.saveState();
    g.reduceClipRegion (face.toNearestInt());
    const float a = vuToAngle (position);
    const auto tip = pointOnArc (a, radius * 0.97f);
    const auto base = pointOnArc (a, radius * 0.25f);
    g.setColour (Colours::black.withAlpha (0.18f));
    g.drawLine (Line<float> (base.translated (3.0f * s, 2.0f * s), tip.translated (3.0f * s, 2.0f * s)), 1.8f * s);
    g.setColour (Colour (0xff141414));
    g.drawLine (Line<float> (base, tip), 1.6f * s);
    g.restoreState();

    // pivot housing and glass
    g.setColour (Colour (0xff23262a));
    g.fillRoundedRectangle (Rectangle<float> (face.getWidth() * 0.34f, 10.0f * s).withCentre ({ face.getCentreX(), face.getBottom() - 4.0f * s }), 4.0f * s);
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.28f), face.getX(), face.getY(), Colours::transparentWhite, face.getX(), face.getCentreY(), false));
    g.fillRoundedRectangle (face.withHeight (face.getHeight() * 0.45f), 4.0f * s);
}
} // namespace chioccia::ui

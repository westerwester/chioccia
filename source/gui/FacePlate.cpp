// Chioccia — static background (original artwork)
#include "FacePlate.h"

#include "ChiocciaLookAndFeel.h"
#include "Controls.h"
#include "Layout.h"

namespace chioccia::ui
{
using namespace juce;

FacePlate::FacePlate()
{
    setInterceptsMouseClicks (false, false);
    setOpaque (false);
}

namespace
{
void drawRackEar (Graphics& g, Rectangle<float> ear)
{
    g.setGradientFill (ColourGradient (Colour (0xffe9ecef), ear.getX(), ear.getY(), Colour (0xff9aa1a8), ear.getRight(), ear.getBottom(), false));
    g.fillRoundedRectangle (ear, 3.0f);
    g.setColour (Colours::black.withAlpha (0.35f));
    g.drawRoundedRectangle (ear, 3.0f, 1.0f);
    for (float fy : { 0.2f, 0.8f })
    {
        auto slot = Rectangle<float> (ear.getWidth() * 0.62f, 13.0f).withCentre ({ ear.getCentreX(), ear.getY() + ear.getHeight() * fy });
        g.setColour (Colour (0xff2b2f33));
        g.fillRoundedRectangle (slot, 6.5f);
        g.setColour (Colours::white.withAlpha (0.5f));
        g.drawRoundedRectangle (slot.translated (0.0f, 1.0f), 6.5f, 0.8f);
    }
}

void titleWordmark (Graphics& g, Rectangle<float> area)
{
    GlyphArrangement ga;
    ga.addFittedText (ChiocciaLookAndFeel::font (area.getHeight() * 0.95f), "CHIOCCIA",
                      area.getX(), area.getY(), area.getWidth(), area.getHeight(), Justification::centred, 1, 0.8f);
    Path p;
    ga.createPath (p);
    const auto b = p.getBounds();

    // 3D extrusion
    g.setColour (Colour (0xff6b3a12));
    for (int i = 6; i > 0; --i)
        g.fillPath (p, AffineTransform::translation ((float) i * 0.5f, (float) i));
    g.setColour (colours::ink);
    g.strokePath (p, PathStrokeType (7.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (Colours::white);
    g.strokePath (p, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setGradientFill (ColourGradient (Colour (0xfffff3b0), b.getX(), b.getY(), Colour (0xffff9d00), b.getX(), b.getBottom(), false));
    g.fillPath (p);
    // glossy highlight on the top half
    g.saveState();
    g.reduceClipRegion (b.withHeight (b.getHeight() * 0.45f).toNearestInt());
    g.setColour (Colours::white.withAlpha (0.35f));
    g.fillPath (p);
    g.restoreState();
}
} // namespace

void FacePlate::paint (Graphics& g)
{
    const float scale = (float) getWidth() / layout::kWidth;
    g.addTransform (AffineTransform::scale (scale));
    using namespace layout;

    // rack ears
    drawRackEar (g, { 0.0f, 10.0f, 44.0f, 320.0f });
    drawRackEar (g, { 956.0f, 10.0f, 44.0f, 320.0f });

    // faceplate
    const auto plateR = face;
    g.setColour (Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (plateR.translated (0.0f, 3.0f), 8.0f);
    g.saveState();
    Path clip;
    clip.addRoundedRectangle (plateR, 8.0f);
    g.reduceClipRegion (clip);

    g.setGradientFill (ColourGradient (colours::skyTop, 0.0f, plateR.getY(), colours::skyBottom, 0.0f, plateR.getBottom(), false));
    g.fillRect (plateR);
    // sunshine glow behind the title
    g.setGradientFill (ColourGradient (Colour (0xfffff6c8).withAlpha (0.9f), 500.0f, 60.0f, Colour (0x00fff6c8), 500.0f, 230.0f, true));
    g.fillEllipse (220.0f, -120.0f, 560.0f, 360.0f);

    // clouds
    art::drawCloud (g, { 70.0f, 30.0f, 150.0f, 52.0f });
    art::drawCloud (g, { 610.0f, 120.0f, 120.0f, 40.0f });
    art::drawCloud (g, { 260.0f, 250.0f, 140.0f, 50.0f });
    art::drawCloud (g, { 680.0f, 22.0f, 170.0f, 56.0f });
    art::drawCloud (g, { 880.0f, 60.0f, 90.0f, 34.0f });
    // warm haze near the floor
    g.setGradientFill (ColourGradient (Colour (0x00ffd86b), 0.0f, 230.0f, Colour (0x66ffcf4d), 0.0f, 332.0f, false));
    g.fillRect (plateR.withTop (230.0f));

    // floating feathers and golden eggs
    art::drawFeather (g, { 250.0f, 70.0f }, 34.0f, 0.9f);
    art::drawFeather (g, { 300.0f, 48.0f }, 26.0f, -0.5f);
    art::drawFeather (g, { 700.0f, 74.0f }, 30.0f, 0.4f);
    art::drawFeather (g, { 735.0f, 300.0f }, 28.0f, -1.1f);
    art::drawFeather (g, { 372.0f, 300.0f }, 24.0f, 1.3f);
    art::drawFeather (g, { 905.0f, 46.0f }, 26.0f, 0.2f);
    art::drawGoldenEgg (g, { 352.0f, 108.0f }, 20.0f, -0.4f);
    art::drawGoldenEgg (g, { 648.0f, 98.0f }, 18.0f, 0.5f);
    art::drawGoldenEgg (g, { 232.0f, 312.0f }, 22.0f, 0.3f);
    art::drawGoldenEgg (g, { 612.0f, 314.0f }, 18.0f, -0.6f);

    // hens
    art::drawChicken (g, chickenLeft, false, true);
    art::drawChicken (g, chickenRight, true, true);
    art::drawNest (g, nest);

    // bottom hinge strip
    g.setGradientFill (ColourGradient (Colour (0xffd5d9de), 0.0f, 318.0f, Colour (0xff8d949b), 0.0f, 332.0f, false));
    g.fillRect (Rectangle<float> (40.0f, 320.0f, 920.0f, 12.0f));
    g.setColour (Colours::black.withAlpha (0.25f));
    for (float x = 48.0f; x < 956.0f; x += 16.0f)
        g.drawVerticalLine ((int) x, 321.0f, 331.0f);
    g.restoreState();

    // plate edge
    g.setColour (Colour (0xffeef1f4));
    g.drawRoundedRectangle (plateR.reduced (1.0f), 8.0f, 2.0f);
    g.setColour (Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (plateR, 8.0f, 1.0f);

    // screws
    for (auto p : { Point<float> (54.0f, 22.0f), Point<float> (946.0f, 22.0f), Point<float> (54.0f, 308.0f), Point<float> (946.0f, 308.0f) })
        art::drawScrew (g, p, 6.0f, 0.6f);

    // title and subtitle
    titleWordmark (g, title);
    g.setColour (colours::ink);
    g.fillRoundedRectangle (subtitle, 10.0f);
    g.setColour (colours::cream);
    g.setFont (ChiocciaLookAndFeel::font (14.0f));
    g.drawText ("OPTO LEVELING AMPLIFIER", subtitle, Justification::centred, false);

    // control labels
    const auto ink = colours::ink, white = Colours::white;
    ChiocciaLookAndFeel::drawOutlinedText (g, "GAIN", gainLabel, 24.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "PEAK REDUCTION", prLabel, 24.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "LIMIT", limitLabel, 19.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "COMPRESS", compressLabel, 19.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "OUTPUT", outputLabel, 19.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "ON", onLabel, 19.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "POWER", powerLabel, 19.0f, ink, white, 5.0f);
    ChiocciaLookAndFeel::drawOutlinedText (g, "DRY", mixScale.withWidth (60.0f), 14.0f, ink, white, 4.0f, Justification::centredLeft);
    ChiocciaLookAndFeel::drawOutlinedText (g, "WET", mixScale.withTrimmedLeft (mixScale.getWidth() - 60.0f), 14.0f, ink, white, 4.0f, Justification::centredRight);
}
} // namespace chioccia::ui

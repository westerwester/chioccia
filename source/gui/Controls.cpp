// Chioccia — original vector artwork (drawn from scratch for this project)
#include "Controls.h"

#include "ChiocciaLookAndFeel.h"

namespace chioccia::ui::art
{
using namespace juce;

namespace
{
Path ellipse (float cx, float cy, float w, float h, float rotation = 0.0f)
{
    Path p;
    p.addEllipse (cx - w * 0.5f, cy - h * 0.5f, w, h);
    p.applyTransform (AffineTransform::rotation (rotation, cx, cy));
    return p;
}

void outlineAndFill (Graphics& g, const Path& p, const FillType& fill, float lineWidth)
{
    g.setColour (colours::ink);
    g.strokePath (p, PathStrokeType (lineWidth, PathStrokeType::curved, PathStrokeType::rounded));
    g.setFillType (fill);
    g.fillPath (p);
}
} // namespace

void drawChicken (Graphics& g, Rectangle<float> box, bool faceLeft, bool beakOpen)
{
    // Unit design space: x 0..1, y 0..1.3 (uniformly scaled into box, bottom-aligned).
    const float s = std::min (box.getWidth(), box.getHeight() / 1.3f);
    const float ox = box.getCentreX() - s * 0.5f, oy = box.getBottom() - s * 1.3f;
    AffineTransform xf = faceLeft ? AffineTransform::scale (-1.0f, 1.0f).translated (1.0f, 0.0f) : AffineTransform();
    xf = xf.scaled (s).translated (ox, oy);
    auto T = [&xf] (Path p) { p.applyTransform (xf); return p; };
    auto pt = [&xf] (float x, float y) { float a = x, b = y; xf.transformPoint (a, b); return Point<float> (a, b); };
    const float lw = s * 0.022f;

    const Colour white (0xfffdfdfd), shade (0xffd9e1ea), comb (colours::red);

    // tail feathers
    for (auto [cx, cy, rot] : { std::tuple { 0.13f, 0.66f, -0.55f }, std::tuple { 0.07f, 0.78f, -1.0f }, std::tuple { 0.17f, 0.57f, -0.2f } })
        outlineAndFill (g, T (ellipse (cx, cy, 0.2f, 0.36f, rot)), FillType (white), lw);

    // comb (behind the head)
    for (auto [cx, cy, r] : { std::tuple { 0.49f, 0.16f, 0.075f }, std::tuple { 0.60f, 0.105f, 0.09f }, std::tuple { 0.71f, 0.15f, 0.075f } })
        outlineAndFill (g, T (ellipse (cx, cy, r * 2.0f, r * 2.0f)), FillType (comb), lw);

    // body + neck + head: outlines first, fills on top → only the outer silhouette keeps its outline
    const Path body = T (ellipse (0.47f, 0.90f, 0.82f, 0.72f));
    const Path neck = T (ellipse (0.56f, 0.62f, 0.50f, 0.46f));
    const Path head = T (ellipse (0.63f, 0.42f, 0.56f, 0.54f));
    g.setColour (colours::ink);
    for (auto* p : { &body, &neck, &head })
        g.strokePath (*p, PathStrokeType (lw * 1.2f, PathStrokeType::curved, PathStrokeType::rounded));
    const auto c = pt (0.55f, 0.6f);
    g.setGradientFill (ColourGradient (white, c.x, c.y, shade, c.x + s * 0.55f, c.y + s * 0.55f, true));
    for (auto* p : { &body, &neck, &head })
        g.fillPath (*p);

    // wing with feather scallops
    outlineAndFill (g, T (ellipse (0.40f, 0.93f, 0.42f, 0.28f, -0.35f)), FillType (Colour (0xfff1f4f8)), lw);
    for (int i = 0; i < 3; ++i)
    {
        Path arc;
        const float x = 0.30f + 0.08f * (float) i, y = 0.99f - 0.03f * (float) i;
        arc.addCentredArc (x, y, 0.05f, 0.035f, -0.35f, MathConstants<float>::pi * 0.6f, MathConstants<float>::pi * 1.4f, true);
        g.setColour (colours::ink.withAlpha (0.55f));
        g.strokePath (T (arc), PathStrokeType (lw * 0.6f));
    }

    // cheek blush
    g.setColour (Colour (0xffff8fa3).withAlpha (0.55f));
    g.fillPath (T (ellipse (0.53f, 0.54f, 0.11f, 0.055f)));

    // eyes
    for (float ex : { 0.56f, 0.76f })
    {
        outlineAndFill (g, T (ellipse (ex, 0.37f, 0.17f, 0.22f)), FillType (Colours::white), lw * 0.9f);
        g.setColour (Colours::black);
        g.fillPath (T (ellipse (ex + 0.03f, 0.39f, 0.075f, 0.095f)));
        g.setColour (Colours::white);
        g.fillPath (T (ellipse (ex + 0.045f, 0.365f, 0.03f, 0.03f)));
    }

    // beak
    Path upper;
    upper.startNewSubPath (0.80f, 0.45f);
    upper.quadraticTo (0.93f, 0.43f, 1.0f, beakOpen ? 0.50f : 0.53f);
    upper.quadraticTo (0.90f, 0.52f, 0.80f, 0.54f);
    upper.closeSubPath();
    Path lower;
    lower.startNewSubPath (0.81f, beakOpen ? 0.57f : 0.54f);
    lower.quadraticTo (0.90f, beakOpen ? 0.58f : 0.54f, 0.96f, beakOpen ? 0.61f : 0.555f);
    lower.quadraticTo (0.88f, 0.64f, 0.80f, beakOpen ? 0.63f : 0.58f);
    lower.closeSubPath();
    if (beakOpen)
    {
        Path mouth;
        mouth.addQuadrilateral (0.80f, 0.50f, 0.97f, 0.51f, 0.94f, 0.60f, 0.80f, 0.60f);
        g.setColour (Colour (0xff7a1020));
        g.fillPath (T (mouth));
        g.setColour (Colour (0xffff6f8a));
        g.fillPath (T (ellipse (0.87f, 0.575f, 0.09f, 0.045f)));
    }
    const auto b0 = pt (0.85f, 0.43f), b1 = pt (0.85f, 0.62f);
    const FillType beakFill (ColourGradient (Colour (0xffffd84a), b0.x, b0.y, Colour (0xffff961f), b1.x, b1.y, false));
    outlineAndFill (g, T (lower), beakFill, lw * 0.9f);
    outlineAndFill (g, T (upper), beakFill, lw * 0.9f);

    // wattle
    outlineAndFill (g, T (ellipse (0.80f, 0.68f, 0.07f, 0.11f)), FillType (comb), lw * 0.8f);

    // feet
    g.setColour (Colour (0xffff9a1f));
    for (float fx : { 0.38f, 0.56f })
    {
        Path foot;
        foot.startNewSubPath (fx, 1.20f); foot.lineTo (fx, 1.28f);
        foot.startNewSubPath (fx, 1.28f); foot.lineTo (fx - 0.05f, 1.30f);
        foot.startNewSubPath (fx, 1.28f); foot.lineTo (fx + 0.06f, 1.30f);
        g.strokePath (T (foot), PathStrokeType (lw * 1.3f, PathStrokeType::curved, PathStrokeType::rounded));
    }
}

void drawGoldenEgg (Graphics& g, Point<float> centre, float width, float rotation)
{
    const float h = width * 1.28f;
    Path egg;
    // egg silhouette: narrower top
    egg.startNewSubPath (0.0f, -h * 0.5f);
    egg.cubicTo (width * 0.42f, -h * 0.5f, width * 0.55f, h * 0.12f, 0.0f, h * 0.5f);
    egg.cubicTo (-width * 0.55f, h * 0.12f, -width * 0.42f, -h * 0.5f, 0.0f, -h * 0.5f);
    egg.closeSubPath();
    egg.applyTransform (AffineTransform::rotation (rotation).translated (centre));
    const auto b = egg.getBounds();
    g.setColour (Colours::black.withAlpha (0.22f));
    g.fillPath (egg, AffineTransform::translation (width * 0.06f, width * 0.1f));
    g.setGradientFill (ColourGradient (colours::goldLight, b.getX(), b.getY(), colours::goldDark, b.getRight(), b.getBottom(), false));
    g.fillPath (egg);
    g.setColour (colours::ink);
    g.strokePath (egg, PathStrokeType (std::max (1.0f, width * 0.07f)));
    g.setColour (Colours::white.withAlpha (0.7f));
    g.fillEllipse (Rectangle<float> (width * 0.22f, h * 0.2f).withCentre (centre + Point<float> (-width * 0.16f, -h * 0.18f).rotatedAboutOrigin (rotation)));
}

void drawFeather (Graphics& g, Point<float> centre, float length, float rotation)
{
    Path f;
    f.startNewSubPath (0.0f, -length * 0.5f);
    f.quadraticTo (length * 0.26f, -length * 0.1f, length * 0.05f, length * 0.45f);
    f.lineTo (-length * 0.03f, length * 0.45f);
    f.quadraticTo (-length * 0.22f, -length * 0.05f, 0.0f, -length * 0.5f);
    f.closeSubPath();
    const auto xf = AffineTransform::rotation (rotation).translated (centre);
    f.applyTransform (xf);
    g.setColour (Colours::white.withAlpha (0.95f));
    g.fillPath (f);
    g.setColour (Colour (0xff9fb0bf));
    g.strokePath (f, PathStrokeType (std::max (0.8f, length * 0.03f)));
    Path rachis;
    rachis.startNewSubPath (0.0f, -length * 0.42f);
    rachis.quadraticTo (length * 0.04f, 0.0f, 0.01f * length, length * 0.6f);
    rachis.applyTransform (xf);
    g.strokePath (rachis, PathStrokeType (std::max (0.8f, length * 0.025f)));
}

void drawCloud (Graphics& g, Rectangle<float> a)
{
    Path cloud;
    cloud.addEllipse (a.getX(), a.getY() + a.getHeight() * 0.35f, a.getWidth(), a.getHeight() * 0.65f);
    cloud.addEllipse (a.getX() + a.getWidth() * 0.15f, a.getY() + a.getHeight() * 0.1f, a.getWidth() * 0.4f, a.getHeight() * 0.8f);
    cloud.addEllipse (a.getX() + a.getWidth() * 0.42f, a.getY(), a.getWidth() * 0.38f, a.getHeight() * 0.85f);
    g.setColour (Colour (0xffd2e6f3).withAlpha (0.7f));
    g.fillPath (cloud, AffineTransform::translation (0.0f, a.getHeight() * 0.08f));
    g.setColour (Colours::white.withAlpha (0.85f));
    g.fillPath (cloud);
}

void drawNest (Graphics& g, Rectangle<float> a)
{
    const Colour straw (0xffc48a43), dark (0xff6e4221);
    // back rim
    g.setColour (dark);
    g.fillEllipse (a.withHeight (a.getHeight() * 0.5f));
    // golden eggs resting in the nest
    const float ew = a.getWidth() * 0.2f;
    drawGoldenEgg (g, { a.getX() + a.getWidth() * 0.3f, a.getY() + a.getHeight() * 0.18f }, ew, -0.3f);
    drawGoldenEgg (g, { a.getX() + a.getWidth() * 0.52f, a.getY() + a.getHeight() * 0.12f }, ew * 1.05f, 0.1f);
    drawGoldenEgg (g, { a.getX() + a.getWidth() * 0.72f, a.getY() + a.getHeight() * 0.2f }, ew * 0.95f, 0.35f);
    // front bowl
    Path bowl;
    bowl.startNewSubPath (a.getX(), a.getY() + a.getHeight() * 0.25f);
    bowl.quadraticTo (a.getCentreX(), a.getY() + a.getHeight() * 0.45f, a.getRight(), a.getY() + a.getHeight() * 0.25f);
    bowl.quadraticTo (a.getRight() - a.getWidth() * 0.05f, a.getBottom(), a.getCentreX(), a.getBottom());
    bowl.quadraticTo (a.getX() + a.getWidth() * 0.05f, a.getBottom(), a.getX(), a.getY() + a.getHeight() * 0.25f);
    bowl.closeSubPath();
    g.setGradientFill (ColourGradient (Colour (0xffb07636), a.getX(), a.getY(), dark, a.getX(), a.getBottom(), false));
    g.fillPath (bowl);
    g.saveState();
    g.reduceClipRegion (bowl);
    Random rng (7);
    g.setColour (straw);
    for (int i = 0; i < 26; ++i)
    {
        const float y = a.getY() + a.getHeight() * (0.3f + 0.65f * rng.nextFloat());
        const float x0 = a.getX() + a.getWidth() * rng.nextFloat() * 0.6f;
        Path st;
        st.startNewSubPath (x0, y);
        st.quadraticTo (x0 + a.getWidth() * 0.2f, y - a.getHeight() * 0.12f, x0 + a.getWidth() * (0.35f + 0.2f * rng.nextFloat()), y + a.getHeight() * 0.05f);
        g.strokePath (st, PathStrokeType (std::max (1.0f, a.getHeight() * 0.035f), PathStrokeType::curved, PathStrokeType::rounded));
    }
    g.restoreState();
    g.setColour (colours::ink);
    g.strokePath (bowl, PathStrokeType (std::max (1.0f, a.getHeight() * 0.04f)));
}

void drawScrew (Graphics& g, Point<float> c, float r, float angle)
{
    auto area = Rectangle<float> (r * 2.0f, r * 2.0f).withCentre (c);
    g.setGradientFill (ColourGradient (Colour (0xfff1f2f4), area.getX(), area.getY(), Colour (0xff6c727a), area.getRight(), area.getBottom(), false));
    g.fillEllipse (area);
    g.setColour (Colours::black.withAlpha (0.55f));
    g.drawEllipse (area, std::max (0.8f, r * 0.12f));
    const auto d = Point<float> (std::cos (angle), std::sin (angle)) * (r * 0.7f);
    g.drawLine (Line<float> (c - d, c + d), std::max (1.0f, r * 0.25f));
}
} // namespace chioccia::ui::art

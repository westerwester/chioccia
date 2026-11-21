// Chioccia — look and feel (original code / original vector artwork)
#include "ChiocciaLookAndFeel.h"

#include <BinaryData.h>

namespace chioccia::ui
{
using namespace juce;

Typeface::Ptr ChiocciaLookAndFeel::typeface()
{
    // Lilita One (SIL Open Font License 1.1, see assets/fonts/OFL-LilitaOne.txt), embedded.
    static Typeface::Ptr tf = Typeface::createSystemTypefaceFor (ChiocciaBinary::LilitaOneRegular_ttf,
                                                                 (size_t) ChiocciaBinary::LilitaOneRegular_ttfSize);
    return tf;
}

Font ChiocciaLookAndFeel::font (float height)
{
    return Font (FontOptions (typeface()).withHeight (std::max (1.0f, height)));
}

ChiocciaLookAndFeel::ChiocciaLookAndFeel()
{
    setDefaultSansSerifTypeface (typeface());
    setColour (PopupMenu::backgroundColourId, colours::chocolate);
    setColour (PopupMenu::textColourId, colours::cream);
    setColour (PopupMenu::highlightedBackgroundColourId, colours::gold);
    setColour (PopupMenu::highlightedTextColourId, colours::ink);
    setColour (ComboBox::textColourId, colours::cream);
    setColour (ComboBox::arrowColourId, colours::goldLight);
    setColour (Label::textColourId, colours::ink);
    setColour (TooltipWindow::backgroundColourId, colours::chocolate);
    setColour (TooltipWindow::textColourId, colours::cream);
    setColour (TextButton::textColourOffId, colours::cream);
    setColour (TextButton::textColourOnId, colours::ink);
    setColour (BubbleComponent::backgroundColourId, colours::chocolate);
    setColour (BubbleComponent::outlineColourId, colours::goldLight);
}

void ChiocciaLookAndFeel::drawOutlinedText (Graphics& g, const String& text, Rectangle<float> area, float height,
                                            Colour fill, Colour outline, float outlineWidth, Justification just)
{
    GlyphArrangement ga;
    ga.addFittedText (font (height), text, area.getX(), area.getY(), area.getWidth(), area.getHeight(), just, 1, 0.7f);
    Path p;
    ga.createPath (p);
    if (outlineWidth > 0.0f)
    {
        g.setColour (outline);
        g.strokePath (p, PathStrokeType (outlineWidth, PathStrokeType::curved, PathStrokeType::rounded));
    }
    g.setColour (fill);
    g.fillPath (p);
}

// --------------------------------------------------------------------------------------------------
void ChiocciaLookAndFeel::drawKnob (Graphics& g, Rectangle<float> area, float angle, bool big, bool hover)
{
    const auto c = area.getCentre();
    const float r = area.getWidth() * 0.5f;

    // drop shadow
    g.setColour (Colours::black.withAlpha (0.35f));
    g.fillEllipse (area.translated (r * 0.06f, r * 0.12f).expanded (r * 0.04f));

    // fluted skirt
    Path skirt;
    const int flutes = big ? 28 : 20;
    for (int i = 0; i <= flutes * 2; ++i)
    {
        const float a = MathConstants<float>::twoPi * (float) i / (float) (flutes * 2);
        const float rr = (i % 2 == 0) ? r : r * 0.94f;
        const auto pt = c + Point<float> (std::sin (a), -std::cos (a)) * rr;
        if (i == 0) skirt.startNewSubPath (pt); else skirt.lineTo (pt);
    }
    skirt.closeSubPath();
    g.setGradientFill (ColourGradient (Colour (0xff4a4a50), c.x - r, c.y - r, Colour (0xff050506), c.x + r, c.y + r, false));
    g.fillPath (skirt);

    // cap
    const float capR = r * 0.80f;
    ColourGradient cap (Colour (0xff5d5d66), c.x - capR * 0.5f, c.y - capR * 0.7f, Colour (0xff0b0b0d), c.x + capR * 0.4f, c.y + capR, true);
    g.setGradientFill (cap);
    g.fillEllipse (Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre (c));
    g.setColour (Colours::white.withAlpha (hover ? 0.28f : 0.16f));
    g.drawEllipse (Rectangle<float> (capR * 2.0f, capR * 2.0f).withCentre (c).reduced (r * 0.03f), r * 0.03f);

    // specular
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.30f), c.x - capR * 0.4f, c.y - capR * 0.8f,
                                       Colours::transparentWhite, c.x, c.y, false));
    g.fillEllipse (Rectangle<float> (capR * 1.3f, capR * 0.9f).withCentre (c.translated (-capR * 0.15f, -capR * 0.4f)));

    // pointer
    const auto dir = Point<float> (std::sin (angle), -std::cos (angle));
    g.setColour (colours::cream);
    g.drawLine (Line<float> (c + dir * (r * 0.18f), c + dir * (r * 0.92f)), std::max (1.5f, r * (big ? 0.075f : 0.10f)));
}

void ChiocciaLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                            float startAngle, float endAngle, Slider& s)
{
    const auto styleName = s.getProperties()[style::key].toString();
    const bool big = styleName == "bigKnob";
    auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool hover = s.isMouseOverOrDragging();

    if (big)
    {
        const float side = std::min (bounds.getWidth(), bounds.getHeight());
        auto plateArea = Rectangle<float> (side, side).withCentre (bounds.getCentre());
        const auto c = plateArea.getCentre();
        const float pr = side * 0.5f;

        // translucent scale plate
        g.setColour (Colours::white.withAlpha (0.55f));
        g.fillEllipse (plateArea.reduced (pr * 0.02f));
        g.setColour (colours::ink.withAlpha (0.35f));
        g.drawEllipse (plateArea.reduced (pr * 0.02f), std::max (1.0f, pr * 0.012f));

        // ticks and numbers 0..100
        for (int i = 0; i <= 10; ++i)
        {
            const float a = startAngle + (float) i / 10.0f * (endAngle - startAngle);
            const auto dir = Point<float> (std::sin (a), -std::cos (a));
            g.setColour (colours::ink);
            g.drawLine (Line<float> (c + dir * (pr * 0.66f), c + dir * (pr * 0.72f)), std::max (1.0f, pr * 0.018f));
            const auto tc = c + dir * (pr * 0.84f);
            const String label (i * 10);
            g.setFont (font (pr * 0.15f));
            g.drawText (label, Rectangle<float> (pr * 0.4f, pr * 0.2f).withCentre (tc), Justification::centred, false);
        }
        drawKnob (g, Rectangle<float> (pr * 1.18f, pr * 1.18f).withCentre (c), angle, true, hover);
        return;
    }

    // small knob: knob on top, value text underneath ("trimKnob": no text, the editor shows a read-out)
    const bool withText = styleName != "trimKnob";
    const float textH = withText ? std::max (9.0f, bounds.getHeight() * 0.22f) : 0.0f;
    auto textArea = bounds.removeFromBottom (textH);
    const float side = std::min (bounds.getWidth(), bounds.getHeight());
    auto knobArea = Rectangle<float> (side, side).withCentre (bounds.getCentre());
    const auto c = knobArea.getCentre();

    // arc showing the value
    Path track, value;
    const float ar = side * 0.47f;
    track.addCentredArc (c.x, c.y, ar, ar, 0.0f, startAngle, endAngle, true);
    value.addCentredArc (c.x, c.y, ar, ar, 0.0f, startAngle, angle, true);
    g.setColour (withText ? Colours::white.withAlpha (0.25f) : colours::ink.withAlpha (0.6f));
    g.strokePath (track, PathStrokeType (side * 0.06f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (colours::gold);
    g.strokePath (value, PathStrokeType (side * 0.06f, PathStrokeType::curved, PathStrokeType::rounded));
    drawKnob (g, knobArea.reduced (side * 0.13f), angle, false, hover);
    if (! withText) return;

    g.setColour (colours::cream);
    g.setFont (font (textH * 0.85f));
    g.drawText (s.getTextFromValue (s.getValue()), textArea, Justification::centred, false);
}

void ChiocciaLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                            Slider::SliderStyle, Slider& s)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float trackH = b.getHeight() * 0.36f;
    auto track = Rectangle<float> (b.getX(), b.getCentreY() - trackH * 0.5f, b.getWidth(), trackH);

    g.setColour (colours::ink);
    g.fillRoundedRectangle (track.expanded (2.0f), trackH * 0.5f + 2.0f);
    g.setColour (colours::chocolate.darker (0.3f));
    g.fillRoundedRectangle (track, trackH * 0.5f);

    auto filled = track.withRight (pos);
    g.setGradientFill (ColourGradient (colours::goldLight, track.getX(), track.getY(), colours::amber, track.getX(), track.getBottom(), false));
    g.fillRoundedRectangle (filled, trackH * 0.5f);

    // golden-egg thumb
    const float eh = b.getHeight() * 0.95f, ew = eh * 0.78f;
    auto egg = Rectangle<float> (ew, eh).withCentre ({ pos, b.getCentreY() });
    g.setColour (Colours::black.withAlpha (0.3f));
    g.fillEllipse (egg.translated (1.5f, 2.5f));
    g.setGradientFill (ColourGradient (colours::goldLight, egg.getX(), egg.getY(), colours::goldDark, egg.getRight(), egg.getBottom(), false));
    g.fillEllipse (egg);
    g.setColour (colours::ink);
    g.drawEllipse (egg, std::max (1.0f, eh * 0.05f));
    g.setColour (Colours::white.withAlpha (0.65f));
    g.fillEllipse (egg.getX() + ew * 0.22f, egg.getY() + eh * 0.14f, ew * 0.25f, eh * 0.2f);
    if (s.isMouseOverOrDragging())
    {
        g.setColour (Colours::white.withAlpha (0.25f));
        g.drawEllipse (egg.expanded (2.0f), 2.0f);
    }
}

void ChiocciaLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool highlighted, bool)
{
    const auto styleName = b.getProperties()[style::key].toString();
    auto r = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();

    if (styleName == "bat" || styleName == "batInverted")
    {
        const bool up = styleName == "batInverted" ? ! on : on;
        const auto c = r.getCentre();
        const float s = std::min (r.getWidth(), r.getHeight());
        // dark mounting plate for contrast
        g.setColour (colours::ink.withAlpha (0.75f));
        g.fillRoundedRectangle (Rectangle<float> (s * 0.62f, s * 0.92f).withCentre (c), s * 0.12f);
        // hex nut
        Path nut;
        nut.addPolygon (c, 6, s * 0.2f, 0.0f);
        g.setGradientFill (ColourGradient (Colour (0xfff4f5f7), c.x - s * 0.2f, c.y - s * 0.2f, Colour (0xff7d838a), c.x + s * 0.2f, c.y + s * 0.2f, false));
        g.fillPath (nut);
        g.setColour (Colours::black.withAlpha (0.5f));
        g.strokePath (nut, PathStrokeType (1.0f));
        // lever
        const float dirY = up ? -1.0f : 1.0f;
        const auto tip = c + Point<float> (0.0f, dirY * s * 0.38f);
        g.setColour (Colours::black.withAlpha (0.35f));
        g.drawLine (Line<float> (c.translated (2.0f, 2.0f), tip.translated (2.0f, 2.0f)), s * 0.11f);
        g.setGradientFill (ColourGradient (Colours::white, c.x - s * 0.05f, c.y, Colour (0xff8a9097), c.x + s * 0.06f, c.y, false));
        g.drawLine (Line<float> (c, tip), s * 0.10f);
        g.fillEllipse (Rectangle<float> (s * 0.16f, s * 0.16f).withCentre (tip));
        if (highlighted)
        {
            g.setColour (colours::goldLight.withAlpha (0.5f));
            g.drawRoundedRectangle (Rectangle<float> (s * 0.62f, s * 0.92f).withCentre (c), s * 0.12f, 1.5f);
        }
        return;
    }

    if (styleName == "segment")
    {
        auto pill = r.reduced (1.0f);
        const float rad = pill.getHeight() * 0.5f;
        g.setColour (colours::ink);
        g.fillRoundedRectangle (pill, rad);
        auto left = pill.withWidth (pill.getWidth() * 0.5f).reduced (2.0f);
        auto right = left.withX (pill.getCentreX() + 2.0f);
        auto active = on ? right : left;
        g.setGradientFill (ColourGradient (colours::goldLight, active.getX(), active.getY(), colours::amber, active.getX(), active.getBottom(), false));
        g.fillRoundedRectangle (active, rad - 2.0f);
        g.setFont (font (pill.getHeight() * 0.62f));
        g.setColour (on ? colours::cream : colours::ink);
        g.drawText (b.getProperties()[style::segmentLeft].toString(), left, Justification::centred, false);
        g.setColour (on ? colours::ink : colours::cream);
        g.drawText (b.getProperties()[style::segmentRight].toString(), right, Justification::centred, false);
        if (highlighted) { g.setColour (Colours::white.withAlpha (0.3f)); g.drawRoundedRectangle (pill, rad, 1.5f); }
        return;
    }

    // "pill" toggle with LED
    auto pill = r.reduced (1.0f);
    const float rad = pill.getHeight() * 0.5f;
    g.setColour (on ? colours::gold : colours::chocolate.brighter (0.15f));
    g.fillRoundedRectangle (pill, rad);
    g.setColour (colours::ink);
    g.drawRoundedRectangle (pill, rad, 1.5f);
    auto led = Rectangle<float> (pill.getHeight() * 0.45f, pill.getHeight() * 0.45f).withCentre ({ pill.getX() + rad, pill.getCentreY() });
    g.setColour (on ? Colour (0xffff5a2b) : Colour (0xff2a1a10));
    g.fillEllipse (led);
    if (on) { g.setColour (Colour (0xffffb36b).withAlpha (0.6f)); g.drawEllipse (led.expanded (1.5f), 1.5f); }
    g.setColour (on ? colours::ink : colours::cream);
    g.setFont (font (pill.getHeight() * 0.58f));
    g.drawText (b.getButtonText(), pill.withTrimmedLeft (rad * 1.7f).withTrimmedRight (rad * 0.5f), Justification::centred, true);
    if (highlighted) { g.setColour (Colours::white.withAlpha (0.25f)); g.fillRoundedRectangle (pill, rad); }
}

void ChiocciaLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    const float rad = r.getHeight() * 0.5f;
    const bool on = b.getToggleState();
    g.setColour (colours::ink);
    g.fillRoundedRectangle (r.translated (0.0f, 2.0f), rad);
    g.setGradientFill (on ? ColourGradient (colours::goldLight, r.getX(), r.getY(), colours::amber, r.getX(), r.getBottom(), false)
                          : ColourGradient (colours::chocolate.brighter (0.25f), r.getX(), r.getY(), colours::chocolate, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (down ? r.translated (0.0f, 1.0f) : r, rad);
    g.setColour (colours::ink);
    g.drawRoundedRectangle (r, rad, 1.5f);
    if (highlighted) { g.setColour (Colours::white.withAlpha (0.18f)); g.fillRoundedRectangle (r, rad); }
}

void ChiocciaLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool)
{
    g.setFont (font (b.getHeight() * 0.58f));
    g.setColour (b.getToggleState() ? colours::ink : colours::cream);
    g.drawText (b.getButtonText(), b.getLocalBounds().reduced (4, 0), Justification::centred, true);
}

void ChiocciaLookAndFeel::drawComboBox (Graphics& g, int w, int h, bool, int, int, int, int, ComboBox& box)
{
    auto r = Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (1.0f);
    const float rad = std::min (r.getHeight() * 0.5f, 10.0f);
    g.setColour (colours::chocolate);
    g.fillRoundedRectangle (r, rad);
    g.setColour (box.isMouseOver (true) ? colours::goldLight : colours::gold.withAlpha (0.8f));
    g.drawRoundedRectangle (r, rad, 1.5f);
    Path arrow;
    const float ax = r.getRight() - r.getHeight() * 0.6f, ay = r.getCentreY();
    const float as = r.getHeight() * 0.16f;
    arrow.addTriangle (ax - as, ay - as * 0.6f, ax + as, ay - as * 0.6f, ax, ay + as * 0.8f);
    g.setColour (colours::goldLight);
    g.fillPath (arrow);
}

Font ChiocciaLookAndFeel::getComboBoxFont (ComboBox& box) { return font ((float) box.getHeight() * 0.52f); }

void ChiocciaLookAndFeel::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (6, 1, box.getWidth() - box.getHeight() - 4, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
    label.setMinimumHorizontalScale (0.6f);
    label.setJustificationType (Justification::centredLeft);
}

Font ChiocciaLookAndFeel::getPopupMenuFont()              { return font (17.0f); }
Font ChiocciaLookAndFeel::getLabelFont (Label& l)          { return font ((float) l.getHeight() * 0.7f); }
Font ChiocciaLookAndFeel::getTextButtonFont (TextButton&, int h) { return font ((float) h * 0.58f); }
Font ChiocciaLookAndFeel::getSliderPopupFont (Slider&)     { return font (16.0f); }

void ChiocciaLookAndFeel::drawBubble (Graphics& g, BubbleComponent&, const Point<float>&, const Rectangle<float>& body)
{
    g.setColour (colours::chocolate);
    g.fillRoundedRectangle (body, 6.0f);
    g.setColour (colours::goldLight);
    g.drawRoundedRectangle (body, 6.0f, 1.5f);
}

void ChiocciaLookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    auto r = Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (colours::chocolate);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (colours::goldLight);
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
    g.setColour (colours::cream);
    g.setFont (font (15.0f));
    g.drawFittedText (text, r.reduced (8.0f, 4.0f).toNearestInt(), Justification::centredLeft, 4);
}

Rectangle<int> ChiocciaLookAndFeel::getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea)
{
    const int w = std::min (320, (int) GlyphArrangement::getStringWidth (font (15.0f), tipText) + 20);
    const int lines = (int) std::ceil (GlyphArrangement::getStringWidth (font (15.0f), tipText) / 300.0f);
    const int h = 10 + 18 * std::max (1, std::min (4, lines));
    return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                           screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h)
        .constrainedWithin (parentArea);
}
} // namespace chioccia::ui

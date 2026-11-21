// Chioccia — look and feel (original artwork drawn with vector graphics, no bitmap assets)
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chioccia::ui
{
namespace colours
{
    const juce::Colour skyTop      { 0xff8fd3f4 };
    const juce::Colour skyBottom   { 0xfffdf3d7 };
    const juce::Colour ink         { 0xff3b2412 };   // dark brown outlines / text
    const juce::Colour cream       { 0xfffff4dc };
    const juce::Colour amber       { 0xffffb300 };
    const juce::Colour gold        { 0xfff2b42c };
    const juce::Colour goldLight   { 0xffffe58a };
    const juce::Colour goldDark    { 0xffa8650a };
    const juce::Colour chocolate   { 0xff4a2c17 };
    const juce::Colour plate       { 0xffe6f2f7 };
    const juce::Colour red         { 0xffe2312b };
    const juce::Colour metal       { 0xffc7ccd1 };
} // namespace colours

/** Component property keys understood by the look and feel. */
namespace style
{
    inline const juce::Identifier key { "chiocciaStyle" };   // "bigKnob", "smallKnob", "trimKnob", "bat", "batInverted", "pill", "segment", "mix"
    inline const juce::Identifier segmentLeft  { "segLeft" };
    inline const juce::Identifier segmentRight { "segRight" };
}

class ChiocciaLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ChiocciaLookAndFeel();

    static juce::Font font (float height);
    static juce::Typeface::Ptr typeface();

    /** Text drawn as a path with an outline (cartoon style), centred in area. */
    static void drawOutlinedText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float height,
                                  juce::Colour fill, juce::Colour outline, float outlineWidth,
                                  juce::Justification just = juce::Justification::centred);

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawBubble (juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>& tip, const juce::Rectangle<float>& body) override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;

private:
    void drawKnob (juce::Graphics&, juce::Rectangle<float> area, float angle, bool big, bool hover);
};
} // namespace chioccia::ui

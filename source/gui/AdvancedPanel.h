// Chioccia — advanced panel (overlay) (original code)
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace chioccia { class ChiocciaProcessor; }

namespace chioccia::ui
{
class AdvancedPanel final : public juce::Component, private juce::Timer
{
public:
    explicit AdvancedPanel (ChiocciaProcessor& processor);
    ~AdvancedPanel() override { stopTimer(); }

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onClose;

private:
    void timerCallback() override;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct Knob { juce::Slider slider; std::unique_ptr<SliderAttachment> attachment; juce::String title; };
    struct Combo { juce::ComboBox box; std::unique_ptr<ComboAttachment> attachment; juce::String title; };

    ChiocciaProcessor& processor;
    std::array<Knob, 6> knobs;
    std::array<Combo, 5> combos;
    juce::ToggleButton hpfOn { "HPF ON" }, compare { "LEVEL-MATCHED COMPARE" };
    std::unique_ptr<ButtonAttachment> hpfOnAttachment, compareAttachment;
    juce::TextButton close { "X" };
    juce::String info;
};
} // namespace chioccia::ui

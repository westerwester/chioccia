// Chioccia — static background (rack panel, sky, artwork, labels). Buffered to an image by the editor.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chioccia::ui
{
class FacePlate final : public juce::Component
{
public:
    FacePlate();
    void paint (juce::Graphics&) override;
};
} // namespace chioccia::ui

// Chioccia — original vector artwork helpers (chickens, golden eggs, feathers, clouds, nest, screws)
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace chioccia::ui::art
{
/** A round cartoon hen ("chioccia"). box is the drawing area; the art keeps its own aspect ratio. */
void drawChicken (juce::Graphics&, juce::Rectangle<float> box, bool faceLeft, bool beakOpen);
void drawGoldenEgg (juce::Graphics&, juce::Point<float> centre, float width, float rotation);
void drawFeather (juce::Graphics&, juce::Point<float> centre, float length, float rotation);
void drawCloud (juce::Graphics&, juce::Rectangle<float> area);
void drawNest (juce::Graphics&, juce::Rectangle<float> area);
void drawScrew (juce::Graphics&, juce::Point<float> centre, float radius, float angle);
} // namespace chioccia::ui::art

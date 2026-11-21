// Chioccia — layout in "design units" (the editor is drawn on a 1000 x 340 grid and scaled).
#pragma once

#include <juce_graphics/juce_graphics.h>

namespace chioccia::ui::layout
{
inline constexpr float kWidth = 1000.0f;
inline constexpr float kHeight = 340.0f;

using R = juce::Rectangle<float>;

inline const R face           { 40.0f, 8.0f, 920.0f, 324.0f };
inline const R chickenLeft    { 14.0f, 100.0f, 178.0f, 242.0f };
inline const R chickenRight   { 852.0f, 108.0f, 150.0f, 195.0f };
inline const R nest           { 846.0f, 250.0f, 120.0f, 74.0f };

inline const R title          { 330.0f, 10.0f, 340.0f, 64.0f };
inline const R subtitle       { 380.0f, 70.0f, 240.0f, 20.0f };

inline const R gainKnob       { 212.0f, 116.0f, 172.0f, 172.0f };
inline const R gainLabel      { 226.0f, 286.0f, 144.0f, 28.0f };
inline const R prKnob         { 632.0f, 116.0f, 172.0f, 172.0f };
inline const R prLabel        { 618.0f, 286.0f, 200.0f, 28.0f };

inline const R vu             { 396.0f, 96.0f, 208.0f, 122.0f };
inline const R vuMode         { 446.0f, 222.0f, 108.0f, 22.0f };
inline const R mixLabel       { 384.0f, 248.0f, 232.0f, 18.0f };
inline const R mix            { 384.0f, 266.0f, 232.0f, 32.0f };
inline const R mixScale       { 384.0f, 298.0f, 232.0f, 16.0f };

inline const R compSwitch     { 92.0f, 222.0f, 52.0f, 58.0f };
inline const R limitLabel     { 58.0f, 199.0f, 120.0f, 22.0f };
inline const R compressLabel  { 48.0f, 281.0f, 140.0f, 22.0f };

inline const R outputKnob     { 776.0f, 92.0f, 74.0f, 74.0f };
inline const R outputLabel    { 758.0f, 66.0f, 110.0f, 24.0f };
inline const R outputValue    { 758.0f, 166.0f, 110.0f, 18.0f };
inline const R powerSwitch    { 787.0f, 214.0f, 52.0f, 60.0f };
inline const R onLabel        { 763.0f, 190.0f, 100.0f, 22.0f };
inline const R powerLabel     { 763.0f, 276.0f, 100.0f, 22.0f };

inline const R presetBox      { 56.0f, 16.0f, 196.0f, 26.0f };
inline const R advancedButton { 820.0f, 16.0f, 124.0f, 26.0f };
inline const R advancedPanel  { 226.0f, 54.0f, 548.0f, 262.0f };
} // namespace chioccia::ui::layout

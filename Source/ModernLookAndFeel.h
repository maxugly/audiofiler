#pragma once

#include <JuceHeader.h>
#include "Config.h" // Include Config.h

class ModernLookAndFeel : public juce::LookAndFeel_V4 {
public:
  ModernLookAndFeel() {
    setColour (juce::TextButton::buttonColourId, Config::buttonBaseColour);
    setColour (juce::TextButton::buttonOnColourId, Config::buttonOnColour);
    setColour (juce::TextButton::textColourOffId, Config::buttonTextColour);
    setColour (juce::TextButton::textColourOnId, Config::buttonTextColour); } // Use same text colour for on/off states

  void setBaseOffColor(juce::Colour color) { setColour(juce::TextButton::buttonColourId, color); }
  void setBaseOnColor(juce::Colour color) { setColour(juce::TextButton::buttonOnColourId, color); }
  void setTextColor(juce::Colour color) { setColour(juce::TextButton::textColourOffId, color); setColour(juce::TextButton::textColourOnId, color); }

  void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    auto cornerSize = Config::buttonCornerRadius;
    auto outlineThickness = Config::buttonOutlineThickness;
    auto bounds = button.getLocalBounds().toFloat().reduced (outlineThickness / 2.0f);

    juce::Colour currentBackgroundColour;
    if (!button.isEnabled()) {
        currentBackgroundColour = Config::disabledButtonBackgroundColour;
    } else {
        currentBackgroundColour = backgroundColour;
        if (shouldDrawButtonAsHighlighted)
            currentBackgroundColour = currentBackgroundColour.brighter (Config::buttonHighlightedBrightnessFactor);
        if (shouldDrawButtonAsDown)
            currentBackgroundColour = currentBackgroundColour.darker (Config::buttonPressedDarknessFactor);
    }

    g.setColour (currentBackgroundColour);
    g.fillRoundedRectangle (bounds, cornerSize);

    g.setColour (Config::buttonOutlineColour);
    g.drawRoundedRectangle (bounds, cornerSize, outlineThickness); }

  juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override {
    juce::Font font = LookAndFeel_V4::getTextButtonFont (button, buttonHeight);
    if (button.getButtonText() == juce::CharPointer_UTF8 ("\xe2\x96\xb6") ||
      button.getButtonText() == juce::CharPointer_UTF8 ("\xe2\x8f\xb8")) {
      font.setHeight (buttonHeight * Config::buttonPlayPauseTextHeightScale); }
    else {
      font.setHeight (buttonHeight * Config::buttonTextHeightScale); }
    return font; }

  void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                      bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);
    
    juce::Colour textColourToUse;
    if (!button.isEnabled()) {
        textColourToUse = Config::disabledButtonTextColour;
    } else {
        textColourToUse = button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                                  : juce::TextButton::textColourOffId);
    }
    g.setColour (textColourToUse);
    g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true); }

  void fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::disabledButtonBackgroundColour);
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
      }
      g.fillRect(0, 0, width, height);
  }

  void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::disabledButtonTextColour.withAlpha(0.5f)); // Faint outline for disabled
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
      }
      g.drawRect(0, 0, width, height, 1); // 1-pixel thick border
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel) };

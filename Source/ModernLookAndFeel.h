#ifndef AUDIOFILER_MODERNLOOKANDFEEL_H
#define AUDIOFILER_MODERNLOOKANDFEEL_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h" 

class ModernLookAndFeel : public juce::LookAndFeel_V4 {
public:

  ModernLookAndFeel() {

    setColour (juce::TextButton::buttonColourId, Config::Colors::Button::base);

    setColour (juce::TextButton::buttonOnColourId, Config::Colors::Button::on);

    setColour (juce::TextButton::textColourOffId, Config::Colors::Button::text);

    setColour (juce::TextButton::textColourOnId, Config::Colors::Button::text); } 

  void setBaseOffColor(juce::Colour color) { setColour(juce::TextButton::buttonColourId, color); }

  void setBaseOnColor(juce::Colour color) { setColour(juce::TextButton::buttonOnColourId, color); }

  void setTextColor(juce::Colour color) { setColour(juce::TextButton::textColourOffId, color); setColour(juce::TextButton::textColourOnId, color); }

  void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    auto cornerSize = Config::Layout::buttonCornerRadius;
    auto outlineThickness = Config::Layout::buttonOutlineThickness;
    auto bounds = button.getLocalBounds().toFloat().reduced (outlineThickness / 2.0f);

    juce::Colour currentBackgroundColour;
    if (!button.isEnabled()) {
        currentBackgroundColour = Config::Colors::Button::disabledBackground;
    } else {
        currentBackgroundColour = backgroundColour;
        if (shouldDrawButtonAsHighlighted)
            currentBackgroundColour = currentBackgroundColour.brighter (Config::Animation::buttonHighlightedBrightness);
        if (shouldDrawButtonAsDown)
            currentBackgroundColour = currentBackgroundColour.darker (Config::Animation::buttonPressedDarkness);
    }

    g.setColour (currentBackgroundColour);
    g.fillRoundedRectangle (bounds, cornerSize);

    g.setColour (Config::Colors::Button::outline);
    g.drawRoundedRectangle (bounds, cornerSize, outlineThickness); }

  juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override {
    juce::Font font = LookAndFeel_V4::getTextButtonFont (button, buttonHeight);
    if (button.getButtonText() == Config::Labels::playButton ||
      button.getButtonText() == Config::Labels::stopButton) {
      font.setHeight (buttonHeight * Config::Layout::Text::buttonPlayPauseHeightScale); }
    else {
      font.setHeight (buttonHeight * Config::Layout::Text::buttonHeightScale); }
    return font; }

  void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                      bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);

    juce::Colour textColourToUse;
    if (!button.isEnabled()) {
        textColourToUse = Config::Colors::Button::disabledText;
    } else {
        textColourToUse = button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                                  : juce::TextButton::textColourOffId);
    }
    g.setColour (textColourToUse);
    g.drawText (button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true); }

  void fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::Colors::Button::disabledBackground);
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
      }
      g.fillRect(0, 0, width, height);
  }

  void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::Colors::Button::disabledText.withAlpha(0.5f)); 
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
      }
      g.drawRect(0, 0, width, height, Config::Layout::Text::editorOutlineThickness); 
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel) };

#endif 

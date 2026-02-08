#pragma once

#include <JuceHeader.h>
#include "Config.h" // Include Config.h

/**
 * @file ModernLookAndFeel.h
 * @brief Defines the custom look and feel for the application's UI components.
 *
 * This class inherits from `juce::LookAndFeel_V4` and overrides several of its
 * drawing methods to provide a consistent, modern, and customized visual style
 * for widgets like buttons and text editors. The styling parameters (colors, radii, etc.)
 * are sourced from the central `Config.h` file.
 */
class ModernLookAndFeel : public juce::LookAndFeel_V4 {
public:
  /**
     * @brief Constructs the ModernLookAndFeel object.
     *
     * In the constructor, we set the default colors for various button states
     * using the values defined in `Config.h`. This ensures that all buttons
     * created while this LookAndFeel is active will share the same base appearance.
     */
  ModernLookAndFeel() {
    setColour (juce::TextButton::buttonColourId, Config::buttonBaseColour);
    setColour (juce::TextButton::buttonOnColourId, Config::buttonOnColour);
    setColour (juce::TextButton::textColourOffId, Config::buttonTextColour);
    setColour (juce::TextButton::textColourOnId, Config::buttonTextColour); } // Use same text colour for on/off states

    //------------------------------------------------------------------------------------------------
    /** @name Color Setters
     *  Public methods to dynamically change the look and feel colors if needed.
     *  @{
     */

    /** @brief Sets the base background color for a button in its "off" state. */
  void setBaseOffColor(juce::Colour color) { setColour(juce::TextButton::buttonColourId, color); }
  /** @brief Sets the base background color for a button in its "on" (toggled) state. */
  void setBaseOnColor(juce::Colour color) { setColour(juce::TextButton::buttonOnColourId, color); }
  /** @brief Sets the text color for a button for both "on" and "off" states. */
  void setTextColor(juce::Colour color) { setColour(juce::TextButton::textColourOffId, color); setColour(juce::TextButton::textColourOnId, color); }

    /** @} */
    //------------------------------------------------------------------------------------------------

    /**
     * @brief Custom drawing logic for a button's background.
     * @param g The graphics context to draw into.
     * @param button The button being drawn.
     * @param backgroundColour The base background color for the current state (on/off).
     * @param shouldDrawButtonAsHighlighted True if the mouse is hovering over the button.
     * @param shouldDrawButtonAsDown True if the button is currently being pressed.
     *
     * This method is overridden to create rounded buttons with custom colors for different
     * interaction states. It handles the appearance for disabled, highlighted (hover), and
     * pressed states, sourcing colors and dimensions from `Config.h`.
     */
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

  /**
     * @brief Determines the font for a text button.
     * @param button The button for which to get the font.
     * @param buttonHeight The height of the button.
     * @return The appropriate font for the button's text.
     *
     * This override allows for custom font sizes based on the button's text. It uses a
     * larger font for the play/pause symbols to make them more visually prominent,
     * while other buttons use a smaller, standard text size. The sizes are scaled
     * relative to the button's height.
     */
  juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override {
    juce::Font font = LookAndFeel_V4::getTextButtonFont (button, buttonHeight);
    if (button.getButtonText() == Config::playButtonText ||
      button.getButtonText() == Config::stopButtonText) {
      font.setHeight (buttonHeight * Config::buttonPlayPauseTextHeightScale); }
    else {
      font.setHeight (buttonHeight * Config::buttonTextHeightScale); }
    return font; }

  /**
     * @brief Custom drawing logic for a button's text.
     * @param g The graphics context to draw into.
     * @param button The button being drawn.
     * @param shouldDrawButtonAsHighlighted True if the mouse is hovering over the button.
     * @param shouldDrawButtonAsDown True if the button is currently being pressed.
     *
     * This override is primarily to handle the text color for disabled buttons. When a
     * button is disabled, it uses a specific low-contrast color from `Config.h`.
     * Otherwise, it uses the default text color for the button's current on/off state.
     */
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

  /**
     * @brief Fills the background of a TextEditor.
     * @param g The graphics context to draw into.
     * @param width The width of the editor.
     * @param height The height of the editor.
     * @param textEditor The text editor being drawn.
     *
     * This override ensures the background of a disabled TextEditor matches the disabled
     * state of other widgets, providing a consistent visual language for interactivity.
     */
  void fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::disabledButtonBackgroundColour);
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
      }
      g.fillRect(0, 0, width, height);
  }

  /**
     * @brief Draws the outline of a TextEditor.
     * @param g The graphics context to draw into.
     * @param width The width of the editor.
     * @param height The height of the editor.
     * @param textEditor The text editor being drawn.
     *
     * This override customizes the editor's outline. For disabled editors, it draws a
     * faint, semi-transparent outline. For enabled editors, it uses the color specified
     * by the editor's `outlineColourId` and a thickness from `Config.h`.
     */
  void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override {
      if (!textEditor.isEnabled()) {
          g.setColour(Config::disabledButtonTextColour.withAlpha(0.5f)); // Faint outline for disabled
      } else {
          g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
      }
      g.drawRect(0, 0, width, height, Config::textEditorOutlineThickness); // Use Config for thickness
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel) };

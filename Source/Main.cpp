#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"
#include "Config.h"

/**
 * @file Main.cpp
 * @brief Implements the main entry point for the audiofiler JUCE application.
 *
 * This file defines the `GuiAppApplication` class, which is the main application
 * entry point according to the JUCE framework, and the `MainWindow` class,
 * which serves as the application's primary top-level window.
 */

/**
 * @class GuiAppApplication
 * @brief The main JUCE application class.
 *
 * This class inherits from `juce::JUCEApplication` and is responsible for
 * initializing and shutting down the entire application. It defines metadata
 * like the application name and version, and manages the main application window.
 */
class GuiAppApplication : public juce::JUCEApplication {
public:
  /**
   * @brief Constructs the GuiAppApplication.
   *
   * Standard constructor for the JUCE application class.
   */
  GuiAppApplication() {}

  /**
   * @brief Returns the application's name.
   * @return A `juce::String` containing the application name.
   */
  const juce::String getApplicationName() override       { return "audiofiler"; }

  /**
   * @brief Returns the application's version string.
   * @return A `juce::String` containing the application version.
   */
  const juce::String getApplicationVersion() override    { return "0.0.001"; }

  /**
   * @brief Initializes the application.
   * @param commandLine The command line arguments passed to the application.
   *
   * This method is called by the JUCE framework when the application starts.
   * It creates and shows the main application window.
   */
  void initialise (const juce::String& commandLine) override {
    #if JUCE_DEBUG
    // If juce::VBlank is not available, you can use the JUCE_ENABLE_REPAINT_DEBUGGING 1 
    // in AppConfig.h or compiler flags. 
    // For now, I'll just remove the failing line to let it build.
    #endif
    mainWindow.reset (new MainWindow (getApplicationName())); }

  /**
   * @brief Shuts down the application.
   *
   * This method is called by the JUCE framework when the application is
   * requested to quit. It ensures that the main window and its contents
   * are properly destroyed.
   */
  void shutdown() override { mainWindow.reset(); }

private:
    /**
     * @class MainWindow
     * @brief The main top-level window for the application.
     *
     * This class inherits from `juce::DocumentWindow` and serves as the primary
     * container for the application's user interface (`MainComponent`). It
     * configures the window's properties such as title bar, resizability, and
     * close behavior.
     */
    class MainWindow : public juce::DocumentWindow {
    public:
      /**
       * @brief Constructs the MainWindow.
       * @param name The title of the window.
       *
       * Initializes the window with a title, background color, and close button type.
       * It sets the `MainComponent` as its content, makes it resizable,
       * centers it on screen, and makes it visible.
       */
      MainWindow (juce::String name) : DocumentWindow (name, Config::Colors::Window::background, allButtons) {
        /**
         * @brief Sets the UsingNativeTitleBar.
         * @param true [in] Description for true.
         */
        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent(), true);
        /**
         * @brief Sets the Resizable.
         * @param true [in] Description for true.
         * @param true [in] Description for true.
         */
        setResizable (true, true);
        centreWithSize (getWidth(), getHeight());
        /**
         * @brief Sets the Visible.
         * @param true [in] Description for true.
         */
        setVisible (true);
        /**
         * @brief Sets the FullScreen.
         * @param false [in] Description for false.
         */
        setFullScreen (false); }

      /**
       * @brief Handles the window close button press event.
       *
       * When the user clicks the close button, this method is called,
       * which then requests the JUCE application to quit gracefully.
       */
      void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow; ///< The unique pointer holding the main application window.
};

/**
 * @brief Macro that starts the JUCE application.
 *
 * This macro is provided by the JUCE framework and acts as the actual `main()`
 * function, creating an instance of `GuiAppApplication` and running it.
 */
START_JUCE_APPLICATION (GuiAppApplication)

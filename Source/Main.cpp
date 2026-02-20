#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"
#include "Utils/Config.h"

class GuiAppApplication : public juce::JUCEApplication {
public:

  GuiAppApplication() {}

  const juce::String getApplicationName() override       { return "audiofiler"; }

  const juce::String getApplicationVersion() override    { return "0.0.001"; }

  void initialise (const juce::String& commandLine) override {
    #if JUCE_DEBUG

    #endif
    mainWindow.reset (new MainWindow (getApplicationName())); }

  void shutdown() override { 
    mainWindow.reset(); 
  }

private:

    class MainWindow : public juce::DocumentWindow {
    public:

      MainWindow (juce::String name) : DocumentWindow (name, Config::Colors::Window::background, juce::DocumentWindow::allButtons) {

        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent(), true);

        setResizable (true, true);
        centreWithSize (getWidth(), getHeight());

        setVisible (true);

        setFullScreen (false); }

      void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow; 
};

START_JUCE_APPLICATION (GuiAppApplication)

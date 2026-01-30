#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

class GuiAppApplication : public juce::JUCEApplication
{
public:
    GuiAppApplication() {}
    const juce::String getApplicationName() override       { return "Sorta++"; }
    const juce::String getApplicationVersion() override    { return "0.0.001"; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override { mainWindow.reset(); }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name) : DocumentWindow (name, juce::Colours::black, allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (GuiAppApplication)
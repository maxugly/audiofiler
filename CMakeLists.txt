cmake_minimum_required(VERSION 3.15)

project(audiofiler VERSION 0.0.0007)

# Enable testing infrastructure
enable_testing()

option(BUILD_TESTS_ONLY "Build tests only" OFF)

# Attempt to disable juceaide build to avoid X11 dependency
set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "" FORCE)

include(cmake/CPM.cmake)
CPMAddPackage(
        NAME juce
        GITHUB_REPOSITORY juce-framework/JUCE
        GIT_TAG 8.0.12
)

if (NOT BUILD_TESTS_ONLY)
    message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
    if (UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)

        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

        include_directories(
            ${GTK3_INCLUDE_DIRS}
        )

        link_directories(${GTK3_LIBRARY_DIRS})
        add_definitions(${GTK3_CFLAGS_OTHER})
    endif ()

    juce_add_gui_app(audiofiler
            PRODUCT_NAME "audiofiler"
    )

    target_compile_definitions(audiofiler PUBLIC JUCE_DEBUG=1)

    target_compile_definitions(audiofiler PRIVATE
            JUCE_USE_MP3AUDIOFORMAT=1
            JUCE_USE_FLAC_AUDIO_FORMAT=1
            JUCE_USE_OGGVORBIS_AUDIO_FORMAT=1
            JUCE_USE_WINDOWS_MEDIA_FORMAT=1
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_LOAD_CURL_SYMBOLS_LAZILY=1
            JUCE_ALSA=1
            JUCE_JACK=1
    )

    target_sources(audiofiler PRIVATE
            # Root
            Source/Main.cpp
            Source/MainComponent.h
            Source/MainComponent.cpp
            Source/MainDomain.h

            # Utils
            Source/Utils/Config.h
            Source/Utils/Config.cpp
            Source/Utils/TimeUtils.h
            Source/Utils/TimeUtils.cpp
            Source/Utils/UIAnimationHelper.h
            Source/Utils/CoordinateMapper.h
            Source/Utils/TimeEntryHelpers.h
            Source/Utils/TimeEntryHelpers.cpp
            Source/Utils/PlaybackHelpers.h
            Source/Utils/PlaybackHelpers.cpp

            # Core
            Source/Core/AudioPlayer.h
            Source/Core/AudioPlayer.cpp
            Source/Core/SessionState.h
            Source/Core/SessionState.cpp
            Source/Core/AppEnums.h
            Source/Core/SilenceAnalysisWorker.h
            Source/Core/SilenceAnalysisWorker.cpp
            Source/Core/WaveformManager.h
            Source/Core/WaveformManager.cpp
            Source/Core/FileMetadata.h

            # Workers
            Source/Workers/SilenceWorkerClient.h
            Source/Workers/SilenceDetector.h
            Source/Workers/SilenceDetector.cpp
            Source/Workers/SilenceAnalysisAlgorithms.h
            Source/Workers/SilenceAnalysisAlgorithms.cpp
            Source/Workers/SilenceDetectionLogger.h
            Source/Workers/SilenceDetectionLogger.cpp

            # Presenters
            Source/Presenters/ControlButtonsPresenter.h
            Source/Presenters/ControlButtonsPresenter.cpp
            Source/Presenters/ControlStatePresenter.h
            Source/Presenters/ControlStatePresenter.cpp
            Source/Presenters/CutButtonPresenter.h
            Source/Presenters/CutButtonPresenter.cpp
            Source/Presenters/CutPresenter.h
            Source/Presenters/CutPresenter.cpp
            Source/Presenters/CutResetPresenter.h
            Source/Presenters/CutResetPresenter.cpp
            Source/Presenters/PlaybackTextPresenter.h
            Source/Presenters/PlaybackTextPresenter.cpp
            Source/Presenters/SilenceThresholdPresenter.h
            Source/Presenters/SilenceThresholdPresenter.cpp
            Source/Presenters/StatsPresenter.h
            Source/Presenters/StatsPresenter.cpp
            Source/Presenters/TransportPresenter.h
            Source/Presenters/TransportPresenter.cpp
            Source/Presenters/SilenceDetectionPresenter.h
            Source/Presenters/SilenceDetectionPresenter.cpp
            Source/Presenters/RepeatPresenter.h
            Source/Presenters/RepeatPresenter.cpp
            Source/Presenters/PlaybackTimerManager.h
            Source/Presenters/PlaybackTimerManager.cpp
            Source/Presenters/PlaybackRepeatController.h
            Source/Presenters/PlaybackRepeatController.cpp

            # UI
            Source/UI/ControlPanel.h
            Source/UI/ControlPanel.cpp
            Source/UI/MouseHandler.h
            Source/UI/MouseHandler.cpp
            Source/UI/KeybindHandler.h
            Source/UI/KeybindHandler.cpp
            Source/UI/LayoutManager.h
            Source/UI/LayoutManager.cpp
            Source/UI/FocusManager.h
            Source/UI/FocusManager.cpp

            # UI/Components
            Source/UI/Components/TransportButton.h
            Source/UI/Components/TransportButton.cpp
            Source/UI/Components/TransportStrip.h
            Source/UI/Components/TransportStrip.cpp
            Source/UI/Components/MarkerStrip.h
            Source/UI/Components/MarkerStrip.cpp

            # UI/Views
            Source/UI/Views/CutLayerView.h
            Source/UI/Views/CutLayerView.cpp
            Source/UI/Views/WaveformView.h
            Source/UI/Views/WaveformView.cpp
            Source/UI/Views/ZoomView.h
            Source/UI/Views/ZoomView.cpp
            Source/UI/Views/PlaybackCursorView.h
            Source/UI/Views/PlaybackCursorView.cpp
            Source/UI/Views/PlaybackCursorGlow.h
            Source/UI/Views/PlaybackCursorGlow.cpp

            # UI/LookAndFeel
            Source/UI/LookAndFeel/ModernLookAndFeel.h
    )

    target_link_libraries(audiofiler PRIVATE
            juce::juce_audio_utils
            juce::juce_audio_formats
            juce::juce_audio_devices
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
            juce::juce_opengl
            ${GTK3_LIBRARIES}
    )

    juce_generate_juce_header(audiofiler)
    target_include_directories(audiofiler PRIVATE Source)
endif()

# Tests
add_executable(tests
    Tests/TestMain.cpp
    Tests/TimeUtilsTest.cpp
    Source/Utils/TimeUtils.cpp
    Source/Utils/PlaybackHelpers.cpp
    Tests/PlaybackHelpersTest.cpp
    Tests/SecurityFixTest.cpp
    Source/Core/AudioPlayer.cpp
    Tests/AudioPlayerTest.cpp
    Source/Utils/Config.cpp
    Source/Core/SessionState.cpp
    Source/Workers/SilenceAnalysisAlgorithms.cpp
    Source/Core/SilenceAnalysisWorker.cpp
    Source/Workers/SilenceDetectionLogger.cpp
    Tests/SilenceAnalysisTest.cpp
)

target_include_directories(tests PRIVATE Source)

target_compile_definitions(tests PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_HEADLESS=1
    JUCE_UNIT_TESTS=1
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_DONT_DECLARE_PROJECTINFO=1
)

target_link_libraries(tests PRIVATE
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_graphics
    juce::juce_events
)

# Register tests with CTest
add_test(NAME AllTests COMMAND tests)

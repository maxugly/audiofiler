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
            Source/Main.cpp
            Source/Config.h
            Source/Config.cpp
            Source/MainComponent.h
            Source/MainComponent.cpp
            Source/TimeUtils.h
            Source/TimeUtils.cpp
            Source/AudioPlayer.h
            Source/AudioPlayer.cpp
            Source/MainDomain.h
            Source/SessionState.h
            Source/SessionState.cpp
            Source/FileMetadata.h
            Source/CoordinateMapper.h
            Source/SilenceWorkerClient.h
            Source/ControlPanel.h
            Source/ControlPanel.cpp
            Source/AppEnums.h
            Source/CutLayerView.h
            Source/CutLayerView.cpp
            Source/CutPresenter.h
            Source/CutPresenter.cpp
            Source/SilenceDetector.h
            Source/SilenceDetector.cpp
            Source/ModernLookAndFeel.h
            Source/LayoutManager.h
            Source/LayoutManager.cpp
            Source/WaveformManager.h
            Source/WaveformManager.cpp
            Source/WaveformView.h
            Source/WaveformView.cpp
            Source/KeybindHandler.h
            Source/KeybindHandler.cpp
            Source/ZoomView.h
            Source/ZoomView.cpp
            Source/ControlStatePresenter.h
            Source/ControlStatePresenter.cpp
            Source/TransportPresenter.h
            Source/TransportPresenter.cpp
            Source/SilenceDetectionPresenter.h
            Source/SilenceDetectionPresenter.cpp
            Source/SilenceAnalysisWorker.h
            Source/SilenceAnalysisWorker.cpp
            Source/SilenceAnalysisAlgorithms.h
            Source/SilenceAnalysisAlgorithms.cpp
            Source/SilenceDetectionLogger.h
            Source/SilenceDetectionLogger.cpp
            Source/ControlPanelCopy.h
            Source/ControlButtonsPresenter.h
            Source/ControlButtonsPresenter.cpp
            Source/SilenceThresholdPresenter.h
            Source/SilenceThresholdPresenter.cpp
            Source/CutResetPresenter.h
            Source/CutResetPresenter.cpp

            Source/CutButtonPresenter.h
            Source/CutButtonPresenter.cpp
            Source/RepeatPresenter.h
            Source/RepeatPresenter.cpp
            Source/RepeatButton.h
            Source/RepeatButton.cpp
            Source/PlaybackRepeatController.h
            Source/PlaybackRepeatController.cpp
            Source/PlaybackTextPresenter.h
            Source/PlaybackTextPresenter.cpp
            Source/TimeEntryHelpers.h
            Source/TimeEntryHelpers.cpp
            Source/StatsPresenter.h
            Source/StatsPresenter.cpp
            Source/MouseHandler.h
            Source/MouseHandler.cpp
            Source/FocusManager.h
            Source/FocusManager.cpp
            Source/PlaybackHelpers.h
            Source/PlaybackHelpers.cpp
            Source/PlaybackCursorGlow.h
            Source/PlaybackCursorGlow.cpp
            Source/PlaybackCursorView.h
            Source/PlaybackCursorView.cpp
            Source/PlaybackTimerManager.h
            Source/PlaybackTimerManager.cpp
            Source/UIAnimationHelper.h
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
endif()

# Tests
add_executable(tests
    Tests/TestMain.cpp
    Tests/TimeUtilsTest.cpp
    Source/TimeUtils.cpp
    Source/PlaybackHelpers.cpp
    Tests/PlaybackHelpersTest.cpp
    Tests/SecurityFixTest.cpp
    Source/AudioPlayer.cpp
    Tests/AudioPlayerTest.cpp
    Source/Config.cpp
    Source/SessionState.cpp
    Source/SilenceAnalysisAlgorithms.cpp
    Source/SilenceAnalysisWorker.cpp
    Source/SilenceDetectionLogger.cpp
    Tests/SilenceAnalysisTest.cpp
)

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

cmake_minimum_required(VERSION 3.15)

project(audiofiler VERSION 0.0.0001)

# Ensure these are globally defined to prevent any target from picking up defaults
add_compile_definitions(JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)

option(BUILD_HEADLESS_TESTS "Build only headless tests" OFF)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
if (UNIX AND NOT APPLE AND NOT BUILD_HEADLESS_TESTS)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    include_directories(
            ${GTK3_INCLUDE_DIRS}
    )

    link_directories(${GTK3_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER})
endif ()

include(cmake/CPM.cmake)
CPMAddPackage(
        NAME juce
        GITHUB_REPOSITORY juce-framework/JUCE
        GIT_TAG 8.0.12
)

if (NOT BUILD_HEADLESS_TESTS)
    juce_add_gui_app(audiofiler
            PRODUCT_NAME "audiofiler"
    )

    target_compile_definitions(audiofiler PUBLIC JUCE_DEBUG=1)

    target_compile_definitions(audiofiler PRIVATE
            JUCE_USE_MP3AUDIOFORMAT=1
            JUCE_USE_FLAC_AUDIO_FORMAT=1
            JUCE_USE_OGGVORBIS_AUDIO_FORMAT=1
            JUCE_USE_WINDOWS_MEDIA_FORMAT=1
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
            Source/AudioPlayer.h
            Source/AudioPlayer.cpp
            Source/PlaybackHelpers.h
            Source/ControlPanel.h
            Source/ControlPanel.cpp
            Source/AppEnums.h
            Source/SilenceDetector.h
            Source/SilenceDetector.cpp
            Source/LayoutManager.h
            Source/LayoutManager.cpp
            Source/WaveformRenderer.h
            Source/WaveformRenderer.cpp
            Source/KeybindHandler.h
            Source/KeybindHandler.cpp
            Source/LoopPresenter.h
            Source/LoopPresenter.cpp
            Source/ControlStatePresenter.h
            Source/ControlStatePresenter.cpp
            Source/TransportPresenter.h
            Source/TransportPresenter.cpp
            Source/SilenceDetectionPresenter.h
            Source/SilenceDetectionPresenter.cpp
            Source/SilenceAnalysisWorker.h
            Source/SilenceAnalysisWorker.cpp
            Source/SilenceDetectionLogger.h
            Source/SilenceDetectionLogger.cpp
            Source/ControlPanelCopy.h
            Source/ControlButtonsPresenter.h
            Source/ControlButtonsPresenter.cpp
            Source/SilenceThresholdPresenter.h
            Source/SilenceThresholdPresenter.cpp
            Source/LoopEditorPresenter.h
            Source/LoopEditorPresenter.cpp
            Source/LoopButtonPresenter.h
            Source/LoopButtonPresenter.cpp
            Source/LoopResetPresenter.h
            Source/LoopResetPresenter.cpp
            Source/LoopButton.h
            Source/LoopButton.cpp
            Source/PlaybackLoopController.h
            Source/PlaybackLoopController.cpp
            Source/PlaybackTextPresenter.h
            Source/PlaybackTextPresenter.cpp
            Source/PlaybackCursorGlow.h
            Source/PlaybackCursorGlow.cpp
            Source/StatsPresenter.h
            Source/StatsPresenter.cpp
            Source/MouseHandler.h
            Source/MouseHandler.cpp
            Source/FocusManager.h
            Source/FocusManager.cpp
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

# Use add_executable instead of juce_add_console_app to verify if it simplifies definition propagation
add_executable(audiofiler_tests
    Source/Tests/PlaybackHelpersTest.cpp
    Source/PlaybackHelpers.h
)

target_compile_definitions(audiofiler_tests PRIVATE
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    JUCE_MODULE_AVAILABLE_juce_core=1
    JUCE_STANDALONE_APPLICATION=1
)

target_link_libraries(audiofiler_tests PRIVATE
    juce::juce_core
)

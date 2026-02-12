cmake_minimum_required(VERSION 3.15)

project(Sorta VERSION 0.0.0001)

option(BUILD_TESTS_ONLY "Build only tests" OFF)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")

if (NOT BUILD_TESTS_ONLY)
    if (UNIX AND NOT APPLE)
        find_package(PkgConfig REQUIRED)

        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

        include_directories(
                #/usr/include/webkitgtk-4.1
                #/usr/include/libsoup-3.0
                ${GTK3_INCLUDE_DIRS}
                /usr/include/gtk-3.0
                /usr/include/gtk-3.0
                /usr/lib/glib-2.0/include
                /usr/include/pango-1.0
                /usr/include/harfbuzz
                /usr/include/cairo
                /usr/include/gdk-pixbuf-2.0
                /usr/include/atk-1.0
        )

        link_directories(${GTK3_LIBRARY_DIRS})
        add_definitions(${GTK3_CFLAGS_OTHER})
    endif ()
endif()

include(cmake/CPM.cmake)
CPMAddPackage(
        NAME juce
        GITHUB_REPOSITORY juce-framework/JUCE
        GIT_TAG 8.0.12
        OPTIONS "JUCE_BUILD_HELPER_TOOLS OFF" "JUCE_BUILD_EXTRAS OFF"
)

set(SORTA_SOURCES
        Source/MainComponent.h
        Source/MainComponent.cpp
        Source/AudioPlayer.h
        Source/AudioPlayer.cpp
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
        Source/SilenceAlgorithms.h
        Source/SilenceAlgorithms.cpp
)

if (NOT BUILD_TESTS_ONLY)
    juce_add_gui_app(Sorta
            PRODUCT_NAME "Sorta"
    )

    target_compile_definitions(Sorta PUBLIC JUCE_DEBUG=1)

    target_compile_definitions(Sorta PRIVATE
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

    target_sources(Sorta PRIVATE
            Source/Main.cpp
            ${SORTA_SOURCES}
    )

    target_link_libraries(Sorta PRIVATE
            juce::juce_audio_utils
            juce::juce_audio_formats
            juce::juce_audio_devices
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
            ${GTK3_LIBRARIES}
    )

    juce_generate_juce_header(Sorta)
endif()

# ==============================================================================
# Testing Infrastructure
# ==============================================================================

enable_testing()

CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY catchorg/Catch2
        GIT_TAG v3.4.0
)

# Create a test executable that only includes the logic to be tested
add_executable(SortaTests
        Source/Tests/SilenceAnalysisTest.cpp
        Source/SilenceAlgorithms.cpp
)

# Link against minimal JUCE modules (no GUI)
target_link_libraries(SortaTests PRIVATE
        Catch2::Catch2WithMain
        juce::juce_core
        juce::juce_audio_basics
)

# Ensure the test executable has correct include paths
target_include_directories(SortaTests PUBLIC Source)

# Use minimal definitions for tests
target_compile_definitions(SortaTests PRIVATE
        JUCE_DEBUG=1
)

# Register the test with CTest
include(CTest)
add_test(NAME AllTests COMMAND SortaTests)

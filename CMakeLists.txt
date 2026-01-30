cmake_minimum_required(VERSION 3.15)

project(Sorta VERSION 0.0.0001)

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

include(cmake/CPM.cmake)
CPMAddPackage(
        NAME juce
        GITHUB_REPOSITORY juce-framework/JUCE
        GIT_TAG origin/master
)

juce_add_gui_app(Sorta
        PRODUCT_NAME "Sorta"
)

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
        Source/MainComponent.h
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
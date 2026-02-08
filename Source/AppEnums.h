#pragma once

/**
 * @file AppEnums.h
 * @brief Defines several enumerations used for managing state within the application.
 *
 * This file provides a central place for application-wide enumerations,
 * which helps in creating more readable and maintainable state management logic.
 * Using enums instead of raw integers or booleans makes the code self-documenting
 * and reduces the risk of errors from using invalid state values.
 */
namespace AppEnums
{
    /**
     * @enum ViewMode
     * @brief Defines the different layout modes for the main user interface.
     */
    enum class ViewMode {
        Classic,  /**< A layout where controls are docked, and the waveform is in a separate area. */
        Overlay   /**< A layout where controls and displays are overlaid on top of the waveform view. */
    };

    /**
     * @enum PlacementMode
     * @brief Defines the state for placing loop points with a mouse click.
     *
     * This enum is used to track whether the application is currently waiting for
     * the user to click on the waveform to set a loop point.
     */
    enum class PlacementMode {
        None,     /**< No placement action is active. The mouse behaves normally. */
        LoopIn,   /**< The next click on the waveform will set the 'loop in' (start) point. */
        LoopOut   /**< The next click on the waveform will set the 'loop out' (end) point. */
    };

    /**
     * @enum ChannelViewMode
     * @brief Defines which audio channel(s) are currently being visualized.
     */
    enum class ChannelViewMode {
        Mono,     /**< Display only a single channel of the audio (e.g., the left channel). */
        Stereo    /**< Display both channels of the audio, typically one above the other. */
    };

    /**
     * @enum ThumbnailQuality
     * @brief Defines the different quality levels for rendering the audio waveform thumbnail.
     *
     * Higher quality settings may result in a more detailed waveform but can take
     * longer to generate and consume more memory.
     */
    enum class ThumbnailQuality {
        Low,      /**< A lower-resolution thumbnail for faster performance. */
        Medium,   /**< A standard-resolution thumbnail balancing performance and detail. */
        High      /**< A high-resolution thumbnail for maximum visual detail. */
    };
}
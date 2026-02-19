#ifndef AUDIOFILER_APPENUMS_H
#define AUDIOFILER_APPENUMS_H

/**
 * @file AppEnums.h
 * @brief Defines several enumerations used for managing state within the application.
 */
namespace AppEnums
{
    /**
     * @enum ViewMode
     * @brief Defines the different layout modes for the main user interface.
     */
    enum class ViewMode {
        Classic,
        Overlay
    };

    /**
     * @enum PlacementMode
     * @brief Defines the state for placing cut points with a mouse click.
     */
    enum class PlacementMode {
        None,
        CutIn,
        CutOut
    };

    /**
     * @enum ChannelViewMode
     */
    enum class ChannelViewMode {
        Mono,
        Stereo
    };

    /**
     * @enum ThumbnailQuality
     */
    enum class ThumbnailQuality {
        Low,
        Medium,
        High
    };
}

#endif 

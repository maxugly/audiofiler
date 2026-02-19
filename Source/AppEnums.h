#ifndef AUDIOFILER_APPENUMS_H
#define AUDIOFILER_APPENUMS_H

namespace AppEnums
{

    enum class ViewMode {
        Classic,
        Overlay
    };

    enum class PlacementMode {
        None,
        CutIn,
        CutOut
    };

    enum class ChannelViewMode {
        Mono,
        Stereo
    };

    enum class ThumbnailQuality {
        Low,
        Medium,
        High
    };

    enum class ActiveZoomPoint {
        None,
        In,
        Out
    };
}

#endif 

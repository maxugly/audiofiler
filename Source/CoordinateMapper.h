/**
 * @file CoordinateMapper.h
 * @brief Utility for pixel/sample conversion math.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_COORDINATEMAPPER_H
#define AUDIOFILER_COORDINATEMAPPER_H

#include <JuceHeader.h>

/**
 * @class CoordinateMapper
 * @brief Utility for converting between screen pixels and audio time.
 *
 * Centralises all coordinate transformation math to ensure consistency across
 * different layers (Waveform, CutLayer, PlaybackCursor) and to prevent
 * duplicate math in UI components.
 */
class CoordinateMapper
{
public:
    /**
     * @brief Converts a horizontal pixel position to audio time in seconds.
     * @param x The horizontal pixel position (relative to component).
     * @param componentWidth The total width of the rendering area.
     * @param totalDuration The total duration of the audio in seconds.
     * @return The corresponding time in seconds.
     */
    static double pixelsToSeconds(float x, float componentWidth, double totalDuration)
    {
        if (componentWidth <= 0.0f || totalDuration <= 0.0)
            return 0.0;

        return (static_cast<double>(x) / static_cast<double>(componentWidth)) * totalDuration;
    }

    /**
     * @brief Converts audio time in seconds to a horizontal pixel position.
     * @param seconds The audio time in seconds.
     * @param componentWidth The total width of the rendering area.
     * @param totalDuration The total duration of the audio in seconds.
     * @return The corresponding horizontal pixel position.
     */
    static float secondsToPixels(double seconds, float componentWidth, double totalDuration)
    {
        if (totalDuration <= 0.0 || componentWidth <= 0.0f)
            return 0.0f;

        return static_cast<float>((seconds / totalDuration) * static_cast<double>(componentWidth));
    }
};

#endif // AUDIOFILER_COORDINATEMAPPER_H

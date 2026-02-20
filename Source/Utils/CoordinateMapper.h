

#ifndef AUDIOFILER_COORDINATEMAPPER_H
#define AUDIOFILER_COORDINATEMAPPER_H

#include <JuceHeader.h>

class CoordinateMapper
{
public:

    static double pixelsToSeconds(float x, float componentWidth, double totalDuration)
    {
        if (componentWidth <= 0.0f || totalDuration <= 0.0)
            return 0.0;

        return (static_cast<double>(x) / static_cast<double>(componentWidth)) * totalDuration;
    }

    static float secondsToPixels(double seconds, float componentWidth, double totalDuration)
    {
        if (totalDuration <= 0.0 || componentWidth <= 0.0f)
            return 0.0f;

        return static_cast<float>((seconds / totalDuration) * static_cast<double>(componentWidth));
    }
};

#endif 

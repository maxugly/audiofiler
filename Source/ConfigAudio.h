#ifndef AUDIOFILER_CONFIG_AUDIO_H
#define AUDIOFILER_CONFIG_AUDIO_H

namespace Config {
    //==============================================================================
    // Audio Settings
    //==============================================================================
    namespace Audio {
         constexpr int thumbnailCacheSize = 5;
         constexpr int thumbnailSizePixels = 512;
         constexpr double keyboardSkipSeconds = 5.0;

         constexpr double loopStepHours = 3600.0;
         constexpr double loopStepMinutes = 60.0;
         constexpr double loopStepSeconds = 1.0;
         constexpr double loopStepMilliseconds = 0.01;
         constexpr double loopStepMillisecondsFine = 0.001;

         constexpr float silenceThresholdIn = 0.01f;
         constexpr float silenceThresholdOut = 0.01f;
         constexpr bool lockHandlesWhenAutoCutActive = false;
    }
}

#endif // AUDIOFILER_CONFIG_AUDIO_H

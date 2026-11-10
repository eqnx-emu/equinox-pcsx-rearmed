#pragma once

// The main loop runs on vsync at 60 Hz. NTSC games (~59.94 fps) run one core frame per
// refresh and the audio resampler absorbs the difference; PAL games (50 fps) would run
// 20 % fast that way, so they run fps/60 frames per refresh instead.
inline constexpr double kPalBelowFps = 55.0;

inline double framesPerRefreshFor(double fps) { return fps < kPalBelowFps ? fps / 60.0 : 1.0; }

// The frame rate the audio resampler is told: spread over 60 Hz refreshes, a PAL game
// makes per refresh the samples a 60 fps core would.
inline double audioFpsFor(double fps) { return fps < kPalBelowFps ? 60.0 : fps; }

// Adds one refresh worth of frames to the debt and returns how many core frames to run.
inline int framesToRun(double& debt, double framesPerRefresh) {
    debt += framesPerRefresh;
    int n = 0;
    while (debt >= 1.0) {
        debt -= 1.0;
        ++n;
    }
    return n;
}

#include "pacing.hpp"

#include "doctest/doctest.h"

TEST_CASE("NTSC runs one frame per refresh") {
    CHECK(framesPerRefreshFor(59.94) == 1.0);
    CHECK(audioFpsFor(59.94) == 59.94);
    double debt = 0;
    for (int i = 0; i < 100; ++i) CHECK(framesToRun(debt, framesPerRefreshFor(59.94)) == 1);
}

TEST_CASE("PAL runs 50 frames in 60 refreshes") {
    const double perRefresh = framesPerRefreshFor(50.0);
    CHECK(audioFpsFor(50.0) == 60.0);
    double debt = 0;
    int frames = 0, skipped = 0;
    for (int i = 0; i < 600; ++i) {
        const int n = framesToRun(debt, perRefresh);
        CHECK(n <= 1);
        frames += n;
        skipped += n == 0;
    }
    CHECK(frames >= 499);
    CHECK(frames <= 500);
    CHECK(skipped == 600 - frames);
}

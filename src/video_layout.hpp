#pragma once

#include "video_prefs.hpp"

struct Rect {
    int x, y, w, h;
    bool operator==(const Rect&) const = default;
};

// Logical columns and lines: the PlayStation's high-resolution modes count as half,
// 512/640 columns as 256/320 and 448..512 lines (interlaced) as 224..256.
unsigned logicalW(unsigned srcW);
unsigned logicalH(unsigned srcH);

// Where the game sits on the screen, centered. An empty frame gives w == 0.
Rect gameRect(Size size, unsigned srcW, unsigned srcH, int screenW, int screenH);

// Integer upscale per axis applied before the bilinear pass for Sharp; each >= 1.
struct Prescale {
    int x, y;
    bool operator==(const Prescale&) const = default;
};
Prescale sharpPrescale(unsigned srcW, unsigned srcH, const Rect& dst);

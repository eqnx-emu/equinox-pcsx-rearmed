#pragma once

enum class Lang { PtBR, EnUS };
enum class Str {
    Continue, SaveState, LoadState, Exit, StateSaved, StateLoaded,
    SaveFailed, NoState, OpenFromEquinox, RomNotFound, RomUnreadable,
    MissingTrack, RomUnsupported, BackToEquinox, DiscOf,
    Video, VideoTitle, Size, Smoothing, Filter, Border, UseDefault, Back,
    SizeOriginal, Size87, Size43, SizeFull, SmoothPixel, SmoothSharp, SmoothSoft,
    FilterNone, FilterScanlines, FilterNtsc, BorderEquinox, BorderCover, BorderTv,
    BorderBlack, VideoSaveFailed, DiscChanged, DiscSwapFailed,
    HintChoose, HintBackToGame, HintChange, LastState, VideoHint, DescNone, DescScanlines, DescNtsc
};

// UTF-8 text, never null. DiscOf is a format with two %u: the disc and the count.
const char* tr(Lang lang, Str s);

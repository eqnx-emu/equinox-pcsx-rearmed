#pragma once

#include <string>
#include <variant>

enum class RomError { None, NotFound, Unreadable, MissingTrack, Unsupported };

// A disc the core can open; the core reads it itself from path.
struct Disc {
    std::string path;
};

// Checks a PlayStation disc before the core opens it. Accepted: .cue, .chd, .pbp, .m3u,
// .iso, .img, .bin (any case). A .cue must name at least one track file and every track
// it names must exist beside it; an .m3u must list at least one disc and every listed
// disc must pass the same check (a listed .cue has its tracks checked too, an .m3u inside
// an .m3u is refused). Paths in a .cue or .m3u are relative to the file that lists them,
// unless absolute. A missing file gives NotFound, a missing track or listed disc
// MissingTrack, a .cue or .m3u that cannot be read or is over 64 KiB Unreadable.
std::variant<Disc, RomError> checkDisc(const std::string& path);

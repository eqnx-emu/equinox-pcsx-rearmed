#include "paths.hpp"

#include "doctest/doctest.h"

TEST_CASE("srm path uses the disc file name") {
    CHECK(srmPath("/switch/equinox/saves/ps1", "/roms/ps1/Ape Escape (USA).m3u") ==
          "/switch/equinox/saves/ps1/Ape Escape (USA).srm");
}

TEST_CASE("srm path tolerates trailing slash") {
    CHECK(srmPath("/s/", "/r/a.cue") == "/s/a.srm");
}

TEST_CASE("stem keeps dots inside name") {
    CHECK(romStem("/roms/Mr. Driller.chd") == "Mr. Driller");
    CHECK(romStem("C:\\roms\\Mr. Driller.chd") == "Mr. Driller");
    CHECK(romStem("noext") == "noext");
}

TEST_CASE("state path") {
    CHECK(statePath("/st", "/r/Game.chd") == "/st/Game.state");
}

TEST_CASE("the state thumbnail sits beside the state") {
    CHECK(stateThumbPath("/st", "/r/Game.chd") == "/st/Game.state.png");
}

TEST_CASE("video prefs path uses the disc file name") {
    CHECK(videoPrefsPath("sdmc:/s", "sdmc:/r/Chrono Cross (USA).cue") ==
          "sdmc:/s/Chrono Cross (USA).video.json");
}

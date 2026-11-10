#include "args.hpp"

#include "doctest/doctest.h"

TEST_CASE("parses full launch line") {
    const char* argv[] = {"sdmc:/equinox/emulators/ps1/runner.bin",
        "--protocol", "1", "--rom", "sdmc:/equinox/roms/ps1/Ape Escape (USA).cue",
        "--title", "Ape Escape", "--saves", "sdmc:/equinox/saves/ps1",
        "--states", "sdmc:/equinox/states/ps1",
        "--return", "sdmc:/switch/Equinox.nro", "--resume", "ps1:3"};
    auto a = parseArgs(15, argv);
    REQUIRE(a);
    CHECK(a->protocol == 1);
    CHECK(a->rom == "sdmc:/equinox/roms/ps1/Ape Escape (USA).cue");
    CHECK(a->title == "Ape Escape");
    CHECK(a->saves == "sdmc:/equinox/saves/ps1");
    CHECK(a->states == "sdmc:/equinox/states/ps1");
    CHECK(a->returnPath == "sdmc:/switch/Equinox.nro");
    CHECK(a->resumeToken == "ps1:3");
}

TEST_CASE("rejects other protocol versions") {
    const char* argv[] = {"self", "--protocol", "2", "--rom", "x", "--return", "y"};
    CHECK_FALSE(parseArgs(7, argv));
}

TEST_CASE("rejects missing protocol") {
    const char* argv[] = {"self", "--rom", "x", "--return", "y"};
    CHECK_FALSE(parseArgs(5, argv));
}

TEST_CASE("rejects missing rom or return") {
    const char* noRom[] = {"self", "--protocol", "1", "--return", "y"};
    CHECK_FALSE(parseArgs(5, noRom));
    const char* noReturn[] = {"self", "--protocol", "1", "--rom", "x"};
    CHECK_FALSE(parseArgs(5, noReturn));
}

TEST_CASE("no arguments means opened from hbmenu") {
    const char* argv[] = {"self"};
    CHECK_FALSE(parseArgs(1, argv));
}

TEST_CASE("a trailing flag without a value is ignored") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "x", "--return", "y", "--resume"};
    auto a = parseArgs(8, argv);
    REQUIRE(a);
    CHECK(a->resumeToken.empty());
}

TEST_CASE("values split on spaces by the launcher are joined back") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "sdmc:/equinox/roms/ps1/Crash",
        "Bandicoot", "(USA).chd", "--title", "Crash", "Bandicoot", "--return",
        "sdmc:/switch/Equinox.nro", "--resume", "t", "--show-fps"};
    auto a = parseArgs(15, argv);
    REQUIRE(a);
    CHECK(a->rom == "sdmc:/equinox/roms/ps1/Crash Bandicoot (USA).chd");
    CHECK(a->title == "Crash Bandicoot");
    CHECK(a->returnPath == "sdmc:/switch/Equinox.nro");
    CHECK(a->resumeToken == "t");
}

TEST_CASE("unknown flags and their values are skipped") {
    const char* argv[] = {"self", "--show-fps", "--future", "x", "--protocol", "1", "--rom", "r",
        "--return", "y"};
    auto a = parseArgs(10, argv);
    REQUIRE(a);
    CHECK(a->rom == "r");
}

TEST_CASE("return command quotes the path") {
    LaunchArgs a;
    a.returnPath = "sdmc:/switch/Equinox App.nro";
    a.resumeToken = "fav:all:2";
    CHECK(buildReturnCommand(a) == "\"sdmc:/switch/Equinox App.nro\" --resume \"fav:all:2\"");
}

TEST_CASE("return command sanitizes its values") {
    LaunchArgs a;
    a.returnPath = "sdmc:/switch/Eq\"x.nro";
    a.resumeToken = "a\"b";
    CHECK(buildReturnCommand(a) == "\"sdmc:/switch/Eqx.nro\" --resume \"ab\"");
}

TEST_CASE("sanitize keeps accents and apostrophes, drops quotes") {
    CHECK(sanitizeArg("Pokémon \"Red\"") == "Pokémon Red");
    CHECK(sanitizeArg("Street Fighter II' Turbo") == "Street Fighter II' Turbo");
}

TEST_CASE("sanitize drops control characters") {
    CHECK(sanitizeArg(std::string("a\n\tb\x01") + "c") == "abc");
}

TEST_CASE("cover and video paths are read, spaces included") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "r", "--return", "y",
        "--cover", "sdmc:/a", "b.png", "--video", "sdmc:/v.json"};
    auto a = parseArgs(12, argv);
    REQUIRE(a);
    CHECK(a->cover == "sdmc:/a b.png");
    CHECK(a->video == "sdmc:/v.json");
}

TEST_CASE("cover and video default to empty") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "r", "--return", "y"};
    auto a = parseArgs(7, argv);
    REQUIRE(a);
    CHECK(a->cover.empty());
    CHECK(a->video.empty());
}

TEST_CASE("the BIOS folder defaults to sdmc:/equinox/bios") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "x", "--return", "y"};
    auto a = parseArgs(7, argv);
    REQUIRE(a);
    CHECK(a->bios == "sdmc:/equinox/bios");
}

TEST_CASE("--bios names another BIOS folder") {
    const char* argv[] = {"self", "--protocol", "1", "--rom", "x", "--return", "y", "--bios", "sdmc:/bios/ps1"};
    auto a = parseArgs(9, argv);
    REQUIRE(a);
    CHECK(a->bios == "sdmc:/bios/ps1");
}

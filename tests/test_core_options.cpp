#include "core_options.hpp"

#include <cstring>

#include "doctest/doctest.h"

TEST_CASE("memory card 1 goes to the frontend, card 2 and rumble are off") {
    CHECK(std::strcmp(runnerOption("pcsx_rearmed_memcard1"), "libretro") == 0);
    CHECK(std::strcmp(runnerOption("pcsx_rearmed_memcard2"), "none") == 0);
    CHECK(std::strcmp(runnerOption("pcsx_rearmed_vibration"), "disabled") == 0);
}

TEST_CASE("other options are not answered by the runner") {
    CHECK(runnerOption("pcsx_rearmed_drc") == nullptr);
    CHECK(runnerOption("pcsx_rearmed_memcard") == nullptr);
    CHECK(runnerOption("") == nullptr);
    CHECK(runnerOption(nullptr) == nullptr);
}

TEST_CASE("declared defaults answer the options the runner leaves alone") {
    CoreOptions o;
    o.declare("pcsx_rearmed_drc", "enabled");
    o.declare("pcsx_rearmed_memcard2", "shared");
    CHECK(std::strcmp(o.value("pcsx_rearmed_drc"), "enabled") == 0);
    CHECK(std::strcmp(o.value("pcsx_rearmed_memcard2"), "none") == 0);  // the runner wins
    CHECK(std::strcmp(o.value("pcsx_rearmed_memcard1"), "libretro") == 0);  // even undeclared
    CHECK(o.value("pcsx_rearmed_unknown") == nullptr);
    CHECK(o.value(nullptr) == nullptr);
}

TEST_CASE("declare ignores empty keys and missing defaults, and clear forgets") {
    CoreOptions o;
    o.declare(nullptr, "x");
    o.declare("", "x");
    o.declare("pcsx_rearmed_bios", nullptr);
    CHECK(o.value("pcsx_rearmed_bios") == nullptr);
    o.declare("pcsx_rearmed_bios", "auto");
    o.declare("pcsx_rearmed_bios", "HLE");  // a later declaration replaces it
    CHECK(std::strcmp(o.value("pcsx_rearmed_bios"), "HLE") == 0);
    o.clear();
    CHECK(o.value("pcsx_rearmed_bios") == nullptr);
}

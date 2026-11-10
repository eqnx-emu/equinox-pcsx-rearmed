#include "rom.hpp"

#include "doctest/doctest.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

fs::path tempDir() {
    static fs::path d = [] {
        auto p = fs::temp_directory_path() / "eqx_test_disc";
        fs::remove_all(p);
        fs::create_directories(p);
        return p;
    }();
    return d;
}

std::string utf8(const fs::path& p) {
    auto u = p.u8string();
    return std::string(u.begin(), u.end());
}

std::string write(const std::string& name, const std::string& text) {
    auto p = tempDir() / fs::path(std::u8string(name.begin(), name.end()));
    fs::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    f << text;
    return utf8(p);
}

RomError errorOf(const std::variant<Disc, RomError>& r) {
    const auto* e = std::get_if<RomError>(&r);
    return e ? *e : RomError::None;
}

}  // namespace

TEST_CASE("single-file images are accepted as they are") {
    for (const char* name : {"a.chd", "b.PBP", "c.iso", "d.img", "e.bin"}) {
        const std::string path = write(name, "data");
        const auto r = checkDisc(path);
        REQUIRE(std::holds_alternative<Disc>(r));
        CHECK(std::get<Disc>(r).path == path);
    }
}

TEST_CASE("other extensions are unsupported") {
    CHECK(errorOf(checkDisc(write("game.sfc", "x"))) == RomError::Unsupported);
    CHECK(errorOf(checkDisc(write("game.zip", "x"))) == RomError::Unsupported);
    CHECK(errorOf(checkDisc(write("noext", "x"))) == RomError::Unsupported);
    CHECK(errorOf(checkDisc(write("dir.cue/track", "x"))) == RomError::Unsupported);
}

TEST_CASE("a missing disc is not found") {
    CHECK(errorOf(checkDisc(utf8(tempDir() / "nope.chd"))) == RomError::NotFound);
    CHECK(errorOf(checkDisc(utf8(tempDir() / "nope.cue"))) == RomError::NotFound);
}

TEST_CASE("a cue with every track present is accepted") {
    write("cue1/Game (Track 1).bin", "t1");
    write("cue1/Game (Track 2).bin", "t2");
    const std::string cue = write("cue1/Game.cue",
                                  "FILE \"Game (Track 1).bin\" BINARY\r\n"
                                  "  TRACK 01 MODE2/2352\r\n"
                                  "    INDEX 01 00:00:00\r\n"
                                  "file \"Game (Track 2).bin\" BINARY\r\n"
                                  "  TRACK 02 AUDIO\r\n");
    CHECK(std::holds_alternative<Disc>(checkDisc(cue)));
}

TEST_CASE("a cue with a missing track reports it") {
    write("cue2/Game (Track 1).bin", "t1");
    const std::string cue = write("cue2/Game.cue",
                                  "FILE \"Game (Track 1).bin\" BINARY\n"
                                  "FILE \"Game (Track 2).bin\" BINARY\n");
    CHECK(errorOf(checkDisc(cue)) == RomError::MissingTrack);
}

TEST_CASE("unquoted names, a byte order mark and tabs are read") {
    write("cue3/game.bin", "t1");
    const std::string cue = write("cue3/Game.cue", "\xEF\xBB\xBF\tFILE game.bin BINARY\n");
    CHECK(std::holds_alternative<Disc>(checkDisc(cue)));
}

TEST_CASE("a cue naming no file is unreadable") {
    CHECK(errorOf(checkDisc(write("cue4/Empty.cue", "REM nothing\nTRACK 01 MODE2/2352\n"))) ==
          RomError::Unreadable);
    CHECK(errorOf(checkDisc(write("cue4/Garbage.cue", "FILE \"\" BINARY\n"))) == RomError::Unreadable);
}

TEST_CASE("an oversized cue is unreadable") {
    CHECK(errorOf(checkDisc(write("cue5/Big.cue", std::string(64 * 1024 + 1, 'x')))) == RomError::Unreadable);
}

TEST_CASE("an m3u lists discs relative to itself") {
    write("m3u1/Game (Disc 1).chd", "d1");
    write("m3u1/discs/Game (Disc 2).bin", "d2");
    write("m3u1/discs/Game (Disc 2).cue", "FILE \"Game (Disc 2).bin\" BINARY\n");
    const std::string m3u = write("m3u1/Game.m3u", "# discs\nGame (Disc 1).chd\r\n\r\ndiscs/Game (Disc 2).cue\n");
    CHECK(std::holds_alternative<Disc>(checkDisc(m3u)));
}

TEST_CASE("an m3u with a missing disc or a broken cue reports a missing track") {
    write("m3u2/Game (Disc 1).chd", "d1");
    CHECK(errorOf(checkDisc(write("m3u2/A.m3u", "Game (Disc 1).chd\nGame (Disc 2).chd\n"))) ==
          RomError::MissingTrack);
    write("m3u2/Broken.cue", "FILE \"gone.bin\" BINARY\n");
    CHECK(errorOf(checkDisc(write("m3u2/B.m3u", "Broken.cue\n"))) == RomError::MissingTrack);
}

TEST_CASE("an m3u with an absolute path is followed") {
    const std::string disc = write("m3u3/abs.chd", "d1");
    CHECK(std::holds_alternative<Disc>(checkDisc(write("m3u3/Abs.m3u", disc + "\n"))));
}

TEST_CASE("an empty m3u, or one listing an m3u or an unknown file, is refused") {
    CHECK(errorOf(checkDisc(write("m3u4/Empty.m3u", "# nothing\n\n"))) == RomError::Unreadable);
    write("m3u4/Inner.m3u", "x.chd\n");
    CHECK(errorOf(checkDisc(write("m3u4/Outer.m3u", "Inner.m3u\n"))) == RomError::Unsupported);
    write("m3u4/readme.txt", "hi");
    CHECK(errorOf(checkDisc(write("m3u4/Txt.m3u", "readme.txt\n"))) == RomError::Unsupported);
}

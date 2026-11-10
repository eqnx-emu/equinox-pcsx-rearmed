#include "rom.hpp"

#include "file_io.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>
#include <vector>

namespace {

// Real cue sheets and playlists are a few KiB; anything bigger is not one.
constexpr uint64_t kMaxListBytes = 64u * 1024;

namespace fs = std::filesystem;

fs::path toPath(const std::string& utf8) { return fs::path(std::u8string(utf8.begin(), utf8.end())); }

std::string extensionOf(const std::string& name) {
    const auto slash = name.find_last_of("/\\");
    const auto dot = name.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return {};
    std::string ext = name.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    return ext;
}

bool isDiscExt(const std::string& ext) {
    return ext == ".cue" || ext == ".chd" || ext == ".pbp" || ext == ".m3u" || ext == ".iso" ||
           ext == ".img" || ext == ".bin";
}

bool isFile(const std::string& path) {
    std::error_code ec;
    return fs::is_regular_file(toPath(path), ec);
}

// The folder part of path, with its trailing separator ("" when there is none).
std::string dirOf(const std::string& path) {
    const auto slash = path.find_last_of("/\\");
    return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
}

bool isAbsolute(const std::string& p) {
    if (p.empty()) return false;
    if (p[0] == '/' || p[0] == '\\') return true;
    // "sdmc:/..." on the Switch, "C:\..." on Windows.
    const auto colon = p.find(':');
    const auto slash = p.find_first_of("/\\");
    return colon != std::string::npos && (slash == std::string::npos || colon < slash);
}

std::string resolve(const std::string& listFile, const std::string& entry) {
    return isAbsolute(entry) ? entry : dirOf(listFile) + entry;
}

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::optional<std::vector<std::string>> readLines(const std::string& path, RomError& err) {
    const auto size = fileSize(path);
    if (!size) {
        err = isFile(path) ? RomError::Unreadable : RomError::NotFound;
        return std::nullopt;
    }
    if (*size > kMaxListBytes) {
        err = RomError::Unreadable;
        return std::nullopt;
    }
    const auto bytes = readFile(path);
    if (!bytes) {
        err = RomError::Unreadable;
        return std::nullopt;
    }
    std::vector<std::string> lines;
    std::string cur;
    size_t i = 0;
    // A UTF-8 byte order mark is not part of the first line.
    if (bytes->size() >= 3 && (*bytes)[0] == 0xEF && (*bytes)[1] == 0xBB && (*bytes)[2] == 0xBF) i = 3;
    for (; i < bytes->size(); ++i) {
        const char c = static_cast<char>((*bytes)[i]);
        if (c == '\n' || c == '\r') {
            lines.push_back(std::move(cur));
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    lines.push_back(std::move(cur));
    return lines;
}

// The file named by a cue FILE line: FILE "name with spaces.bin" BINARY, or FILE name.bin BINARY.
std::optional<std::string> cueFileName(const std::string& rawLine) {
    const std::string line = trim(rawLine);
    if (line.size() < 5) return std::nullopt;
    std::string keyword = line.substr(0, 4);
    std::transform(keyword.begin(), keyword.end(), keyword.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    if (keyword != "FILE" || !std::isspace(static_cast<unsigned char>(line[4]))) return std::nullopt;
    const std::string rest = trim(line.substr(5));
    if (rest.empty()) return std::nullopt;
    if (rest[0] == '"') {
        const auto close = rest.find('"', 1);
        if (close == std::string::npos || close == 1) return std::nullopt;
        return rest.substr(1, close - 1);
    }
    // Unquoted: the name ends at the last space, before the file type.
    const auto space = rest.find_last_of(" \t");
    return space == std::string::npos ? rest : trim(rest.substr(0, space));
}

RomError checkCue(const std::string& path) {
    RomError err = RomError::None;
    const auto lines = readLines(path, err);
    if (!lines) return err;
    bool any = false;
    for (const std::string& line : *lines) {
        const auto name = cueFileName(line);
        if (!name) continue;
        any = true;
        if (!isFile(resolve(path, *name))) return RomError::MissingTrack;
    }
    return any ? RomError::None : RomError::Unreadable;
}

RomError checkOne(const std::string& path, bool insideM3u);

RomError checkM3u(const std::string& path) {
    RomError err = RomError::None;
    const auto lines = readLines(path, err);
    if (!lines) return err;
    bool any = false;
    for (const std::string& raw : *lines) {
        const std::string line = trim(raw);
        if (line.empty() || line[0] == '#') continue;
        any = true;
        const RomError e = checkOne(resolve(path, line), true);
        if (e == RomError::NotFound) return RomError::MissingTrack;
        if (e != RomError::None) return e;
    }
    return any ? RomError::None : RomError::Unreadable;
}

RomError checkOne(const std::string& path, bool insideM3u) {
    const std::string ext = extensionOf(path);
    if (!isDiscExt(ext) || (insideM3u && ext == ".m3u")) return RomError::Unsupported;
    if (!isFile(path)) return RomError::NotFound;
    if (ext == ".cue") return checkCue(path);
    if (ext == ".m3u") return checkM3u(path);
    return RomError::None;
}

}  // namespace

std::variant<Disc, RomError> checkDisc(const std::string& path) {
    const RomError e = checkOne(path, false);
    if (e != RomError::None) return e;
    return Disc{path};
}

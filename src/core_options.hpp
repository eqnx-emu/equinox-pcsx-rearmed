#pragma once

#include <map>
#include <string>

// Values for the PCSX ReARMed core options. The core declares every option with its
// default (SET_CORE_OPTIONS_V2); the runner answers a few of them its own way and the
// rest with the declared default, the way a full libretro frontend would.
class CoreOptions {
public:
    void declare(const char* key, const char* defaultValue);  // null or empty key: ignored
    void clear() { defaults_.clear(); }
    // The runner's answer, else the declared default, else nullptr (the core keeps its own).
    const char* value(const char* key) const;

private:
    std::map<std::string, std::string, std::less<>> defaults_;
};

// The options the runner answers itself, whatever the core declares; nullptr otherwise.
// Memory card 1 is handed to the frontend (saved as <game>.srm), card 2 is left empty,
// and rumble is off (the runner has no rumble interface).
const char* runnerOption(const char* key);

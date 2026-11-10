#include "core_options.hpp"

#include <cstring>

namespace {
struct Answer {
    const char* key;
    const char* value;
};

constexpr Answer kAnswers[] = {
    {"pcsx_rearmed_memcard1", "libretro"},
    {"pcsx_rearmed_memcard2", "none"},
    {"pcsx_rearmed_vibration", "disabled"},
};
}  // namespace

const char* runnerOption(const char* key) {
    if (!key) return nullptr;
    for (const Answer& a : kAnswers)
        if (std::strcmp(key, a.key) == 0) return a.value;
    return nullptr;
}

void CoreOptions::declare(const char* key, const char* defaultValue) {
    if (!key || !*key || !defaultValue) return;
    defaults_[key] = defaultValue;
}

const char* CoreOptions::value(const char* key) const {
    if (!key || !*key) return nullptr;
    if (const char* v = runnerOption(key)) return v;
    const auto it = defaults_.find(std::string_view(key));
    return it == defaults_.end() ? nullptr : it->second.c_str();
}

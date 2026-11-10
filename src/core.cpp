#include "core.hpp"

#include <cstdarg>
#include <cstdio>

#include "core_options.hpp"
#include "libretro.h"

namespace {

// The DualShock subclass PCSX ReARMed declares (RETRO_DEVICE_PSE_DUALSHOCK).
constexpr unsigned kDualShock = RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 1);

Core* g_core = nullptr;
std::string g_systemDir;
std::string g_saveDir;
double g_fps = 0;  // last timing the core announced through SET_SYSTEM_AV_INFO
retro_disk_control_callback g_disk{};
bool g_hasDisk = false;
CoreOptions g_options;

void declareOptions(const retro_core_options_v2* opts) {
    if (!opts || !opts->definitions) return;
    for (const retro_core_option_v2_definition* d = opts->definitions; d->key; ++d)
        g_options.declare(d->key, d->default_value ? d->default_value : d->values[0].value);
}

void logCb(enum retro_log_level level, const char* fmt, ...) {
    if (level < RETRO_LOG_WARN) return;
    std::va_list ap;
    va_start(ap, fmt);
    std::fprintf(stderr, "[pcsx] ");
    std::vfprintf(stderr, fmt, ap);
    va_end(ap);
}

bool envCb(unsigned cmd, void* data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            // Answered with RGB565 only: pcsx_rearmed_rgb32_output stays off.
            return *static_cast<const enum retro_pixel_format*>(data) == RETRO_PIXEL_FORMAT_RGB565;
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
            *static_cast<const char**>(data) = g_systemDir.c_str();
            return true;
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            *static_cast<const char**>(data) = g_saveDir.c_str();
            return true;
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
            static_cast<retro_log_callback*>(data)->log = logCb;
            return true;
        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
            *static_cast<bool*>(data) = true;
            return true;
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO:
        case RETRO_ENVIRONMENT_SET_GEOMETRY:  // the frame size comes with every frame
            return true;
        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
            const auto* av = static_cast<const retro_system_av_info*>(data);
            if (av->timing.fps > 0) g_fps = av->timing.fps;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE:
            g_disk = *static_cast<const retro_disk_control_callback*>(data);
            g_hasDisk = true;
            return true;
        case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION:
            *static_cast<unsigned*>(data) = 2;
            return true;
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2:
            declareOptions(static_cast<const retro_core_options_v2*>(data));
            return true;
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL:
            // Only the English set: its keys and defaults are the same in every language.
            declareOptions(static_cast<const retro_core_options_v2_intl*>(data)->us);
            return true;
        case RETRO_ENVIRONMENT_GET_VARIABLE: {
            auto* var = static_cast<retro_variable*>(data);
            var->value = g_options.value(var->key);
            return var->value != nullptr;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
            *static_cast<bool*>(data) = false;
            return true;
        default:
            return false;  // other core options and the rest: defaults
    }
}

void videoCb(const void* data, unsigned w, unsigned h, size_t pitch) {
    if (g_core && g_core->onVideo && data) g_core->onVideo(data, w, h, pitch);
}

void audioSampleCb(int16_t l, int16_t r) {
    if (g_core && g_core->onAudio) {
        const int16_t s[2] = {l, r};
        g_core->onAudio(s, 1);
    }
}

size_t audioBatchCb(const int16_t* data, size_t frames) {
    if (g_core && g_core->onAudio) g_core->onAudio(data, frames);
    return frames;
}

void inputPollCb() {}

int16_t inputStateCb(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (!g_core) return 0;
    if (device == RETRO_DEVICE_JOYPAD) return g_core->onInput ? g_core->onInput(port, id) : 0;
    if (device == RETRO_DEVICE_ANALOG) return g_core->onAnalog ? g_core->onAnalog(port, index, id) : 0;
    return 0;
}

}  // namespace

Core::~Core() { unload(); }

bool Core::load(const std::string& discPath, const std::string& systemDir, const std::string& saveDir) {
    if (loaded_ || g_core) return false;
    g_core = this;
    g_systemDir = systemDir;
    g_saveDir = saveDir;
    g_fps = 0;
    g_hasDisk = false;
    g_options.clear();

    retro_set_environment(envCb);
    retro_init();
    retro_set_video_refresh(videoCb);
    retro_set_audio_sample(audioSampleCb);
    retro_set_audio_sample_batch(audioBatchCb);
    retro_set_input_poll(inputPollCb);
    retro_set_input_state(inputStateCb);
    // Before the game loads, so the pads are DualShocks from the first frame.
    retro_set_controller_port_device(0, kDualShock);
    retro_set_controller_port_device(1, kDualShock);

    retro_game_info game{};
    game.path = discPath.c_str();
    if (!retro_load_game(&game)) {
        retro_deinit();
        g_core = nullptr;
        return false;
    }

    retro_system_av_info av{};
    retro_get_system_av_info(&av);
    if (av.timing.fps > 0) fps_ = av.timing.fps;
    if (g_fps > 0) fps_ = g_fps;
    if (av.timing.sample_rate > 0) sampleRate_ = av.timing.sample_rate;
    loaded_ = true;
    return true;
}

void Core::runFrame() {
    if (loaded_) retro_run();
}

void Core::unload() {
    if (!loaded_) return;
    retro_unload_game();
    retro_deinit();
    loaded_ = false;
    g_core = nullptr;
    g_hasDisk = false;
}

std::span<uint8_t> Core::saveRam() {
    if (!loaded_) return {};
    const size_t n = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    void* p = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    if (!n || !p) return {};
    return {static_cast<uint8_t*>(p), n};
}

std::vector<uint8_t> Core::serialize() {
    if (!loaded_) return {};
    std::vector<uint8_t> out(retro_serialize_size());
    if (out.empty() || !retro_serialize(out.data(), out.size())) return {};
    return out;
}

bool Core::unserialize(std::span<const uint8_t> state) {
    return loaded_ && !state.empty() && retro_unserialize(state.data(), state.size());
}

unsigned Core::discCount() const {
    if (!loaded_ || !g_hasDisk || !g_disk.get_num_images) return 0;
    return g_disk.get_num_images();
}

unsigned Core::discIndex() const {
    if (!loaded_ || !g_hasDisk || !g_disk.get_image_index) return 0;
    return g_disk.get_image_index();
}

bool Core::setDisc(unsigned i) {
    if (!loaded_ || !g_hasDisk || !g_disk.set_eject_state || !g_disk.set_image_index) return false;
    if (i >= discCount()) return false;
    if (!g_disk.set_eject_state(true)) return false;
    const bool swapped = g_disk.set_image_index(i);
    // Close the tray even when the swap failed, so the game is not left without a disc.
    const bool closed = g_disk.set_eject_state(false);
    return swapped && closed;
}

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

// Thin wrapper over the statically linked libretro PCSX ReARMed core. Only one instance
// can be loaded at a time (libretro callbacks are plain C functions).
class Core {
public:
    ~Core();

    // retro_init + retro_load_game on the disc at discPath (the core reads it itself).
    // systemDir holds the BIOS files (scph*.bin); without one the core uses its HLE BIOS.
    // saveDir is handed to the core as its save folder.
    bool load(const std::string& discPath, const std::string& systemDir, const std::string& saveDir);
    void runFrame();  // retro_run
    void unload();

    double fps() const { return fps_; }                // ~59.94 NTSC, ~50 PAL
    double sampleRate() const { return sampleRate_; }  // 44100
    std::span<uint8_t> saveRam();                      // memory card 1, 128 KiB
    std::vector<uint8_t> serialize();
    bool unserialize(std::span<const uint8_t> state);

    // Multi-disc games (.m3u, multi-disc .pbp). Count is 0 or 1 for a single disc.
    unsigned discCount() const;
    unsigned discIndex() const;
    // Opens the tray, swaps to disc i and closes it. False when the core refuses.
    bool setDisc(unsigned i);

    // Wired by the platform layer.
    std::function<void(const void* px, unsigned w, unsigned h, size_t pitch)> onVideo;
    std::function<void(const int16_t* lr, size_t frames)> onAudio;
    std::function<int16_t(unsigned port, unsigned id)> onInput;  // id: RETRO_DEVICE_ID_JOYPAD_*
    // index: RETRO_DEVICE_INDEX_ANALOG_LEFT/RIGHT, axis: RETRO_DEVICE_ID_ANALOG_X/Y.
    std::function<int16_t(unsigned port, unsigned index, unsigned axis)> onAnalog;

private:
    bool loaded_ = false;
    double fps_ = 59.94;
    double sampleRate_ = 44100.0;
};

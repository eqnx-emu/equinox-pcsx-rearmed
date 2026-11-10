#pragma once

enum class MenuItem { Continue, SaveState, LoadState, Video, Disc, Exit };
enum class MenuAction { None, Close, SaveState, LoadState, OpenVideo, NextDisc, Exit };

// In-game menu state. Without a saved state "Load state" is disabled and skipped by up/down.
// The disc item ("Disc 1 of 2") is shown only for a game with more than one disc.
class Menu {
public:
    bool isOpen() const { return open_; }
    void open();
    void close() { open_ = false; }
    void setHasState(bool has);
    bool hasState() const { return hasState_; }
    // current is 0-based; a count of 0 or 1 hides the disc item.
    void setDiscs(unsigned count, unsigned current);
    unsigned discCount() const { return discCount_; }
    unsigned disc() const { return disc_; }
    bool shown(MenuItem i) const { return i != MenuItem::Disc || discCount_ > 1; }
    MenuItem selected() const { return sel_; }

    MenuAction up();
    MenuAction down();
    MenuAction confirm();
    MenuAction back();  // same as Close

private:
    void move(int dir);
    bool enabled(MenuItem i) const { return shown(i) && (i != MenuItem::LoadState || hasState_); }
    bool open_ = false;
    bool hasState_ = false;
    unsigned discCount_ = 0, disc_ = 0;
    MenuItem sel_ = MenuItem::Continue;
};

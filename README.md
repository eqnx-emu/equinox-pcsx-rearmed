# equinox-pcsx-rearmed

A standalone Nintendo Switch homebrew (`.nro`) that runs the
[PCSX ReARMed](https://github.com/libretro/pcsx_rearmed) libretro core. It is launched by
the Equinox app (a Moonlight fork) to play PlayStation games.

## License

- PCSX ReARMed (`extern/pcsx_rearmed/COPYING`) is GPL version 2. The `.nro` links it
  statically, so the built `.nro` as a whole is distributed under the GPL v2; this
  repository is its corresponding source. That is one reason the runner is
  distributed separately from Equinox and is not bundled with it.
- The frontend (the code in `src/`, `tests/`, `docker/`, `tools/`): this repository
  is MIT (see `LICENSE`), which is compatible with the GPL v2 the combined `.nro` falls under.
- `extern/miniz` is MIT (see `extern/miniz/LICENSE`); it writes the savestate pictures.

## Build

Clone with submodules: `git clone --recurse-submodules ...`
(or `git submodule update --init`).

Switch `.nro` (needs Docker, uses the `devkitpro/devkita64` image):

    docker compose run --rm switch

Output: `build/docker/equinox-pcsx-rearmed.nro`. The first run also installs
`switch-sdl2` and `switch-sdl2_ttf` from devkitPro portlibs.

Host unit tests:

    cmake -B build/host -DEQX_HOST_TESTS=ON
    cmake --build build/host
    ctest --test-dir build/host --output-on-failure

## Games and BIOS

`--rom` is a disc image: `.cue` (with its `.bin` tracks beside it), `.chd`, `.pbp`,
`.iso`, `.img`, `.bin`, or an `.m3u` listing the discs of a multi-disc game, one path per
line, relative to the `.m3u`. The runner checks the files before starting the core: a
missing file, a `.cue` track or an `.m3u` disc that is not there shows an error and
offers the way back to Equinox. Archives (`.zip`, `.7z`) are not read: use `.chd` to
save space.

A multi-disc game shows "Disco 1 de 2" in the pause menu; selecting it moves to the next
disc, as if the tray had been opened and the disc swapped.

The BIOS is looked for in `--bios` (default `sdmc:/equinox/bios`): the core picks the
`scph*.bin` that fits the game's region (`scph5501.bin` for USA, `scph5500.bin` for
Japan, `scph5502.bin` for Europe, among others). Without one it runs on its built-in HLE
BIOS, which plays most games but not all. Equinox does not provide BIOS files.

## Controls

The Switch pad plays a DualShock, by button position: A = Circle, B = Cross,
X = Triangle, Y = Square, L/R = L1/R1, ZL/ZR = L2/R2, the stick clicks = L3/R3,
Plus = Start, Minus = Select. The D-pad is the D-pad and the sticks are the analog
sticks; L1 + R1 + Select switches the pad's analog mode, as on the DualShock's ANALOG
button. Two players are supported.

Minus and Plus together open the pause menu (ZL + ZR are L2 + R2 here, so they cannot
be the menu combo the other Equinox runners use). Touching the screen, or player 1's
pad disconnecting, also opens it.

## Protocol

Equinox starts the runner by chain-loading `equinox-pcsx-rearmed.nro` with this
argument line (protocol version 1):

    --protocol 1 --rom <path> --title <title> --saves <dir> --states <dir>
    --return <equinox.nro> --resume <token> [--cover <png>] [--video <json>] [--bios <dir>]

| Argument     | Required | Meaning |
|--------------|----------|---------|
| `--protocol` | yes      | Must be `1`; any other version is rejected. |
| `--rom`      | yes      | Path of the disc image to run (see Games and BIOS). |
| `--title`    | no       | Display name of the game. |
| `--saves`    | no       | Directory for memory cards (`<disc stem>.srm`, memory card 1). |
| `--states`   | no       | Directory for save states. Saving a state also writes a small picture of the game beside it (`<stem>.state.png`), shown in the pause menu. |
| `--return`   | yes      | Path of the Equinox `.nro` to reopen on exit. |
| `--resume`   | no       | Opaque token. The runner never interprets it; Equinox decides what it holds (screen, filter, game) and gets it back unchanged. |
| `--cover`    | no       | PNG of the game cover, used by the `cover` border. Without it, `cover` falls back to `equinox`. |
| `--video`    | no       | Global video defaults file (`sdmc:/equinox/emulators/ps1/video.json`, written by Equinox). Without it the runner uses the built-in defaults. |
| `--bios`     | no       | Directory with the PlayStation BIOS files. Default `sdmc:/equinox/bios`. |

Without a valid line (opened from hbmenu) the runner shows "Abra este emulador pelo
Equinox" for 4 seconds and exits without scheduling a return.

On exit the runner calls `envSetNextLoad` with `"<return>" --resume "<token>"`, so
Equinox reopens with `--resume <token>`. libnx splits that string on spaces with
double-quote grouping and no escapes, so double quotes and control characters are
stripped from the values.

## Video options

The pause menu has a "Vídeo" item. The global defaults come from the `--video` file;
each game keeps its own overrides in `<saves>/<disc stem>.video.json`, named after the
disc file name without its extension (`Chrono Cross.m3u` gives
`Chrono Cross.video.json`), written by the runner with the options changed in that
game. An option changed back to the global default is dropped from the file; an option
not touched in the game keeps its value even if the global default later matches it.
The `tv` border only shows around `original`. Both files are JSON objects; unknown keys
are ignored, and a missing key or unknown value follows the default.

| Key      | Values |
|----------|--------|
| `size`   | `4:3` (default), `original`, `full` |
| `smooth` | `pixel` (default), `sharp`, `soft` |
| `filter` | `none` (default), `scanlines` |
| `border` | `equinox` (default), `cover`, `tv`, `black` |

`original` scales the picture by a whole factor, counting the 512/640-wide and
interlaced modes at half their size, so it does not jump when a game switches modes.

    {"size": "original", "filter": "scanlines"}

## Release and manifest

Every push to `main` runs the host tests, builds the `.nro` in the
`devkitpro/devkita64` container and, when `VERSION_MAJOR/MINOR/ALTER` in
`CMakeLists.txt` names a version with no release yet, publishes release
`v<version>` with two assets: `equinox-pcsx-rearmed.nro` and `manifest.json`.
Bump the version to ship an update.

`manifest.json` is generated by `tools/manifest.sh <nro> <version>` (protocol
read from `kProtocolVersion` in `src/args.hpp`):

    {"id": "pcsx-rearmed", "console": "ps1", "version": "1.0.0", "protocol": 1,
     "asset": "equinox-pcsx-rearmed.nro", "size": 9973925, "sha256": "<hex>"}

| Key        | Meaning |
|------------|---------|
| `id`       | Runner id, always `pcsx-rearmed`. |
| `console`  | Console this runner plays, `ps1`. |
| `version`  | Runner version, same as the release tag without the `v`. |
| `protocol` | Launch protocol version the runner speaks (see Protocol). |
| `asset`    | Name of the release asset to download. |
| `size`     | Size of the asset in bytes. |
| `sha256`   | Lowercase hex SHA-256 of the asset. |

## Testing on a Switch with nxlink

    nxlink -s -p equinox-pcsx-rearmed.nro build/docker/equinox-pcsx-rearmed.nro -- \
        --protocol 1 --rom "sdmc:/equinox/roms/ps1/Game Name.chd" --title "Game Name" \
        --saves sdmc:/equinox/saves/ps1 --states sdmc:/equinox/states/ps1 \
        --return sdmc:/switch/Equinox.nro --resume t --show-fps

Under nxlink the runner sends stdout/stderr to the `-s` server. hbmenu's netloader
drops quotes, so the runner joins a value's words back up to the next `--` flag.
Known issue: if the `-s` server is still listening when you choose Exit, the next
program crashes back to HOME. Stop nxlink first, or test exits without `-s`.
Users never hit this: outside nxlink the runner opens no socket.

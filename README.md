# Bloodborne PC Port for Windows, with an installer

**This fork**

[![Latest release](https://img.shields.io/github/v/release/AndrewRicc/bloodborne_pc_windows_port?include_prereleases&style=for-the-badge&logo=github&logoColor=white&labelColor=1a0f0f&color=8b0000&label=RELEASE)](https://github.com/AndrewRicc/bloodborne_pc_windows_port/releases)
[![Downloads](https://img.shields.io/github/downloads/AndrewRicc/bloodborne_pc_windows_port/total?style=for-the-badge&logo=github&logoColor=white&labelColor=1a0f0f&color=b22222&label=DOWNLOADS)](https://github.com/AndrewRicc/bloodborne_pc_windows_port/releases)
[![Build and tests](https://img.shields.io/github/actions/workflow/status/AndrewRicc/bloodborne_pc_windows_port/windows-installer.yml?branch=master&style=for-the-badge&logo=githubactions&logoColor=white&labelColor=1a0f0f&label=BUILD%20%2B%20TESTS)](https://github.com/AndrewRicc/bloodborne_pc_windows_port/actions/workflows/windows-installer.yml)
[![Last commit](https://img.shields.io/github/last-commit/AndrewRicc/bloodborne_pc_windows_port?style=for-the-badge&logo=git&logoColor=white&labelColor=1a0f0f&color=6b6b6b&label=LAST%20COMMIT)](https://github.com/AndrewRicc/bloodborne_pc_windows_port/commits)
![Platform](https://img.shields.io/badge/PLATFORM-WINDOWS-0078D6?style=for-the-badge&logo=windows&logoColor=white&labelColor=1a0f0f)

**Based on [Mrsuss60/bloodborne_pc_windows_port](https://github.com/Mrsuss60/bloodborne_pc_windows_port)**

[![Upstream release](https://img.shields.io/github/v/release/Mrsuss60/bloodborne_pc_windows_port?style=for-the-badge&logo=github&logoColor=white&labelColor=1a0f0f&color=8b0000&label=UPSTREAM%20RELEASE)](https://github.com/Mrsuss60/bloodborne_pc_windows_port/releases/latest)
[![Upstream stars](https://img.shields.io/github/stars/Mrsuss60/bloodborne_pc_windows_port?style=for-the-badge&logo=github&logoColor=white&labelColor=1a0f0f&color=c9a227&label=UPSTREAM%20STARS)](https://github.com/Mrsuss60/bloodborne_pc_windows_port/stargazers)
[![Upstream last commit](https://img.shields.io/github/last-commit/Mrsuss60/bloodborne_pc_windows_port?style=for-the-badge&logo=git&logoColor=white&labelColor=1a0f0f&color=6b6b6b&label=UPSTREAM%20LAST%20COMMIT)](https://github.com/Mrsuss60/bloodborne_pc_windows_port/commits)

---

A native 64-bit Windows port of the PlayStation 4 executable of *Bloodborne* (CUSA03173, version
1.09). The game's code runs directly on your CPU, without emulation, and its graphics are
translated to Vulkan.

The port itself is the work of [deadinside28](https://github.com/deadinside28/bloodborne_pc)
(the original, for Linux) and [Mrsuss60](https://github.com/Mrsuss60/bloodborne_pc_windows_port)
(the Windows adaptation). **This fork exists to make it easy to use for everyone**: you download
one installer, point it at your game folder, and play. Nothing to compile, no tools to install.

> **No game files or copyrighted assets are included.** You must provide your own decrypted dump
> of Bloodborne (CUSA03173, version 1.09).
> This project is not affiliated with Sony Interactive Entertainment, FromSoftware, AMD or NVIDIA.

**Status: playable on Windows.** The game boots, loads saves, and runs with audio and gamepad
support.

---

## What this fork adds

| | Upstream (Mrsuss60) | This fork |
|---|---|---|
| Getting it | A ZIP to extract, then `launch_gui.bat` | A Windows installer with a guided setup, a Start menu entry and an uninstaller |
| Checking your PC | By hand | The installer checks Windows, CPU, graphics driver and Python, and can install Python |
| Game folder | Picked in the launcher | Asked by the installer, and validated (CUSA03173, 1.09) |
| Desktop shortcut | None | Optional, with the icon taken from your own game copy |
| Saves from an older installation | Copied by hand | Imported by the installer, or with **Import saves...** in the launcher |
| In-game menu language | English or Russian | 12 languages: English, Russian, Italian, Spanish, French, German, Portuguese, Polish, Turkish, Chinese, Japanese, Korean |
| Upscalers | FSR 3.1, FSR 4, TAA | The same, plus NVIDIA DLSS, built on your PC |
| Building from source | Install the tools yourself, then `build.bat` | `setup.bat` checks every requirement, installs what is missing and builds |
| Quality checks | | Automatic build, tests and installer test on every change (GitHub Actions) |

Everything else (the renderer, the runtime, the launcher, the patches) is upstream's work, merged
here regularly. This fork currently includes upstream up to **v0.4.2**.

---

## Install and play

1. Download **`bbport-setup-<version>.exe`** from the
   [releases page](https://github.com/AndrewRicc/bloodborne_pc_windows_port/releases).
2. Run it. It checks your PC, then asks for:
   - the folder of **your own copy of the game** (the one with `eboot.bin`, `dvdroot_ps4` and
     `sce_sys`). The game is not copied: the port reads it where it is;
   - optionally, the folder of an earlier installation, to **import your saves**;
   - whether you want a **desktop shortcut**.
3. Start **Bloodborne PC Port** from the desktop or the Start menu, and press
   **Launch Bloodborne** in the launcher.

Some stutter on the first start is normal: the game is preparing its shaders.

Windows may warn that the installer comes from an unknown publisher: it is not signed.

### Requirements

- **Operating system**: Windows 10 or Windows 11, 64-bit.
- **Processor**: x86-64 with AVX2.
- **Graphics card**: Vulkan 1.3 with current drivers (NVIDIA GeForce GTX 10-series, AMD Radeon
  RX 400-series, Intel Arc A-series, or newer).
- **Python** 3.10 or newer: the installer finds it, and offers to install it if it is missing.
- **Game files**: your decrypted `CUSA03173` folder, version 1.09.

### What the installer contains

The port already built, its DLLs, the launcher and the FSR 4 shaders. It never contains game
files, the game's icon, saves or settings, and the build refuses to finish if it finds one.

**DLSS is built on your PC, not shipped.** The installer carries nothing of NVIDIA: on a PC with
an NVIDIA RTX card it offers to build DLSS, and `build_dlss.bat` in the installation folder does
the same later. It downloads NVIDIA's SDK files (63 MB) from NVIDIA's own repository and compiles
the small bridge; that needs the Microsoft C++ compiler (Visual Studio Build Tools, about 7 GB,
installed on request if missing). FSR 3.1, FSR 4 and TAA need none of this and work on every
recent card, NVIDIA included.

Uninstalling (Windows Settings > Apps) removes the port and leaves your saves and settings.

---

## Build from source

For developers:

```powershell
git clone -c core.longpaths=true https://github.com/AndrewRicc/bloodborne_pc_windows_port.git bloodborne
cd bloodborne
.\setup.bat
```

`setup.bat` checks every requirement, installs only what is missing (Python, MSYS2 and its
libraries, and on NVIDIA RTX cards the Visual Studio Build Tools and the DLSS SDK), downloads the
FSR 4 shaders and builds. The first build takes 20-30 minutes and about 15 GB.

**[GUIDE.md](GUIDE.md)** has the full instructions: where to put the game, the setup options,
common problems, how to run the tests and how to build the installer yourself.

---

## Controls and in-game features

- **Gamepad**: controllers are supported out of the box through SDL3.
- **Mouse and keyboard**: supported, with sensitivity and axis options in the in-game menu.
- **In-game menu**: press `Insert` on the keyboard or `L3 + R3` on the gamepad. The first entry is
  the language; "Auto" follows the Windows language. Upscaler, resolution, sharpness and effects
  are changed there, and saved to `bbport.ini` when the menu closes.
- **Performance HUD**: `F11` or `Shift + Tab`.
- **Free camera**: when enabled in the launcher or the menu, hold `Cross` and press `L3` (or hold
  `Space` and press `Z` on the keyboard) to cycle camera modes.
- **Debug menu**: if the debug fonts (`DbgFont14h.ccm` and `DbgFont14h.tpf`) are installed into
  `dvdroot_ps4/font/`, open it with `Tab` or the controller touchpad.

Saves and shader caches are stored in `user\`; settings in `bbport.ini`.

---

## How the port works

These are the foundations laid by the original project and by the Windows adaptation.

- **Native execution**: the game's eboot is converted offline into a flat memory image; PS4 libc
  and libSceFios2 functions are linked directly into the binary. No CPU emulation or instruction
  translation happens at run time.
- **Win32 runtime**: Linux `mmap`, `mprotect` and POSIX primitives are replaced with
  `VirtualAlloc`, `VirtualProtect` and native Win32 synchronization (`src/win32_compat.c`,
  `src/win32_memory.c`, `src/runtime_host.c`).
- **High frame rates**: community simulation patches allow 60 FPS, 90 FPS or uncapped play with
  correct physics and movement speeds.
- **Synthetic motion vectors and temporal upscaling**: Bloodborne has no velocity buffer, so the
  port computes camera motion from the depth buffer and object motion from the previous frame's
  vertex positions. The scene can render at a lower resolution while the upscaler reaches your
  display resolution, and the UI stays sharp at native resolution.
- **Multi-threaded draw pipeline**: PS4 command buffers are decoded on one thread while draw calls
  are bound and recorded on another.
- **Frame pacing**: presenter queue pacing (`BB_FRAMES_AHEAD=2`) bounds GPU run-ahead and
  stabilizes frame times.
- **Graphical launcher**: `launch_gui.bat` (`launcher.py`) sets frame rate, resolution, window
  mode, upscaling and the community patches from `Bloodborne.xml`. `run.bat` starts the game
  directly from a script.
- **AT9 audio**: LibAtrac9 decodes the game's audio natively.

---

## Repository layout

| Path | Description |
|---|---|
| `setup.bat` | One-step setup from source: requirements, downloads, build |
| `GUIDE.md` | Full instructions for building, testing and packaging |
| `src/` | Win32 loader (`probe.c`), memory management and PS4 HLE runtime |
| `gpu/` | Vulkan video core, command pipeline and upscaler integration |
| `gpu/dlss/` | DLSS bridge (a separate library, MIT licence) |
| `gpu/shim/bbport_i18n.cpp` | The in-game menu texts in 12 languages |
| `scripts/` | Eboot preparation, patch compiler (`patches.py`), save import |
| `scripts/windows/` | Setup, tests and installer scripts |
| `packaging/windows/` | The installer (Inno Setup) |
| `patches/` | Community XML patches for Bloodborne version 1.09 |
| `launcher.py`, `launch_gui.bat`, `run.bat` | Launcher and direct start |
| `modules/` | Where your game copy and the DLSS SDK go when building from source (ignored by git) |
| `tests/` | Tests |
| `docs/` | Architecture notes, upscaler documentation and change logs |

---

## Credits and licences

This project is licensed under the **GNU General Public License v2 or later**
([LICENSE](LICENSE)), as it builds on shadPS4 components. The DLSS bridge in `gpu/dlss` is a
separate library under the MIT licence ([gpu/dlss/LICENSE.txt](gpu/dlss/LICENSE.txt)).

This fork only adds an easier way in. The port is the work of:

- **[deadinside28](https://github.com/deadinside28)**: creator of the original Linux port
  ([bbport](https://github.com/deadinside28/bloodborne_pc)): the flat memory image architecture,
  the two-stage Vulkan pipeline and the synthetic motion vectors.
- **[Mrsuss60](https://github.com/Mrsuss60)**: the
  [Windows port](https://github.com/Mrsuss60/bloodborne_pc_windows_port) this fork is based on.
- **shadPS4 Team**: base Vulkan video core and shader recompiler.
- **FireBurn**: FSR-Vulkan runtime and temporal upscaling backend.
- **Thealexbarney**: LibAtrac9 audio decoding library.
- **ocornut**: Dear ImGui, used by the in-game menu.
- **Community patch authors**: Kyo, Lance McDonald, illusion, emoose, auser1337 and contributors,
  for the 60 FPS, camera and engine patches.
- **zackcage6**: testing and bug reporting upstream.
- **[parsifal295](https://github.com/parsifal295)**, **[wizardnun](https://github.com/wizardnun)**
  and **[Vorshim92](https://github.com/Vorshim92)**: pull requests to the Windows port.

In this fork: installer, one-step setup, multilingual menu, DLSS bridge, save import, tests and
automatic builds by **[AndrewRicc](https://github.com/AndrewRicc)**.

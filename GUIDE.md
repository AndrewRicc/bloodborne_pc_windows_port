# Bloodborne on PC with DLSS and a multilingual menu: quick guide

This repository is the Windows port of Bloodborne (by Mrsuss60, based on deadinside28's bbport)
with two additions: NVIDIA **DLSS** as an upscaler and an **in-game menu in 12 languages**.

The game is not included: you need your own copy, dumped from a PS4.

## Folder layout

| Item | Purpose |
|---|---|
| `setup.bat` | Does the whole setup: checks the requirements, installs what is missing, builds |
| `Bloodborne.exe` | Starts the launcher without a console window; `setup.bat` builds it and puts a shortcut to it on the desktop |
| `launch_gui.bat` | Starts the graphical launcher |
| `run.bat` | Starts the game directly |
| `modules\game\` | **The game folder (`CUSA03173`) goes here** |
| `modules\dlss-sdk\` | The NVIDIA DLSS SDK ends up here (`setup.bat` downloads it) |
| `modules\icon.ico` | Optional: your own icon for `Bloodborne.exe` and the shortcut |
| `fsr4_shaders\` | The FSR 4 shaders end up here (`setup.bat` downloads them) |
| `out\` | The built game (`bbport.exe` and its DLLs) |
| `user\savedata\` | Save files |
| `bbport.ini` | The settings chosen in the in-game menu |
| `scripts\windows\` | The scripts used by `setup.bat` |
| `gpu\`, `src\`, `third_party\` | The source code |

`modules`, `fsr4_shaders`, `out`, `user`, `bbport.ini` and `Bloodborne.exe` are ignored by git.

## What you need

- **The game**: Bloodborne, European release (`CUSA03173`), updated to **1.09**. The folder must
  contain `eboot.bin`, `dvdroot_ps4` and `sce_sys`.
- **PC**: 64-bit Windows 10 or 11, a CPU with AVX2, a graphics card with Vulkan 1.3 and up-to-date
  drivers.
- **Disk space**: about 15 GB, plus the game.
- **For DLSS**: an NVIDIA RTX card. Without one the game still works, with FSR.
- **Git**, to download the project. If you do not have it:

```powershell
winget install --id Git.Git -e
```

Then close and reopen PowerShell. `setup.bat` installs everything else.

## 1. Download the project

Open PowerShell in the folder where you want to keep it and run:

```powershell
git clone -c core.longpaths=true https://github.com/AndrewRicc/bloodborne_pc_windows_port.git bloodborne
```

```powershell
cd bloodborne
```

Do not download it as a ZIP from GitHub: some components would be missing.

## 2. Put the game in `modules\game`

Copy the `CUSA03173` folder into `modules\game`, like this:

```
bloodborne\modules\game\CUSA03173\eboot.bin
bloodborne\modules\game\CUSA03173\dvdroot_ps4\
bloodborne\modules\game\CUSA03173\sce_sys\
```

The port reads that folder without modifying it. If the game is already somewhere else on the
disk and you do not want to copy it, leave it there: you will point to it in step 4.

## 3. Run the setup

```powershell
.\setup.bat
```

The script checks every requirement and installs only what is missing:

| Check | If missing |
|---|---|
| 64-bit Windows, CPU with AVX2 | It stops: the game cannot run |
| Python 3.10+ | Installs it with winget |
| MSYS2 in `C:\msys64` (the compiler) | Installs it with winget and updates it |
| MSYS2 libraries (about 2.5 GB) | Installs them |
| Visual Studio Build Tools with C++ (about 7 GB) | Installs them, only if you have an RTX card |
| DLSS SDK in `modules\dlss-sdk` (about 1.6 GB) | Downloads it, only if you have an RTX card |
| Source submodules and the FSR fix | Downloads them and applies it |
| FSR 4 shaders in `fsr4_shaders` (about 45 MB) | Downloads them |
| `Bloodborne.exe` and the desktop shortcut | Builds the first and creates the second, after the game is built |

Then it builds: the first time takes 20-30 minutes. At the end you must see `Setup complete.`

Windows may ask you to confirm the installations ("Do you want to allow..." windows): answer yes.

If the script stops with an error, fix what it reports and **run it again**: the steps already
done are skipped. The same applies after updating the code, to rebuild.

Options:

- `.\setup.bat -CheckOnly` reports what is missing without installing or building anything.
- `.\setup.bat -NoDlss` skips Visual Studio and the NVIDIA SDK (about 9 GB less).
- `.\setup.bat -NoShortcut` does not build `Bloodborne.exe` and leaves the desktop alone.

## 4. Start the game

The easy way: double-click the **Bloodborne** shortcut on the desktop. It opens the launcher.

The shortcut points to `Bloodborne.exe` in this folder, which only starts the launcher: the game
itself, its DLLs and your game data stay where they are. Do not move `Bloodborne.exe` elsewhere;
if you move the whole folder, run `setup.bat` again to recreate the shortcut.

**The icon** is not part of this repository. `setup.bat` makes it from the icon inside your own
game copy (`modules\game\CUSA03173\sce_sys\icon0.png`). To use a different one, save it as
`modules\icon.ico` and run `setup.bat` again. With the game outside `modules\game` and no
`modules\icon.ico`, the shortcut gets the default Windows icon.

From the folder, the launcher also starts with a double-click on `launch_gui.bat`, or

```powershell
.\launch_gui.bat
```

If the game is in `modules\game\CUSA03173` the launcher finds it by itself; otherwise press
**Browse** and pick your `eboot.bin`. Keep 60 FPS and 1080p and press **Launch Bloodborne**.

Without the launcher:

```powershell
.\run.bat
```

With the game in another folder:

```powershell
.\run.bat "D:\path\to\CUSA03173"
```

Some stutter on the first start is normal: the game is preparing its shaders.

## 5. In game

- **Ins** on the keyboard, or **L3 + R3** on the controller, opens the menu.
- **Language**: it is the first entry. "Auto" follows the Windows language.
- **Upscaler**: pick **DLSS**. *Native AA* is the highest quality with no FPS gain; *Quality*,
  *Balanced*, *Performance* and *Ultra Performance* lower the internal resolution.
- **Sub-pixel jitter**: it must stay enabled, otherwise DLSS works poorly.
- **F11** or **Shift+Tab** shows the performance HUD.
- The choices are saved to `bbport.ini` when the menu closes.

If the image shimmers with DLSS, try the **DLSS: invert jitter sign** checkbox. If the problem
remains, go back to FSR 3.1 from the same menu.

**DLSS status.** It initializes and runs without errors. Its image quality in gameplay has not
been thoroughly verified yet.

## Common problems

| Symptom | What to do |
|---|---|
| `setup.bat` says the folder is not a git clone | You downloaded the ZIP: redo step 1 with `git clone`. |
| Error `429` while installing the libraries | The MSYS2 server is busy: run `setup.bat` again after a few minutes. |
| `cannot open output file ... bbport.exe` | The game is running: close it and run `setup.bat` again. |
| Python is installed but "cannot find a python.exe" | Close PowerShell, reopen it and run `setup.bat` again. |
| DLSS greyed out in the menu | You need an RTX card with a recent driver. Check that `out` contains `bbdlss.dll` and `nvngx_dlss.dll`; if they are missing run `setup.bat` again. |
| "FSR 4 (INT8)" fails with `missing fsr4_shaders/...` | The FSR 4 shaders are missing: run `setup.bat` again, it downloads them (about 45 MB). |
| "FSR 4.1.1 (INT8)" greyed out, "not supported by the GPU" | Expected on Windows, even with a recent AMD GPU: see the upscaler notes below. |
| Orange text "DLSS unavailable: ..." | The game switched to FSR 3.1 by itself; the text after the colon gives the reason. |
| `No eboot.bin in ...` | The game is not in `modules\game\CUSA03173`: copy it there or pass the path to `run.bat`. |
| The game does not start and crashes right away after earlier runs | A `bbport.exe` from an earlier run may still be in memory: check with `Get-Process bbport`; if it cannot be closed, restart the PC. |
| The game is extremely slow | It is using the integrated graphics: in Settings > System > Display > Graphics add `out\bbport.exe` and choose "High performance". |

## Which upscalers work on Windows

| Upscaler | What it needs |
|---|---|
| FSR 3.1, TAA | Nothing: they work on every GPU. |
| FSR 4 (INT8) | The shaders in `fsr4_shaders` (`setup.bat` downloads them) and a GPU with the required Vulkan features (8-bit integers, integer dot product, compute shader derivatives). Works on recent AMD, NVIDIA and Intel cards. |
| FSR 4.1.1 (INT8) | The `VK_VALVE_shader_mixed_float_dot_product` Vulkan extension in the driver, plus a set of files (`fsr4_411`) that must be generated from AMD's DLL with tools that only run on Linux. In practice it cannot be used on Windows today. |
| DLSS | An NVIDIA RTX card. |

To find out whether the driver exposes the FSR 4.1.1 extension:

```powershell
vulkaninfo | Select-String mixed_float_dot_product
```

If it prints nothing, the driver does not support it and the entry stays greyed out.

## Known limits of the setup script

- MSYS2 must be in `C:\msys64`, the default path.
- The branches that install missing programs have seen little testing on clean PCs: if one fails,
  the message says which step, and the table above covers the known cases.

## Building the Windows installer

For people who should not have to build anything, the port can be packed into a normal Windows
installer. It checks the PC, asks for the game folder, installs the port already built and
creates the shortcuts. It needs no MSYS2 and no Visual Studio on the target PC, only Python.

Build the port first (`setup.bat`), then:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows\make_installer.ps1
```

The result is `dist\bbport-setup-<date>-<commit>.exe` (about 55 MB). It needs Inno Setup 6 on the
PC that builds it (`winget install --id JRSoftware.InnoSetup -e`).

What the installer contains, and what it never contains:

| In the package | Never in the package |
|---|---|
| `bbport.exe`, its DLLs, the launcher and its scripts | Any game file |
| The FSR 4 shaders (AMD, MIT licence) | The game's icon: it is made on each PC from that user's own game copy |
| `LICENSE` (GPL) and `SOURCE.txt` with the repository and commit | Files prepared from the game (`out\boot*.bin`, `eboot.elf`), saves, settings |
| | NVIDIA DLSS files, unless you ask for them |

The script refuses to finish if it finds one of the files on the right in the staging folder.

**Commit and push before building a package you publish.** The port is GPL: who receives the
installer must be able to get the exact source, and `SOURCE.txt` names the commit it was built from.

**DLSS.** By default the installer has no DLSS: NVIDIA's SDK licence does not allow using the SDK
in a way that subjects it to an open source licence, and the port is GPL. `-WithDlss` packs
`bbdlss.dll` and `nvngx_dlss.dll` anyway; keep such a package private. Anyone can still get DLSS
by building from source with `setup.bat`.

Unattended installation, for testing:

```powershell
.\bbport-setup-<version>.exe /VERYSILENT /SUPPRESSMSGBOXES /GAME="D:\path\to\CUSA03173"
```

Uninstalling (Windows Settings > Apps) removes the port and the files it prepared from the game;
saves (`user\`) and settings stay in the installation folder.

## Tests

After building, run the tests that need neither a graphics card nor the game:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows\run_tests.ps1
```

They check the menu table (one text per language, same placeholders), that the launcher and the
preparation scripts compile, four Python test suites and five C++ test programs (settings and
upscaler fallback, shader recompiler, motion vectors, UI composition). The other tests in `tests\`
were written for Linux or need a Vulkan device, and are not run.

After building an installer, this installs it silently into a temporary folder, checks what it
installed and that nothing of the game is inside, then uninstalls it:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\windows\test_installer.ps1
```

## Automatic builds and releases (GitHub Actions)

`.github\workflows\windows-installer.yml` builds the port on a GitHub Windows machine, runs both
test scripts above and packs the installer:

- **every push to `master` and every pull request**: build, tests, installer. The installer is
  attached to the run as an artifact (Actions tab > the run > Artifacts). A failed test stops the
  run before any installer is made.
- **a version tag**: the same, then the installer is published as a GitHub release.

To publish a release:

```powershell
git tag v0.3.0
```

```powershell
git push origin v0.3.0
```

Use the name of your own remote in place of `origin` if it is called differently.

Installers built there never contain DLSS: the machines have no NVIDIA card and the workflow asks
for a build without it. On a fork, GitHub Actions must be enabled once in the repository's
Actions tab.

## Licences

The port is under the GNU GPL, version 2 or later (`LICENSE`). The DLSS bridge in `gpu\dlss`
(`bbdlss.h`, `bbdlss.cpp`, `build_bbdlss.bat`) is a separate library under the MIT licence
(`gpu\dlss\LICENSE.txt`); the NVIDIA DLSS SDK it links stays under NVIDIA's own licence.

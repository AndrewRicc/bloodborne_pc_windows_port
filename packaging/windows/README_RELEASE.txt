Bloodborne PC Port (bbport) - Windows build with multilingual menu
=================================================================

This package contains the port only. It contains no game files: you need your own copy of
Bloodborne (European release CUSA03173, updated to 1.09), dumped from your PS4. The folder
must contain eboot.bin, dvdroot_ps4 and sce_sys.

Not affiliated with Sony Interactive Entertainment, FromSoftware, AMD or NVIDIA.

Starting the game
-----------------
Use the "Bloodborne PC Port" shortcut (desktop or Start menu), or Bloodborne.exe in this
folder. It opens the launcher: check the path of your eboot.bin, then press "Launch Bloodborne".
Some stutter on the first start is normal: the game is preparing its shaders.

In game: Insert (or L3 + R3 on the controller) opens the menu. The first entry is the language.
F11 or Shift+Tab shows the performance HUD.

Requirements
------------
64-bit Windows 10 or 11, a CPU with AVX2, a graphics card with Vulkan 1.3 and current drivers,
Python 3.10 or newer (with tkinter).

Upscalers
---------
FSR 3.1, FSR 4 (INT8) and TAA are included. FSR 4.1.1 cannot be used on Windows today.
DLSS is only present when this package was built with it; a package without it shows DLSS
greyed out in the menu. To get DLSS, build the port from source (see GUIDE.md in the repository).

Your files
----------
Saves:     user\savedata   (kept when you uninstall or update)
Settings:  bbport.ini, launcher_settings.json
Crash logs: out\crash_*.log

Saves from an earlier installation: the installer asks for them, and the launcher has an
"Import saves..." button. Pick the old installation folder or its user\savedata folder. Saves
already present are moved to user\savedata_backup_<date>, never overwritten.

The shortcut icon is made on your PC from the icon inside your own game copy.

Source code and licences
------------------------
The port is free software under the GNU GPL, version 2 or later (LICENSE). SOURCE.txt names the
repository and the exact commit this package was built from. The FSR 4 shaders are under AMD's
MIT licence (fsr4_shaders\LICENSE-FSR4-v07.txt). The DLLs in out\ come from the MSYS2 MinGW-w64
packages (FFmpeg, SDL3, Vulkan loader and others) under their own free licences.

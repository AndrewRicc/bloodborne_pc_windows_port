# modules

Everything the port uses that is not part of the source code lives here. Git ignores the contents
of these folders: nothing you put in them ends up in the repository.

| Folder | What goes in it | Who fills it |
|---|---|---|
| `game\CUSA03173\` | Your copy of Bloodborne (European release, 1.09): `eboot.bin`, `dvdroot_ps4`, `sce_sys` | You, by copying the folder |
| `dlss-sdk\` | The NVIDIA DLSS SDK, used only to build | `setup.bat` |

If the game is already somewhere else on the disk you do not need to copy it: pass the path to
`run.bat` or pick it in the launcher. See `GUIDE.md` in the main folder.

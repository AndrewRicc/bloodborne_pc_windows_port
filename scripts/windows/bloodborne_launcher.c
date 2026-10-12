// SPDX-License-Identifier: GPL-2.0-or-later
// Bloodborne.exe: a small windowless starter for the graphical launcher, so that the port can be
// started from a desktop shortcut with its own icon. It runs launch_gui.bat from the folder this
// executable is in, without showing a console. setup.bat builds it (with the icon, when there is
// one) and creates the shortcut.
#include <windows.h>
#include <wchar.h>

static int Fail(const wchar_t* text) {
    MessageBoxW(NULL, text, L"Bloodborne", MB_OK | MB_ICONERROR);
    return 1;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR arguments, int show) {
    (void)instance; (void)previous; (void)arguments; (void)show;
    wchar_t dir[1024];
    DWORD length = GetModuleFileNameW(NULL, dir, 1024);
    if (length == 0 || length >= 1024) return Fail(L"Cannot find the folder of Bloodborne.exe.");
    wchar_t* slash = wcsrchr(dir, L'\\');
    if (!slash) return Fail(L"Cannot find the folder of Bloodborne.exe.");
    *slash = 0;
    if (!SetCurrentDirectoryW(dir)) return Fail(L"Cannot open the folder of Bloodborne.exe.");
    if (GetFileAttributesW(L"launch_gui.bat") == INVALID_FILE_ATTRIBUTES) {
        return Fail(L"launch_gui.bat is missing next to Bloodborne.exe.\n"
                    L"Keep Bloodborne.exe in the port's folder and use the desktop shortcut.");
    }
    wchar_t cmd[MAX_PATH + 16];
    UINT system = GetSystemDirectoryW(cmd, MAX_PATH);
    if (system == 0 || system >= MAX_PATH) return Fail(L"Cannot find cmd.exe.");
    wcscat(cmd, L"\\cmd.exe");
    SetEnvironmentVariableW(L"BB_LAUNCHER_NO_CONSOLE", L"1");
    // "< NUL": the script's "pause" after an error must not wait for a key nobody can press.
    wchar_t command[] = L"cmd.exe /d /c .\\launch_gui.bat < NUL";
    STARTUPINFOW startup = {.cb = sizeof(startup)};
    PROCESS_INFORMATION process;
    if (!CreateProcessW(cmd, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, dir, &startup, &process)) {
        return Fail(L"Cannot start launch_gui.bat.");
    }
    // launch_gui.bat only reports a missing Python (exit code 1) and then starts the launcher.
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(process.hProcess, &code);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (code != 0) return Fail(L"Python 3.10+ was not found. Run setup.bat in the port's folder.");
    return 0;
}

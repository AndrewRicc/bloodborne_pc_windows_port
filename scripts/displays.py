"""The monitor list of the launcher.

bb-gpu-capabilities.exe --displays prints one line per monitor in SDL's order:
"name<TAB>WxH<TAB>primary (1 or 0)". The game reads BB_DISPLAY as a 1-based number in that order
or as part of a monitor's name; anything that matches nothing means the primary monitor.

Library (launcher.py): connected(tool), choices(text, saved), parse(text).
"""
import subprocess
from pathlib import Path


def parse(text: str) -> list[tuple[str, str]]:
    """(BB_DISPLAY value, label) per monitor. The value is the name when the game would pick that
    same monitor from it, otherwise the 1-based number: the name is "?", all digits, not ASCII,
    padded with blanks or blank, or inside another monitor's name (ignoring case)."""
    rows = []
    for line in text.splitlines():
        fields = line.rstrip("\r").split("\t")
        if len(fields) == 3:
            rows.append(fields)
    names = [name for name, _, _ in rows]
    result = []
    for number, (name, size, primary) in enumerate(rows, 1):
        others = [other.lower() for index, other in enumerate(names, 1) if index != number]
        by_number = (name == "?" or name.isdigit() or not name.isascii() or name != name.strip()
                     or not name.strip() or any(name.lower() in other for other in others))
        value = str(number) if by_number else name
        label = f"{number}: {name} ({size})" + (", primary" if primary == "1" else "")
        result.append((value, label))
    return result


def choices(text: str, saved: str) -> list[tuple[str, str]]:
    """The drop-down entries: the primary monitor, the connected ones, and the saved choice when
    that monitor is not connected any more."""
    entries = [("", "Primary monitor")] + parse(text)
    if saved and saved not in [value for value, _ in entries]:
        entries.append((saved, f"{saved} (not connected)"))
    return entries


def connected(tool: Path) -> str:
    """The tool's --displays output; "" on any failure or after 5 seconds."""
    try:
        done = subprocess.run([str(tool), "--displays"], stdin=subprocess.DEVNULL,
                              capture_output=True, text=True, encoding="utf-8", errors="replace",
                              timeout=5,
                              creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    except (OSError, subprocess.SubprocessError):
        return ""
    return done.stdout if done.returncode == 0 else ""

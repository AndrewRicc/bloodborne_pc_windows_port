#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Imports save data from another installation of the port into this one.

The saves live in <user>/savedata/<user id>/<title id>/ (runtime_savedata.c). The source may be
any folder around them: the old installation folder, its user folder, the savedata folder, a
<user id> folder or a <title id> folder. Saves already present for the same user are moved to
<user>/savedata_backup_<date>_<time> first: nothing is overwritten or deleted.

Used by the launcher ("Import saves...") and by the Windows installer:
    python scripts/savedata_import.py <source folder> --user <user folder> [--check]
Exit code 0: imported (or, with --check, something to import was found); 1: nothing found;
2: the copy failed.
"""
import argparse
import re
import shutil
import sys
from datetime import datetime
from pathlib import Path

TITLE = re.compile(r"^[A-Z]{4}\d{5}$")
MAX_DEPTH = 4  # a source folder is searched this deep for a savedata folder


def _titles(user_dir: Path) -> list[str]:
    """Title ids that have save folders in a <user id> folder."""
    try:
        return sorted(p.name for p in user_dir.iterdir() if p.is_dir() and TITLE.match(p.name))
    except OSError:
        return []


def _users(savedata: Path) -> dict[str, Path]:
    """<user id> folders of a savedata folder that hold at least one title."""
    try:
        return {p.name: p for p in sorted(savedata.iterdir())
                if p.is_dir() and p.name.isdigit() and _titles(p)}
    except OSError:
        return {}


def find_saves(source) -> dict[str, Path]:
    """The save folders in or around `source`, as {user id: folder with the title folders}."""
    source = Path(source)
    if not source.is_dir():
        return {}
    if TITLE.match(source.name):  # .../savedata/1/CUSA00207
        return {source.parent.name if source.parent.name.isdigit() else "1": source.parent}
    if _titles(source):  # .../savedata/1
        return {source.name if source.name.isdigit() else "1": source}
    if _users(source):  # .../savedata
        return _users(source)
    # An installation folder or its user folder: look for savedata below it.
    pending = [(source, 0)]
    while pending:
        folder, depth = pending.pop(0)
        candidate = folder / "savedata"
        if _users(candidate):
            return _users(candidate)
        if depth < MAX_DEPTH:
            try:
                pending += [(p, depth + 1) for p in sorted(folder.iterdir())
                            if p.is_dir() and not p.is_symlink()]
            except OSError:
                pass
    return {}


def describe(found: dict[str, Path]) -> str:
    return "; ".join(f"user {user}: {', '.join(_titles(folder))}" for user, folder in found.items())


def import_saves(source, user_dir) -> str:
    """Copies the saves found around `source` into <user_dir>/savedata. Returns a summary.
    Raises FileNotFoundError when there is nothing to import, ValueError when the source is
    this installation's own save folder."""
    found = find_saves(source)
    if not found:
        raise FileNotFoundError(f"no save data found in {source}")
    savedata = Path(user_dir) / "savedata"
    backup = None
    for user, folder in found.items():
        target = savedata / user
        if target.exists() and target.resolve() == folder.resolve():
            raise ValueError("these are already the saves of this installation")
        if target.exists() and any(target.iterdir()):
            if backup is None:
                backup = Path(user_dir) / f"savedata_backup_{datetime.now():%Y%m%d_%H%M%S}"
            backup.mkdir(parents=True, exist_ok=True)
            shutil.move(str(target), str(backup / user))
        target.parent.mkdir(parents=True, exist_ok=True)
        # Everything in the <user id> folder: the title folders and their .memory companions.
        shutil.copytree(folder, target, dirs_exist_ok=True)
    summary = f"Imported save data ({describe(found)}) into {savedata}"
    if backup:
        summary += f". The saves that were there are in {backup}"
    return summary


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("source", help="folder with the saves, or any folder around them")
    parser.add_argument("--user", required=True, help="user folder of this installation")
    parser.add_argument("--check", action="store_true", help="only report what would be imported")
    args = parser.parse_args()
    found = find_saves(args.source)
    if not found:
        print(f"No save data found in {args.source}")
        return 1
    if args.check:
        print(f"Found save data: {describe(found)}")
        return 0
    try:
        print(import_saves(args.source, args.user))
    except ValueError as error:
        print(f"Nothing imported: {error}")
        return 1
    except OSError as error:
        print(f"Import failed: {error}")
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())

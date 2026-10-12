"""Checks the in-game menu table (gpu/shim/bbport_i18n.cpp): every entry has one text per language
and the same printf placeholders in all of them. Usage: check_i18n.py <bbport_i18n.cpp>"""
import re
import sys

sys.stdout.reconfigure(encoding="utf-8")
s = open(sys.argv[1], encoding="utf-8").read()
body = s[s.index("constexpr Entry kEntries[] = {"):s.index("// clang-format on")]
entries = re.findall(r"\{(\w+), \{(.*?)\}\},\n", body, re.S)
fmt = re.compile(r"%[-+ #0]*\d*(?:\.\d+)?[a-z]")
stray = re.compile(r"%(?![-+ #0]*\d*(?:\.\d+)?[a-z])")
bad = 0
for name, content in entries:
    parts, cur = [], ""
    for lit, comma in re.findall(r'"((?:[^"\\]|\\.)*)"|(,)', content):
        if comma:
            parts.append(cur)
            cur = ""
        else:
            cur += lit
    parts.append(cur)
    if len(parts) != 12:
        print("COUNT", name, len(parts))
        bad += 1
        continue
    ref = fmt.findall(parts[0])
    for i, p in enumerate(parts):
        if fmt.findall(p) != ref:
            print("FMT", name, i, fmt.findall(p), ref)
            bad += 1
        if stray.search(p):
            print("STRAY %", name, i)
            bad += 1
print(len(entries), "entries, problems:", bad)
sys.exit(1 if bad or not entries else 0)

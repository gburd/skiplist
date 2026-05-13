#!/usr/bin/env python3
"""Print per-file coverage from gcovr's JSON summary."""
import json
import sys

with open(sys.argv[1]) as f:
    d = json.load(f)

for entry in d["files"]:
    fn = entry["filename"]
    lp = entry["line_percent"]
    bp = entry["branch_percent"]
    print(f"{fn:50s} {lp:5.1f}% lines  {bp:5.1f}% branches")
print()
print(f"OVERALL: {d['line_percent']:.1f}% lines, {d['branch_percent']:.1f}% branches")

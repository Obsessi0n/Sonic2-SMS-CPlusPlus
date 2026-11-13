"""Checks that tests/physics_tests.cpp and its Python mirror expect the same numbers, in the same order."""
import re
import sys
from pathlib import Path

cpp = Path(sys.argv[1]).read_text()
py = Path(sys.argv[2]).read_text()

# The expected value is the last whole number before the message string of each equal(...).
pattern = re.compile(r'equal\((?:[^"\n]*?),\s*(-?\d+)\s*,\s*f?"')
a = [int(m) for m in pattern.findall(cpp)]
b = [int(m) for m in pattern.findall(py)]
print(len(a), "numbers in the C++,", len(b), "in the mirror")
for i, (x, y) in enumerate(zip(a, b)):
    if x != y:
        print("differ at", i, ":", x, "in the C++,", y, "in the mirror; before it:", a[max(0, i - 3):i])
        break
else:
    print("the same" if len(a) == len(b) else "one is longer: " + str(a[len(b):] or b[len(a):]))

# The lists of numbers written as arrays in the C++ and as tuples in the mirror.
arrays = re.findall(r'=\s*\{\s*((?:-?\d+\s*,\s*)+-?\d+)\s*\}', cpp)
print("arrays in the C++:")
for text in arrays:
    print("  ", [int(v) for v in text.split(",")])

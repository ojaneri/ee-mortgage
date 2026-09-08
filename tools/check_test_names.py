#!/usr/bin/env python3
"""Check that every test name mentioned in the docs still exists.

The markdown under requirements/ names specific tests all over the place, which
is useful right up until somebody renames one and the docs quietly start lying.
This walks tests/*.cpp for the real names and every Suite.TestName in the
markdown, and complains about anything it cannot find.

Names written as a gtest filter (trailing '*') are treated as prefixes, since
that is what they are.

Osvaldo Janeri Filho <janeri@gmail.com>
"""
import glob
import re
import sys

TEST_RE = re.compile(r"^TEST\((\w+),\s*(\w+)\)", re.M)
# Suite.Name, optionally a gtest-filter prefix ending in '*'
MENTION_RE = re.compile(r"\b([A-Z]\w+)\.([A-Za-z_]\w*)(\*?)")


def real_test_names():
    names = set()
    for path in sorted(glob.glob("tests/*.cpp")):
        for suite, name in TEST_RE.findall(open(path).read()):
            names.add(f"{suite}.{name}")
    return names


def main():
    names = real_test_names()
    if not names:
        print("no tests found - run this from the repository root", file=sys.stderr)
        return 2
    suites = {n.split(".", 1)[0] for n in names}

    problems = []
    for doc in sorted(glob.glob("README.md") + glob.glob("requirements/*.md")):
        text = open(doc).read()
        for suite, name, star in MENTION_RE.findall(text):
            if suite not in suites:
                continue  # some other dotted thing, not a test name
            mentioned = f"{suite}.{name}"
            if star:
                if not any(n.startswith(mentioned) for n in names):
                    problems.append((doc, mentioned + "*"))
            elif mentioned not in names:
                problems.append((doc, mentioned))

    for doc, name in problems:
        print(f"{doc}: no such test: {name}")
    print(f"{len(names)} tests, {len(problems)} stale reference(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())

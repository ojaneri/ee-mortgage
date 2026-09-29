#!/usr/bin/env python3
"""Build docs/console.html, a replay of the two terminal sessions.

The page is generated from docs/run_correct.txt and docs/run_buggy.txt, which
are real transcripts, so nothing on screen is invented: the replay types each
"$ command" line and then streams the output that command actually printed.
Re-run this whenever the transcripts are regenerated.

The transcripts are embedded in the page rather than fetched, so it works when
opened straight from disk (file:// blocks fetch in most browsers).

Osvaldo Janeri Filho <janeri@gmail.com>
Marcos Klosowski Junior <marcosj.2017@alunos.utfpr.edu.br>
"""
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
SESSIONS = [
    ("correct", "Correct — mortgage.cpp", DOCS / "run_correct.txt"),
    ("buggy", "Buggy — mortgage_buggy.cpp", DOCS / "run_buggy.txt"),
]
TEMPLATE = pathlib.Path(__file__).with_name("console_template.html")


def main():
    data = [
        {"id": key, "label": label, "text": path.read_text(encoding="utf-8")}
        for key, label, path in SESSIONS
    ]
    # "</" would close the <script> element the JSON sits in.
    payload = json.dumps(data, ensure_ascii=False).replace("</", "<\\/")
    html = TEMPLATE.read_text(encoding="utf-8").replace("/*__SESSIONS__*/[]", payload)
    out = DOCS / "console.html"
    out.write_text(html, encoding="utf-8")
    print(f"wrote {out.relative_to(ROOT)} ({len(html) // 1024} KiB)")


if __name__ == "__main__":
    main()

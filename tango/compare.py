#!/usr/bin/env python3
"""Run paired measurements and write their results to the Actions job summary."""
import argparse
import os
from pathlib import Path
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("corpus", type=Path, help="HarfBuzz source tree supplying both workers")
    parser.add_argument("output", type=Path)
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--seconds", type=int, default=5)
    parser.add_argument("--title", help="Heading for the Markdown report")
    parser.add_argument("--labels", nargs=2, metavar=("BASE", "CANDIDATE"),
                        default=("Baseline", "Candidate"), help="Column labels for the two workers")
    args = parser.parse_args()
    if min(args.repetitions, args.seconds) < 1:
        parser.error("repetitions and seconds must be positive")

    cases = [
        ("perf/fonts/Roboto-Regular.ttf", "en-thelittleprince.txt"),
        ("perf/fonts/Amiri-Regular.ttf", "fa-thelittleprince.txt"),
        ("test/subset/data/fonts/SourceSerifVariable-Roman.ttf", "react-dom.txt"),
        ("perf/fonts/NotoNastaliqUrdu-Regular.ttf", "fa-thelittleprince.txt"),
        ("perf/fonts/NotoNastaliqUrdu-Regular.ttf", "fa-words.txt"),
    ]
    summary = [f"### {args.title}", ""] if args.title else []
    summary.extend([f"| Font / text | Trial | {args.labels[0]} | {args.labels[1]} | Difference |",
                    "|---|---:|---:|---:|---:|"])
    for font_path, text in cases:
        font = Path(font_path).name
        env = dict(os.environ, HARF_FONT=str(args.corpus / font_path),
                   HARF_TEXT=str(args.corpus / "perf/texts" / text), NO_COLOR="1")
        name = f"{font}/{text}"
        for trial in range(1, args.repetitions + 1):
            samples = args.output / font / text / str(trial)
            samples.mkdir(parents=True, exist_ok=False)
            cmd = [str(args.candidate.resolve()), "compare", str(args.baseline.resolve()),
                   "--time", str(args.seconds), "--dump", str(samples)]
            run = subprocess.run(cmd, env=env, text=True, stdout=subprocess.PIPE)
            print(run.stdout, end="", flush=True)
            match = re.search(re.escape(name) +
                              r"\s+\[\s*([^\]]+?)\s+\.\.\.\s+([^\]]+?)\s*\]\s+([+-][\d.]+%)(\*)?",
                              run.stdout)
            # Tango also exits 1 for a measured regression. Keep that row in
            # the report. Missing results and worker failures fail the job.
            csv = samples / f"{font}-{text}.csv"
            if (not match or not csv.is_file() or not csv.stat().st_size or
                    run.returncode not in (0, 1) or
                    (run.returncode == 1 and not (match[3].startswith("+") and match[4]))):
                raise SystemExit(f"{name} trial {trial} did not complete (exit {run.returncode})")
            marker = " (significant)" if match[4] else ""
            summary.append(f"| {name} | {trial} | {match[1]} | {match[2]} | {match[3]}{marker} |")

    report = "\n".join(summary) + "\n"
    print(report)
    (args.output / "summary.md").write_text(report)
    if step_summary := os.getenv("GITHUB_STEP_SUMMARY"):
        with open(step_summary, "a") as out:
            out.write(report + "\n")


if __name__ == "__main__":
    main()

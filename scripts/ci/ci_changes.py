"""Cheap, conservative routing for hosted checks; never a physics certificate.

A successful PR check is reused for its merge only when the source trees match.
API/diff uncertainty runs checks rather than silently skipping them.
"""

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
import subprocess
import sys
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


def git(*args: str) -> str:
    return subprocess.check_output(["git", *args], text=True).strip()


def classify(paths: list[str]) -> dict[str, bool]:
    flags = dict.fromkeys(("python", "lint", "integrity", "web", "engine"), False)
    for path in paths:
        if path in ("scripts/ci/ci_changes.py", "scripts/ci/test_ci_changes.py"):
            return dict.fromkeys(flags, True)
        if path.startswith("scripts/") or path in ("requirements.txt", "requirements-dev.txt", "pyproject.toml"):
            flags["python"] = flags["lint"] = True
        # Browser serving is Python code; strict native backends and the shared
        # JS constants also have direct Python regression consumers.
        if path.startswith("engine/web/") and path.endswith(".py"):
            flags["python"] = flags["lint"] = True
        if path == "engine/web/js/constants.js" or path.startswith("engine/strict/"):
            flags["python"] = True
        if path.startswith("docs/") or path.endswith(".md") or path.startswith("scripts/") or path == ".gitignore":
            flags["integrity"] = True
        # These maintained reference files are consumed directly by Python tests.
        if path in (
            "docs/theory/01_reference/dimensional_map.json",
            "docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md",
            "docs/theory/07_assessment/core_ledgers/LEDGER.md",
            "docs/theory/07_assessment/core_ledgers/LEDGER_INDEX.md",
            "docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md",
            "docs/theory/07_assessment/core_ledgers/TRACKER_RESOLVED_ITEMS.md",
            "docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS_INDEX.md",
        ):
            flags["python"] = True
        if path.startswith("engine/web/") or path in (
            "package.json", "package-lock.json", ".dependency-cruiser.json", ".stylelintrc.json",
        ) or (path.startswith("tsconfig") and path.endswith(".json")):
            flags["web"] = True
        if path.startswith(("engine/include/", "engine/src/", "engine/tests/", "engine/cmake/", "engine/cuda/")):
            flags["engine"] = True
        if path == "engine/CMakeLists.txt":
            flags["engine"] = True
        if path == ".github/workflows/ci.yml":
            flags["python"] = flags["lint"] = flags["integrity"] = True
        if path == ".github/workflows/web-source-gate.yml":
            flags["web"] = True
        if path == ".github/workflows/engine-source-gate.yml":
            flags["engine"] = True
    return flags


def changed_paths(event_name: str, event: dict) -> list[str] | None:
    if event_name == "pull_request":
        before, after = (event["pull_request"][side]["sha"] for side in ("base", "head"))
        comparison = f"{before}...{after}"
    elif event_name == "push":
        before, after = event.get("before"), event.get("after")
        if not before or before == "0" * 40:
            return None
        comparison = f"{before}..{after}"
    else:
        return None
    return subprocess.check_output(["git", "diff", "--name-only", "-z", comparison]).decode().rstrip("\0").split("\0")


def github_json(endpoint: str) -> dict | list:
    token = os.environ.get("GH_TOKEN", "")
    headers = {"Accept": "application/vnd.github+json", "X-GitHub-Api-Version": "2022-11-28"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    request = Request(f"{os.environ.get('GITHUB_API_URL', 'https://api.github.com')}{endpoint}", headers=headers)
    with urlopen(request, timeout=10) as response:
        return json.load(response)


def checked_pr_merge(event_name: str, event: dict, workflow: str) -> bool:
    """Reuse only a successful PR run of this workflow on the identical tree."""
    if event_name != "push" or len(git("rev-list", "--parents", "-n", "1", "HEAD").split()) != 3:
        return False
    sha = event["after"]
    repository = os.environ["GITHUB_REPOSITORY"]
    for pr in github_json(f"/repos/{repository}/commits/{sha}/pulls"):
        if not pr.get("merged_at") or pr.get("merge_commit_sha") != sha:
            continue
        head = pr["head"]["sha"]
        if git("rev-parse", f"{head}^{{tree}}") != git("rev-parse", "HEAD^{tree}"):
            continue
        runs = github_json(f"/repos/{repository}/actions/workflows/{workflow}/runs?event=pull_request&head_sha={head}&per_page=100")
        attempts = runs["workflow_runs"]
        # API order is creation order, not rerun-attempt order. An older run
        # may have a newly pending attempt; every returned run must be final.
        if not attempts or runs.get("total_count", len(attempts)) > len(attempts):
            continue
        if any(run.get("status") != "completed" for run in attempts):
            continue
        dated = []
        for run in attempts:
            timestamp = run.get("run_started_at") or run.get("created_at")
            try:
                when = datetime.fromisoformat(timestamp.replace("Z", "+00:00"))
            except (AttributeError, ValueError):
                break
            if when.tzinfo is None:
                break
            dated.append((when, run))
        if len(dated) != len(attempts):
            continue
        latest_time = max(when for when, _ in dated)
        latest = [run for when, run in dated if when == latest_time]
        # Missing or tied attempt times cannot prove which result is current.
        if len(latest) == 1 and latest[0].get("conclusion") == "success":
            return True
    return False


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--workflow", required=True)
    parser.add_argument("--scope", choices=("ci", "web", "engine"), default="ci")
    args = parser.parse_args()
    event_name = os.environ.get("GITHUB_EVENT_NAME", "workflow_dispatch")
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text(encoding="utf-8"))
    try:
        paths = changed_paths(event_name, event)
        flags = classify(paths) if paths is not None else dict.fromkeys(classify([]), True)
    except (KeyError, subprocess.CalledProcessError, UnicodeDecodeError) as error:
        print(f"Could not determine changed paths; running checks: {error}", file=sys.stderr)
        flags = dict.fromkeys(classify([]), True)
    duplicate = False
    if any(flags.values()):
        try:
            duplicate = checked_pr_merge(event_name, event, args.workflow)
        except (KeyError, subprocess.CalledProcessError, HTTPError, URLError, TimeoutError, json.JSONDecodeError) as error:
            print(f"Could not verify prior PR success; running checks: {error}", file=sys.stderr)
    if duplicate:
        flags = dict.fromkeys(flags, False)
    flags["run"] = any(flags[key] for key in ("python", "lint", "integrity")) if args.scope == "ci" else flags[args.scope]
    print(json.dumps({"reuse_successful_pr": duplicate, **flags}, sort_keys=True))
    with Path(os.environ["GITHUB_OUTPUT"]).open("a", encoding="utf-8") as output:
        for key, value in flags.items():
            output.write(f"{key}={str(value).lower()}\n")


if __name__ == "__main__":
    main()

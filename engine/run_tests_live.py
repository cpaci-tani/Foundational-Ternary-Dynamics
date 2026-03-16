#!/usr/bin/env python3
"""Live CTest runner with Server-Sent Events streaming.

Serves the test dashboard and runs each test executable directly,
streaming output line-by-line via SSE for real-time visibility.

Usage:
    python engine/run_tests_live.py
    python engine/run_tests_live.py --port 8081 --build-dir engine/build_cuda
"""

import argparse
import http.server
import json
import os
import re
import subprocess
import sys
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlparse, parse_qs

# ── Shared config (set in main) ─────────────────────────────────────────

BUILD_DIR = "engine/build"
CONFIG = "Release"
WEB_DIR = "engine/web"

# ── Regex for parsing test output ────────────────────────────────────────

RE_CHECK = re.compile(r"^\s{2,}(PASS|FAIL)\s{2}(.+)")
RE_SECTION = re.compile(r"^\s*=+\s*(.*?)\s*=+\s*$")

# ── Category map (imported from run_tests_json.py logic) ─────────────────

CATEGORY_RULES = [
    ({"constants", "lorentz", "lattice", "ontic_chain"}, "Core"),
    ({"born_infeld", "energy", "gauss", "stress_energy", "thermodynamics"}, "Core"),
    ({"lagrangian", "magnetic_lagrangian", "dissipation", "variational_coulomb"}, "Lagrangian"),
    ({"maxwell", "em_energy_conservation", "continuity", "poynting", "larmor"}, "Electromagnetism"),
    ({"dipole_radiation", "dispersion_relation", "thomson_scattering", "em_fields"}, "Electromagnetism"),
    ({"gauss_convergence", "lorentz_force", "selective_damping"}, "Electromagnetism"),
    ({"wave_collapse", "wave_speed", "interference", "gauge", "polarization"}, "Waves & Gauge"),
    ({"momentum", "magnetic", "flux_mediated", "entanglement"}, "Waves & Gauge"),
    ({"genesis", "gravity_dynamics", "annihilation", "annihilation_conservation"}, "Dynamics"),
    ({"portable_field", "particle_lifetime", "vortex"}, "Dynamics"),
    ({"voxel_properties", "lattice_operators", "discrete_operators"}, "Operators"),
    ({"bridge_dynamics", "csv_export", "logic_engine"}, "Infrastructure"),
    ({"poisson_coulomb", "energy_tracking", "energy_conservation"}, "Energy & Poisson"),
    ({"selffield_profile", "wavepacket"}, "Energy & Poisson"),
    ({"particle_engine", "scale_bridge", "hydrogen_scale1", "multiscale_bridge"}, "Multi-Scale"),
    ({"atom_engine", "atom_scale_bridge"}, "Atom Engine"),
    ({"dual_substrate"}, "Dual Substrate"),
    ({"latency_field"}, "Latency"),
    ({"falsifiability"}, "Falsifiability"),
    ({"inflation", "dark_matter", "cosmological_constant"}, "Cosmology"),
    ({"consciousness", "sloop"}, "Consciousness"),
    ({"lorentz_invariance", "electroweak", "hydrogen_em_only"}, "Precision"),
    ({"correlations", "ensemble", "spectral", "tracker", "light", "benchmark"}, "Analysis"),
]

PREFIX_RULES = [
    ("pe_", "PE Extensions"),
    ("ae_", "AE Extensions"),
    ("campaign_ae_", "AE Campaigns"),
    ("campaign_pe_", "PE Campaigns"),
    ("campaign_poisson_", "Poisson Campaigns"),
    ("campaign_", "Campaigns"),
]


def categorize(name):
    for name_set, cat in CATEGORY_RULES:
        if name in name_set:
            return cat
    for prefix, cat in PREFIX_RULES:
        if name.startswith(prefix):
            return cat
    return "Other"


# ── Get test list from ctest ─────────────────────────────────────────────

def get_test_list():
    """Get all registered tests with their executable paths."""
    result = subprocess.run(
        ["ctest", "--test-dir", BUILD_DIR, "-C", CONFIG, "--show-only=json-v1"],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        # Fallback to ctest -N
        return get_test_list_fallback()

    data = json.loads(result.stdout)
    tests = []
    for i, t in enumerate(data.get("tests", []), 1):
        name = t["name"]
        cmd = t.get("command", [])
        tests.append({
            "index": i,
            "name": name,
            "command": cmd,
            "category": categorize(name),
        })
    return tests


def get_test_list_fallback():
    """Fallback: get test names from ctest -N."""
    result = subprocess.run(
        ["ctest", "--test-dir", BUILD_DIR, "-C", CONFIG, "-N"],
        capture_output=True, text=True
    )
    tests = []
    for line in result.stdout.splitlines():
        m = re.match(r"\s+Test\s+#(\d+):\s+(.+)", line)
        if m:
            name = m.group(2).strip()
            tests.append({
                "index": int(m.group(1)),
                "name": name,
                "command": [],
                "category": categorize(name),
            })
    return tests


# ── Run state (tracks whether a run is in progress) ─────────────────────

run_lock = threading.Lock()
is_running = False
stop_requested = False
current_proc = None  # currently running subprocess (for force-stop)


# ── SSE Handler ──────────────────────────────────────────────────────────

class LiveTestHandler(http.server.SimpleHTTPRequestHandler):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def do_POST(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/stop":
            self.handle_stop()
        else:
            self.send_response(404)
            self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path == "/api/list":
            self.handle_list()
        elif parsed.path == "/api/run":
            params = parse_qs(parsed.query)
            self.handle_run(params)
        elif parsed.path == "/api/status":
            self.handle_status()
        else:
            super().do_GET()

    def log_message(self, format, *args):
        # Suppress noisy HTTP logs except for API calls
        try:
            first = str(args[0]) if args else ""
            if "/api/" in first:
                super().log_message(format, *args)
        except Exception:
            pass

    def handle_list(self):
        """Return list of all registered tests as JSON."""
        tests = get_test_list()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps(tests).encode())

    def handle_status(self):
        """Return whether a run is in progress."""
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps({"running": is_running, "stop_requested": stop_requested}).encode())

    def handle_stop(self):
        """Force-stop the current test run."""
        global stop_requested, current_proc
        if not is_running:
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"stopped": False, "reason": "no run in progress"}).encode())
            return

        stop_requested = True
        # Kill the currently running test subprocess if any
        proc = current_proc
        if proc and proc.poll() is None:
            try:
                proc.kill()
            except OSError:
                pass

        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(json.dumps({"stopped": True}).encode())

    def handle_run(self, params):
        """SSE endpoint — run tests and stream events."""
        global is_running, stop_requested, current_proc

        with run_lock:
            if is_running:
                self.send_response(409)
                self.end_headers()
                self.wfile.write(b"Run already in progress")
                return
            is_running = True
            stop_requested = False
            current_proc = None

        try:
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Connection", "keep-alive")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()

            # Get test list
            tests = get_test_list()
            total = len(tests)

            # Apply filter if provided
            filter_pattern = params.get("filter", [None])[0]
            if filter_pattern:
                regex = re.compile(filter_pattern)
                tests = [t for t in tests if regex.search(t["name"])]

            # Send init event with all tests
            self.send_event("init", {
                "tests": [{"index": t["index"], "name": t["name"], "category": t["category"]} for t in tests],
                "total": len(tests),
                "timestamp": datetime.now(timezone.utc).isoformat(timespec="seconds"),
            })

            passed = 0
            failed = 0

            for i, test in enumerate(tests):
                # Check for stop request between tests
                if stop_requested:
                    # Mark remaining tests as stopped
                    self.send_event("stopped", {
                        "at_index": test["index"],
                        "completed": passed + failed,
                        "total": len(tests),
                    })
                    break

                name = test["name"]
                cmd = test["command"]
                index = test["index"]

                # Send test_start
                self.send_event("test_start", {
                    "index": index,
                    "name": name,
                    "progress": i,
                    "total": len(tests),
                })

                if not cmd:
                    # No command available, skip
                    self.send_event("test_end", {
                        "index": index, "name": name,
                        "status": "skipped", "duration": 0,
                        "checks_passed": 0, "checks_failed": 0,
                    })
                    continue

                # Run test executable directly
                start_time = time.monotonic()
                checks_passed = 0
                checks_failed = 0

                try:
                    proc = subprocess.Popen(
                        cmd,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                        text=True,
                        bufsize=1,  # line buffered
                        cwd=BUILD_DIR,
                    )
                    current_proc = proc

                    for line in iter(proc.stdout.readline, ""):
                        line = line.rstrip("\n\r")

                        # Check for stop mid-test
                        if stop_requested:
                            proc.kill()
                            break

                        # Check for PASS/FAIL
                        m = RE_CHECK.match(line)
                        if m:
                            status = m.group(1).lower()
                            check_name = m.group(2).strip()
                            if status == "pass":
                                checks_passed += 1
                            else:
                                checks_failed += 1
                            self.send_event("check", {
                                "index": index,
                                "status": status,
                                "name": check_name,
                            })
                        else:
                            # Send raw line
                            self.send_event("line", {
                                "index": index,
                                "text": line,
                            })

                    proc.wait(timeout=600)
                    current_proc = None
                    duration = time.monotonic() - start_time

                    if stop_requested:
                        test_passed = False
                    else:
                        test_passed = proc.returncode == 0

                except subprocess.TimeoutExpired:
                    proc.kill()
                    current_proc = None
                    duration = time.monotonic() - start_time
                    test_passed = False
                    self.send_event("line", {
                        "index": index,
                        "text": "*** TIMEOUT (600s) ***",
                    })
                except Exception as e:
                    current_proc = None
                    duration = time.monotonic() - start_time
                    test_passed = False
                    self.send_event("line", {
                        "index": index,
                        "text": f"*** ERROR: {e} ***",
                    })

                if test_passed:
                    passed += 1
                else:
                    failed += 1

                self.send_event("test_end", {
                    "index": index,
                    "name": name,
                    "status": "passed" if test_passed else "failed",
                    "duration": round(duration, 3),
                    "checks_passed": checks_passed,
                    "checks_failed": checks_failed,
                })

            # Send run complete
            self.send_event("done", {
                "total": len(tests),
                "passed": passed,
                "failed": failed,
                "timestamp": datetime.now(timezone.utc).isoformat(timespec="seconds"),
            })

        except (BrokenPipeError, ConnectionResetError):
            pass  # client disconnected
        finally:
            is_running = False
            stop_requested = False
            current_proc = None

    def send_event(self, event_type, data):
        try:
            msg = f"event: {event_type}\ndata: {json.dumps(data)}\n\n"
            self.wfile.write(msg.encode())
            self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            raise


# ── Main ─────────────────────────────────────────────────────────────────

def main():
    global BUILD_DIR, CONFIG, WEB_DIR

    parser = argparse.ArgumentParser(description="Live CTest runner with SSE streaming")
    parser.add_argument("--port", type=int, default=8080, help="Server port (default: 8080)")
    parser.add_argument("--build-dir", default="engine/build", help="CMake build directory")
    parser.add_argument("--config", default="Release", help="Build configuration")
    parser.add_argument("--web-dir", default="engine/web", help="Web root directory")
    args = parser.parse_args()

    BUILD_DIR = args.build_dir
    CONFIG = args.config
    WEB_DIR = args.web_dir

    # Verify build dir exists
    if not Path(BUILD_DIR).exists():
        print(f"Build directory not found: {BUILD_DIR}")
        sys.exit(1)

    server = http.server.HTTPServer(("", args.port), LiveTestHandler)
    print(f"FTD Live Test Server on http://localhost:{args.port}/tests.html")
    print(f"  Build dir: {BUILD_DIR}")
    print(f"  Config: {CONFIG}")
    print(f"  API: /api/list, /api/run, /api/status")
    print(f"  Press Ctrl+C to stop")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        server.shutdown()


if __name__ == "__main__":
    main()

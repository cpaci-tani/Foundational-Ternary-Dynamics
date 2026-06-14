#!/usr/bin/env python3
import os
import json
import re
import sys

def main():
    # Paths to files
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    scale0_path = os.path.join(root_dir, "engine", "config", "scenarios", "scale0.json")
    cpp_files = [
        os.path.join(root_dir, "engine", "src", "scenarios.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "flux.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "light.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "s0_seed.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "s0_field.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "vacuum.cpp"),
        os.path.join(root_dir, "engine", "src", "scenarios", "quantum.cpp")
    ]

    # Verify paths exist
    if not os.path.exists(scale0_path):
        print(f"Error: scale0.json not found at {scale0_path}")
        sys.exit(1)

    for path in cpp_files:
        if not os.path.exists(path):
            print(f"Error: C++ source file not found at {path}")
            sys.exit(1)

    # Load scenario definitions
    with open(scale0_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    scenarios = data.get("scenarios", [])
    if not scenarios:
        print("Error: No scenarios found in scale0.json")
        sys.exit(1)

    print(f"Loaded {len(scenarios)} scenarios from scale0.json.")

    # Read combined C++ contents
    cpp_content = ""
    for path in cpp_files:
        with open(path, "r", encoding="utf-8") as f:
            cpp_content += f.read() + "\n"

    no_op_ids = {
        "flux-dispersion", "flux-stable-vortex", "flux-hydrogen", "flux-rutherford",
        "flux-gravitational-wave", "flux-dark-matter", "flux-baryogenesis",
        "flux-cosmic-web", "flux-black-hole", "flux-damping", "light-prism"
    }

    missing_scenarios = []
    failed_checks = []

    for sc in scenarios:
        sc_id = sc.get("id")
        if not sc_id:
            print("Warning: Scenario with no ID found in scale0.json")
            continue

        # Look for the exact Scenario ID comment
        pattern = rf"//\s*Scenario ID:\s*{re.escape(sc_id)}"
        match = re.search(pattern, cpp_content)
        if not match:
            missing_scenarios.append(sc_id)
            print(f"[FAIL] Scenario '{sc_id}': comment 'Scenario ID: {sc_id}' not found.")
            continue

        # Find the comment block containing this ID
        # Let's extract lines around the match to verify the required fields
        # Find the index in cpp_content and grab the block around it
        start_idx = match.start()
        # Find end of lines for this comment block (e.g. next non-comment line or just first 10 lines)
        block_text = cpp_content[start_idx:start_idx + 1000]

        # Checks
        has_purpose = bool(re.search(r"//\s*Physical Purpose:", block_text))
        has_params = bool(re.search(r"//\s*Initial Condition Parameters:", block_text))
        has_behavior = bool(re.search(r"//\s*Expected Behavio?ur:", block_text))

        sc_failed = False
        reasons = []

        if not has_purpose:
            reasons.append("Missing 'Physical Purpose'")
            sc_failed = True
        if not has_params:
            reasons.append("Missing 'Initial Condition Parameters'")
            sc_failed = True
        if not has_behavior:
            reasons.append("Missing 'Expected Behaviour'")
            sc_failed = True

        if sc_id in no_op_ids:
            has_discrepancy = bool(re.search(r"//\s*Discrepancy:", block_text))
            if not has_discrepancy:
                reasons.append("Missing 'Discrepancy' for no-op scenario")
                sc_failed = True

        if sc_failed:
            failed_checks.append((sc_id, reasons))
            print(f"[FAIL] Scenario '{sc_id}': {', '.join(reasons)}")
        else:
            print(f"[PASS] Scenario '{sc_id}' comments verified.")

    print("\nVerification Summary:")
    print(f"Total scenarios in scale0.json: {len(scenarios)}")
    print(f"Scenarios passing comment verification: {len(scenarios) - len(missing_scenarios) - len(failed_checks)}")
    
    if missing_scenarios:
        print(f"Scenarios completely missing comments ({len(missing_scenarios)}): {', '.join(missing_scenarios)}")
    if failed_checks:
        print(f"Scenarios with incomplete comments ({len(failed_checks)}):")
        for sc_id, reasons in failed_checks:
            print(f"  - {sc_id}: {', '.join(reasons)}")

    if missing_scenarios or failed_checks:
        sys.exit(1)
    else:
        print("All scenario comments successfully verified!")
        sys.exit(0)

if __name__ == "__main__":
    main()

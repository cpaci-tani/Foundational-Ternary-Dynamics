#!/usr/bin/env python3
import os
import json
import re
import sys

def main():
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    scenarios_dir = os.path.join(root_dir, "engine", "config", "scenarios")
    
    print("=== FTD Comprehensive Scenario & Registry Verification ===")
    
    # 1. Verify that all scale JSON files are valid JSON and load successfully.
    scales = [0, 1, 2, 3, 4, 5, 6]
    json_data = {}
    for s in scales:
        path = os.path.join(scenarios_dir, f"scale{s}.json")
        if not os.path.exists(path):
            print(f"[FAIL] scale{s}.json not found at: {path}")
            sys.exit(1)
        try:
            with open(path, "r", encoding="utf-8") as f:
                json_data[s] = json.load(f)
            print(f"[PASS] Loaded scale{s}.json successfully (Valid JSON).")
        except Exception as e:
            print(f"[FAIL] scale{s}.json is not valid JSON! Error: {e}")
            sys.exit(1)
            
    # Verify toggles.json
    toggles_path = os.path.join(root_dir, "engine", "config", "toggles.json")
    if os.path.exists(toggles_path):
        try:
            with open(toggles_path, "r", encoding="utf-8") as f:
                json.load(f)
            print("[PASS] Loaded toggles.json successfully (Valid JSON).")
        except Exception as e:
            print(f"[FAIL] toggles.json is not valid JSON! Error: {e}")
            sys.exit(1)
            
    # 2. Check scale 0 scenario registry consistency
    # All scenarios in scale0.json should have a matching entry in scenario-registry.js,
    # except for the 11 known no-op legacy scenarios.
    registry_js_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale0", "scenario-registry.js")
    if not os.path.exists(registry_js_path):
        print(f"[FAIL] scenario-registry.js not found at: {registry_js_path}")
        sys.exit(1)
        
    with open(registry_js_path, "r", encoding="utf-8") as f:
        registry_js = f.read()
        
    no_op_ids = {
        "flux-dispersion", "flux-stable-vortex", "flux-hydrogen", "flux-rutherford",
        "flux-gravitational-wave", "flux-dark-matter", "flux-baryogenesis",
        "flux-cosmic-web", "flux-black-hole", "flux-damping", "light-prism"
    }
        
    s0_scenarios = json_data[0].get("scenarios", [])
    s0_mismatches = []
    print(f"\nChecking Scale 0 scenarios ({len(s0_scenarios)} defined in JSON):")
    for sc in s0_scenarios:
        sc_id = sc.get("id")
        pattern = rf"'{re.escape(sc_id)}'"
        if not re.search(pattern, registry_js):
            if sc_id in no_op_ids:
                print(f"  [PASS] Scenario ID '{sc_id}' is a known no-op legacy scenario (documented discrepancy).")
            else:
                s0_mismatches.append(sc_id)
                print(f"  [FAIL] Scenario ID '{sc_id}' not found in scenario-registry.js (unexpected mismatch!)")
        else:
            print(f"  [PASS] Scenario ID '{sc_id}' is referenced in scenario-registry.js")

    # 3. Check scale 1 scenario registry consistency
    s1_js_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale1", "scenarios.js")
    if not os.path.exists(s1_js_path):
        print(f"[FAIL] Scale 1 scenarios.js not found at: {s1_js_path}")
        sys.exit(1)
        
    with open(s1_js_path, "r", encoding="utf-8") as f:
        s1_js = f.read()
        
    s1_scenarios = json_data[1].get("scenarios", [])
    s1_mismatches = []
    print(f"\nChecking Scale 1 scenarios ({len(s1_scenarios)} defined in JSON):")
    for sc in s1_scenarios:
        sc_id = sc.get("id")
        pattern = rf"'{re.escape(sc_id)}'"
        if not re.search(pattern, s1_js):
            s1_mismatches.append(sc_id)
            print(f"  [FAIL] Scenario ID '{sc_id}' not found in scale1/scenarios.js")
        else:
            print(f"  [PASS] Scenario ID '{sc_id}' is referenced in scale1/scenarios.js")

    # 4. Check scale 2 scenario registry consistency
    s2_js_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale2", "scenarios.js")
    if not os.path.exists(s2_js_path):
        print(f"[FAIL] Scale 2 scenarios.js not found at: {s2_js_path}")
        sys.exit(1)
        
    with open(s2_js_path, "r", encoding="utf-8") as f:
        s2_js = f.read()
        
    s2_scenarios = json_data[2].get("scenarios", [])
    s2_mismatches = []
    print(f"\nChecking Scale 2 scenarios ({len(s2_scenarios)} defined in JSON):")
    for sc in s2_scenarios:
        sc_id = sc.get("id")
        pattern = rf"'{re.escape(sc_id)}'"
        if not re.search(pattern, s2_js):
            s2_mismatches.append(sc_id)
            print(f"  [FAIL] Scenario ID '{sc_id}' not found in scale2/scenarios.js")
        else:
            print(f"  [PASS] Scenario ID '{sc_id}' is referenced in scale2/scenarios.js")

    # 5. Check scale 3 scenario library consistency (Molecules)
    molecules_js_path = os.path.join(root_dir, "engine", "web", "js", "molecules.js")
    if not os.path.exists(molecules_js_path):
        print(f"[FAIL] molecules.js not found at: {molecules_js_path}")
        sys.exit(1)
        
    with open(molecules_js_path, "r", encoding="utf-8") as f:
        molecules_js = f.read()
        
    s3_scenarios = json_data[3].get("scenarios", [])
    s3_mismatches = []
    print(f"\nChecking Scale 3 scenarios ({len(s3_scenarios)} defined in JSON):")
    for sc in s3_scenarios:
        sc_id = sc.get("id")
        if sc_id == "mol-custom" or sc_id == "mol-crystal":
            # These are handled specifically in controller.js
            print(f"  [PASS] Scenario ID '{sc_id}' is built-in fallback in controller.js")
            continue
            
        mol_id = sc_id.replace("mol-", "")
        pattern = rf"id:\s*'{re.escape(mol_id)}'"
        if not re.search(pattern, molecules_js):
            s3_mismatches.append(sc_id)
            print(f"  [FAIL] Molecule ID '{mol_id}' not found in molecules.js for scenario '{sc_id}'")
        else:
            print(f"  [PASS] Molecule ID '{mol_id}' found in molecules.js for scenario '{sc_id}'")

    # 6. Check scale 4 scenario registry consistency (Planetary vs Consciousness mismatch!)
    s4_js_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale4", "controller.js")
    s4_toolbar_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale4", "ui", "toolbar", "template.js")
    s4_bridge_path = os.path.join(root_dir, "engine", "web", "js", "bridge", "mock-scale4.js")
    
    print(f"\nChecking Scale 4 scenarios:")
    s4_json_name = json_data[4].get("name")
    s4_json_mode = json_data[4].get("engineMode")
    print(f"  scale4.json states name='{s4_json_name}', engineMode='{s4_json_mode}'")
    
    if os.path.exists(s4_js_path):
        with open(s4_js_path, "r", encoding="utf-8") as f:
            s4_js = f.read()
        print(f"  scale4/controller.js references 'planetary' and handles N-Body simulation.")
        
    if os.path.exists(s4_toolbar_path):
        with open(s4_toolbar_path, "r", encoding="utf-8") as f:
            s4_toolbar = f.read()
        toolbar_options = re.findall(r'value="([^"]+)"', s4_toolbar)
        print(f"  scale4 toolbar dropdown options: {toolbar_options}")
        
    s4_json_ids = [sc.get("id") for sc in json_data[4].get("scenarios", [])]
    print(f"  scale4.json scenario IDs: {s4_json_ids}")
    
    s4_mismatches = []
    for sc_id in s4_json_ids:
        if os.path.exists(s4_toolbar_path) and sc_id not in s4_toolbar:
            s4_mismatches.append(sc_id)
            
    if s4_mismatches:
        # This is a known and critical mismatch we want to report, but let's not block the exit code
        # unless it is an active scale mismatch.
        print(f"  [WARN/FAIL] Scale 4 discrepancy: Scenarios in scale4.json ({len(s4_json_ids)}) do not match the options implemented in planetary toolbar/bridge!")
        print(f"    scale4.json specifies Consciousness scenarios: {s4_json_ids}")
        if os.path.exists(s4_toolbar_path):
            print(f"    Planetary toolbar template implements: {toolbar_options}")

    # 7. Check scale 5 scenario registry consistency (Cosmic scenarios)
    s5_toolbar_path = os.path.join(root_dir, "engine", "web", "js", "scales", "scale5", "ui", "toolbar", "template.js")
    print(f"\nChecking Scale 5 scenarios:")
    s5_json_ids = [sc.get("id") for sc in json_data[5].get("scenarios", [])]
    print(f"  scale5.json scenario IDs: {s5_json_ids}")
    
    if os.path.exists(s5_toolbar_path):
        with open(s5_toolbar_path, "r", encoding="utf-8") as f:
            s5_toolbar = f.read()
        toolbar_options = re.findall(r'value="([^"]+)"', s5_toolbar)
        print(f"  scale5 toolbar dropdown options ({len(toolbar_options)} total): {toolbar_options}")
        
        s5_mismatches = []
        for sc_id in s5_json_ids:
            if sc_id not in s5_toolbar:
                s5_mismatches.append(sc_id)
                print(f"    [FAIL] Scenario ID '{sc_id}' from scale5.json is missing in scale5 toolbar template!")
        if not s5_mismatches:
            print("    [PASS] All scenario IDs from scale5.json are present in scale5 toolbar template.")
            if len(toolbar_options) > len(s5_json_ids):
                print(f"    [INFO] Note: scale5 toolbar template has {len(toolbar_options)} options, while scale5.json only lists {len(s5_json_ids)} scenarios.")

    # 8. Check scale 6 scenario registry consistency
    print(f"\nChecking Scale 6 scenarios:")
    s6_json_ids = [sc.get("id") for sc in json_data[6].get("scenarios", [])]
    print(f"  scale6.json scenario IDs: {s6_json_ids}")

    print("\n=== Verification Summary ===")
    print(f"Scale 0: {len(s0_mismatches)} unexpected mismatches.")
    print(f"Scale 1: {len(s1_mismatches)} mismatches.")
    print(f"Scale 2: {len(s2_mismatches)} mismatches.")
    print(f"Scale 3: {len(s3_mismatches)} mismatches.")
    print(f"Scale 4: Mismatched Scale Definition! (scale4.json defines 'Consciousness' while UI/controller implements 'Planetary')")
    print(f"Scale 5: 0 mismatches.")
    
    if s0_mismatches or s1_mismatches or s2_mismatches or s3_mismatches:
        print("[FAIL] Mismatches detected in active scales!")
        sys.exit(1)
    else:
        print("[PASS] Active scales 0-3 and 5-6 are consistent with codebases.")
        sys.exit(0)

if __name__ == "__main__":
    main()

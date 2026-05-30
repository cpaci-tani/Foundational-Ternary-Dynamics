from collections import Counter

ledger_path = r"c:\Users\cpaci\Desktop\ftd\docs\theory\07_assessment\LEDGER.md"

with open(ledger_path, "r", encoding="utf-8") as f:
    lines = f.readlines()

row_ids = []
for i, line in enumerate(lines, 1):
    if line.strip().startswith("|") and "FTD-" in line:
        parts = [p.strip() for p in line.split("|")]
        if len(parts) > 1:
            first_col = parts[1].strip()
            if "FTD-" in first_col:
                row_ids.append((first_col, i))

id_names = [rid[0] for rid in row_ids]
counter = Counter(id_names)
duplicates = {k: v for k, v in counter.items() if v > 1}

print("=== Primary Row IDs Count ===")
print(f"Total rows with FTD: {len(row_ids)}")
print(f"Unique row IDs: {len(counter)}")
print(f"Duplicate row IDs (occurring as primary table row ID): {duplicates}")

if duplicates:
    print("\nFAIL: Found duplicate primary row IDs:")
    for k, v in duplicates.items():
        print(f"  {k}: {v} times")
        # Find lines
        for r, line_num in row_ids:
            if r == k:
                print(f"    Line {line_num}")
else:
    print("\nPASS: No duplicate primary row IDs found in LEDGER.md.")

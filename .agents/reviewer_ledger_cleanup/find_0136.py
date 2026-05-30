import re

ledger_path = r"c:\Users\cpaci\Desktop\ftd\docs\theory\07_assessment\LEDGER.md"

with open(ledger_path, "r", encoding="utf-8") as f:
    lines = f.readlines()

for i, line in enumerate(lines, 1):
    if line.strip().startswith("|") and "FTD-0136" in line:
        parts = [p.strip() for p in line.split("|")]
        if len(parts) > 1 and "FTD-0136" in parts[1]:
            print(f"Line {i}: {line[:120]}...")

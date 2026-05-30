## 2026-05-30T05:22:53Z
You are the Report Writer for the FTD project.
Your objective is to:
1. Ensure the gap report `c:\Users\cpaci\Desktop\ftd\REPORT_GAP_ANALYSIS.md` accurately summarizes the epistemic drift and tag violations identified in the previous handoff.
2. Update the canonical ledger `c:\Users\cpaci\Desktop\ftd\docs\theory\07_assessment\TRACKER_OPEN_ITEMS.md`. You must append a new section `## §10 Epistemic Integrity Gaps` to the end of the file containing specific action items for the identified tag inflations from the report (lepton mass ratios, alpha precision formulas, SU2 decay rates).
CRITICAL ENCODING RULE: You MUST NOT use PowerShell, `Out-File`, `Set-Content`, `replace_file_content` or `multi_replace_file_content` to edit the ledger, because that corrupts the UTF-8 encoding.
Instead, use the `write_to_file` tool to create a short python script at `c:\Users\cpaci\Desktop\ftd\.agents\append.py` that opens the ledger with `encoding='utf-8'` and appends the `## §10...` section as a string. Then use `run_command` to run `python .agents\append.py`.
3. CRITICAL FRAMEWORK RULE: You MUST write your prompt to `original_prompt.md` in your `.agents/` working directory. You MUST create `progress.md` and `handoff.md` in your working directory. Ensure all files are generated correctly, or we will fail the audit.
4. When you are finished, send me a message.

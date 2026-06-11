import os

filepath = ".gitignore"
with open(filepath, "r", encoding="utf-8") as f:
    lines = f.readlines()

seen = set()
cleaned_lines = []
for line in lines:
    stripped = line.strip()
    if not stripped:
        # Keep empty lines but avoid more than one consecutive empty line
        if cleaned_lines and cleaned_lines[-1] != "\n":
            cleaned_lines.append("\n")
        continue
    
    if stripped.startswith("#"):
        # We can keep comments, but let's avoid duplicate comments
        if stripped not in seen:
            seen.add(stripped)
            cleaned_lines.append(line)
    else:
        # Regular rule
        if stripped not in seen:
            seen.add(stripped)
            cleaned_lines.append(line)

# Now apply the specific request "add gitignore to gitignore"
# Ensure `.gitignore` is ignored and NOT explicitly tracked
final_lines = []
for line in cleaned_lines:
    if line.strip() == "!.gitignore":
        continue  # Remove the explicit tracking
    final_lines.append(line)

if ".gitignore\n" not in final_lines and ".gitignore" not in [x.strip() for x in final_lines]:
    final_lines.append(".gitignore\n")

with open(filepath, "w", encoding="utf-8") as f:
    f.writelines(final_lines)

print("Cleaned .gitignore")

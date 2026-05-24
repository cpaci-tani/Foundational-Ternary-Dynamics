"""Check for orphan labels (defined but never referenced) in the G* paper."""
import re

with open(r'C:\Users\cpaci\Desktop\ftd\docs\papers\PAPER_GSTAR_INTRODUCTION.tex') as f:
    text = f.read()

labels = set(re.findall(r'\\label\{([^}]+)\}', text))
ref_iter = re.findall(r'\\ref\{([^}]+)\}|\\eqref\{([^}]+)\}', text)
ref_set = set()
for r in ref_iter:
    for x in r:
        if x:
            ref_set.add(x)

undefined = ref_set - labels
orphans = labels - ref_set

print(f'Labels defined: {len(labels)}')
print(f'Labels referenced: {len(ref_set)}')
print()
print(f'UNDEFINED references ({len(undefined)}):')
if undefined:
    for r in sorted(undefined):
        print(f'  MISS {r}')
else:
    print('  (none -- every reference resolves)')
print()
print(f'Orphan labels (defined but never \\ref-d): {len(orphans)}')
# Filter out section labels — those are often used for `\S\ref{}` form
sec_orphans = [o for o in orphans if o.startswith('sec:')]
non_sec_orphans = [o for o in orphans if not o.startswith('sec:')]
print(f'  Section labels not \\ref-d: {len(sec_orphans)} (often referenced as \\S\\ref{{...}})')
print(f'  Non-section orphans ({len(non_sec_orphans)}):')
for o in sorted(non_sec_orphans):
    print(f'    {o}')

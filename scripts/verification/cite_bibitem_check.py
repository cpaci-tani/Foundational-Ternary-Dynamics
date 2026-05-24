"""Cross-check every \\cite{} against \\bibitem{} in the G* paper."""
import re

with open(r'C:\Users\cpaci\Desktop\ftd\docs\papers\PAPER_GSTAR_INTRODUCTION.tex') as f:
    text = f.read()

# bibitem keys
bibitems = set(re.findall(r'\\bibitem\{([^}]+)\}', text))

# cite keys: handles \cite{k1,k2} and \cite[opt]{k} and \cite[opt1,opt2]{k}
cite_matches = re.findall(r'\\cite(?:\[[^\]]*\])?\{([^}]+)\}', text)
citations = set()
for m in cite_matches:
    for k in m.split(','):
        citations.add(k.strip())

print(f'Bibitems found: {len(bibitems)}')
print(f'Unique cite keys: {len(citations)}')
print()

missing = citations - bibitems
print('Missing bibitems (cited but no bibitem):')
if missing:
    for k in sorted(missing):
        print(f'  MISS {k}')
else:
    print('  (none -- all cite keys resolve)')
print()

unused = bibitems - citations
print('Unused bibitems (bibitem but never cited):')
if unused:
    for k in sorted(unused):
        print(f'  UNUSED {k}')
else:
    print('  (none -- every bibitem is cited)')

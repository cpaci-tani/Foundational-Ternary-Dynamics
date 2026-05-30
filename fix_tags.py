import re

path = 'c:/Users/cpaci/Desktop/ftd/docs/theory/03_derivations/DERIV_LATTICE_SU2_WEAK.md'
with open(path, 'r', encoding='utf-8') as f:
    lines = f.read().split('\n')

for i in range(len(lines)):
    if '## 6.3 What This Means for Decay Rates [THEOREM]' in lines[i]:
        lines[i] = lines[i].replace('[THEOREM]', '[IMPOSED]')
    if '- ~50 decay rates = [THEOREM]' in lines[i]:
        lines[i] = lines[i].replace('[THEOREM]', '[IMPOSED]')
        if 'all numerical inputs now FTD-derived' in lines[i]:
            lines[i] = lines[i].replace('(all numerical inputs now FTD-derived)', '(all numerical inputs now FTD-derived, substitution identity into standard QFT kinematics)')
    
    if 376 <= i + 1 <= 420:
        lines[i] = lines[i].replace('**THEOREM**', '**IMPOSED**')

with open(path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines))

print("Done fixing tags in SU2_WEAK.")

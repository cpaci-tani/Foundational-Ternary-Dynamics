#!/usr/bin/env python3
"""Item 0.21: Verify constants.h re-exports match ontic.h (no stale overrides)"""
import re

with open('engine/include/ftd/constants.h', 'r', encoding='utf-8') as f:
    constants_h = f.read()

with open('engine/include/ftd/ontic.h', 'r', encoding='utf-8') as f:
    ontic_h = f.read()

# Extract all 'using ontic::NAME' from constants.h
using_pattern = re.compile(r'using ontic::(\w+);')
using_names = using_pattern.findall(constants_h)

# Extract all declared names in ontic.h
decl_pattern = re.compile(r'inline\s+constexpr\s+(?:double|int|bool)\s+(\w+)')
ontic_names = set(decl_pattern.findall(ontic_h))
# Add function names
func_pattern = re.compile(r'inline\s+(?:double|int)\s+(\w+)\s*\(')
ontic_names.update(func_pattern.findall(ontic_h))

print(f'constants.h re-exports: {len(using_names)} names')
print(f'ontic.h declared names: {len(ontic_names)} names')

# Stale re-exports
stale = [n for n in using_names if n not in ontic_names]
if stale:
    print(f'\nSTALE RE-EXPORTS (FAIL): {stale}')
else:
    print(f'\nNo stale re-exports. All {len(using_names)} names exist in ontic.h. PASS')

# Not re-exported
skip = {'ontic_audit', 'check', 'check_close'}
not_exported = sorted(n for n in ontic_names if n not in using_names and n not in skip)
if not_exported:
    print(f'\nontic.h names NOT re-exported ({len(not_exported)}):')
    for n in not_exported:
        print(f'  - {n}')
    # Check if these are neutrino masses or other recent additions
    print('\n(These may be intentionally not re-exported if engine does not use them)')

# Check for shadowed names in engine-specific section
engine_section = constants_h.split('// Engine-specific constants')[1] if '// Engine-specific constants' in constants_h else ''
shadow_pattern = re.compile(r'inline\s+constexpr\s+(?:double|int)\s+(\w+)\s*=')
engine_names = shadow_pattern.findall(engine_section)
shadows = [n for n in engine_names if n in ontic_names]
if shadows:
    print(f'\nWARNING - constants.h shadows ontic names: {shadows}')
else:
    print(f'\nNo shadowed names. PASS')
    print(f'Engine-specific constants: {engine_names}')

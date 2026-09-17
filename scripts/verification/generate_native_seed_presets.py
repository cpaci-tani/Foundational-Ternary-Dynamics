"""Export constructor-authored schemas; never duplicate C++ defaults in JavaScript.

The compiled exporter resolves actual defaults. Source expressions and binding
locations are annotations, never a second evaluator for those expressions.
"""
from pathlib import Path
import argparse
import json
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'engine/web/js/seeding/generated/native-seeds.json'
IDS = OUTPUT.with_name('native-seed-ids.js')


def arguments(text, start):
    depth = 0
    quote = False
    escape = False
    parts = []
    begin = start
    for i in range(start, len(text)):
        c = text[i]
        if quote:
            if escape: escape = False
            elif c == '\\': escape = True
            elif c == '"': quote = False
        elif c == '"': quote = True
        elif c in '([{': depth += 1
        elif c in ')]}':
            if c == ')' and depth == 0:
                parts.append(text[begin:i].strip())
                return parts
            depth -= 1
        elif c == ',' and depth == 0:
            parts.append(text[begin:i].strip())
            begin = i + 1
    raise ValueError('Unclosed seed binding')


def binding_index():
    index = {}
    paths = sorted((ROOT / 'engine/src/scenarios').glob('*.cpp')) + [ROOT / 'engine/src/scenarios.cpp']
    for path in paths:
        text = path.read_text(encoding='utf8')
        # Lexical branch spans prevent a shared helper/protocol from accidentally
        # inheriting the last scenario above it. All names in an OR branch share
        # the same binding; the old nearest-name lookup lost every earlier alias.
        masked = re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"',
                        lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)
        branches = []
        for branch in re.finditer(r'\bif\s*\(([^{}]*?)\)\s*\{', text):
            names = re.findall(r'name\s*==\s*"([a-z0-9-]+)"', branch[1])
            if not names: continue
            opening = branch.end()-1
            depth, end = 1, opening+1
            while end < len(masked) and depth:
                depth += (masked[end] == '{') - (masked[end] == '}')
                end += 1
            branches.append((opening, end, tuple(names)))
        variables_by_scope = {}
        for call in re.finditer(r'seed::(?:real|integer|choice|boolean|uint32)\(', text):
            args = arguments(text, call.end())
            if len(args) < 2 or not re.fullmatch(r'"[\w.]+"', args[0]): continue
            key = args[0][1:-1]
            enclosing = [b for b in branches if b[0] < call.start() < b[1]]
            scenarios = enclosing[-1][2] if enclosing else ('*',)
            scope = enclosing[-1][0] if enclosing else -1
            variables = variables_by_scope.setdefault(scope, {})
            line = text.count('\n', 0, call.start()) + 1
            dependencies = sorted({variables[token] for token in re.findall(r'\b\w+\b', args[1]) if token in variables})
            for scenario in scenarios:
                index.setdefault((scenario, key), []).append(dict(
                    source=str(path.relative_to(ROOT)).replace('\\','/'), line=line,
                    defaultExpression=args[1], dependencies=dependencies))
            declaration = re.search(r'\b(\w+)\s*=\s*$', text[max(0,call.start()-100):call.start()])
            if declaration: variables[declaration.group(1)] = key
    return index


def annotate(data):
    bindings = binding_index()
    for schema in data['presets'].values():
        schema['schemaIdentity'] = 'native-seed-2'
        for p in schema['properties']:
            candidates = bindings.get((schema['scenarioId'], p['key']), bindings.get(('*',p['key']), []))
            if candidates: p.update(candidates[0])
            else:
                # Registry-driven protocol and RNG inputs are emitted by the
                # shared dispatcher, not individual constructor branches.
                if not p['key'].startswith('protocol.'):
                    raise ValueError(f"Missing concrete source binding: {schema['scenarioId']} / {p['key']}")
                source = ROOT / 'engine/src/scenarios.cpp'
                lines = source.read_text(encoding='utf8').splitlines()
                p.update(source='engine/src/scenarios.cpp',
                         line=next(i+1 for i,s in enumerate(lines) if 'seed::boolean(key,' in s),
                         defaultExpression='registered constructor profile', dependencies=[])
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--binary', type=Path, default=ROOT / 'engine/build/Release/test_scenario_seed_defaults.exe')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='ftd-seed-schema-') as temp:
        target = Path(temp) / 'native.json'
        result = subprocess.run([str(args.binary), '--describe', str(target)],
            env={**os.environ, 'FTD_FORCE_CPU':'1'}, capture_output=True, text=True)
        if result.returncode: raise SystemExit(result.stderr[-4000:])
        data = annotate(json.loads(target.read_text(encoding='utf8')))
    serialized = json.dumps(data, ensure_ascii=False, separators=(',', ':')) + '\n'
    names = [key.rsplit('@',1)[0] for key in data['presets'] if key.endswith('@33')]
    ids = '// Generated from the compiled native seed registry.\nexport const NATIVE_SEED_IDS = Object.freeze(' + json.dumps(names) + ');\n'
    for path, content in [(OUTPUT, serialized), (IDS, ids)]:
        if args.check:
            if not path.exists() or path.read_text(encoding='utf8') != content: raise SystemExit(f'Stale native seed schema: {path}')
        else: path.write_text(content, encoding='utf8', newline='\n')
    print(f'{len(names)} native scenarios; {len(data["presets"])} resolved schemas match constructor defaults.')


if __name__ == '__main__': main()

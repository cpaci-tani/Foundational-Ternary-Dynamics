"""Bind every registered preparation schema to its constructor and effect tests.

--check rejects registry/schema/effect drift. This is software coverage metadata,
not physical qualification and not a substitute for executing the named gates.
"""
import argparse
from collections import defaultdict
from hashlib import sha256
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from phi_v2_lattice import web_scenarios as W

OUTPUT = ROOT / 'engine/config/web_seeding_coverage.json'
NATIVE = 'engine/web/js/seeding/generated/native-seeds.json'
FINITE = 'engine/web/js/seeding/generated/finite-seeds.json'

def read(path):
    return json.loads((ROOT / path).read_text(encoding='utf8'))

def inventory():
    code = """
    import {SCALE0_SCENARIOS} from './engine/web/js/scales/scale0/scenario-registry.js';
    import {seedCoverage} from './engine/web/js/seeding/recipe.js';
    process.stdout.write(JSON.stringify(seedCoverage(SCALE0_SCENARIOS)));
    """
    result = subprocess.run(['node', '--input-type=module', '-e', code], cwd=ROOT,
                            check=True, capture_output=True, encoding='utf-8')
    rows = json.loads(result.stdout)
    native, finite = read(NATIVE)['presets'], read(FINITE)['schemas']
    effects = read('engine/config/native_seed_effects.json')
    effect_map = {(e['scenarioId'], e['key']): e for e in effects['effects']}
    finite_tests = read('engine/config/finite_seed_effects.json')
    tests = defaultdict(list)
    for row in finite_tests['rows']:
        tests[(row['category'],row['kind'],row['key'])].append(row['test'])
    categories = defaultdict(list)
    property_bindings=[]
    for row in rows:
        schemas = {k:v for k,v in native.items() if v['scenarioId'] == row['scenarioId']}
        if not schemas: raise ValueError('Missing native schema: ' + row['scenarioId'])
        row.update(schemaFile=NATIVE, schemaKeys=list(schemas), defaultTest='scenario_seed_defaults',
                   effectTest='scenario_seed_effects')
        categories[row['category']].append(row['scenarioId'])
        for schema_key,schema in schemas.items():
            for prop in schema['properties']:
                effect=effect_map.get((row['scenarioId'],prop['key']))
                if not effect or not effect['effectVerified']:
                    raise ValueError(f'Missing verified effect: {schema_key}:{prop["key"]}')
                property_bindings.append(dict(schema=schema_key,key=prop['key'],binding=prop['binding'],
                    source=prop['source'],test='scenario_seed_effects',testedSize=effects['size']))
    for scenario in W.scenarios():
        sid='record-'+scenario.id
        row=dict(scenarioId=sid,category='Finite records · '+scenario.family,capability='record-components',
                 parameterAudit='nine-kind-record-editors',presetSizes=list(scenario.sizes),source=scenario.source,
                 schemaFile=FINITE,schemaKeys=[f'{sid}@{n}' for n in scenario.sizes],
                 defaultTest='test_v2_default_recipe_is_byte_identical_to_the_frozen_checkpoint',effectTest=finite_tests['testModule'])
        rows.append(row);categories[row['category']].append(sid)
        for schema_key in row['schemaKeys']:
            schema=finite[schema_key]
            for prop in schema['properties']:
                path=prop['path']
                if len(path)==1: identity=('recipe',None,path[0])
                else:
                    kind=schema['recipe']['components'][path[1]]['kind']
                    if path[2]=='enabled':identity=('activation',kind,'enabled')
                    elif path[2]=='region':identity=('region',None,path[3])
                    else:identity=('collection' if len(path)>4 else 'property',kind,path[3])
                refs=tests.get(identity)
                if not refs:raise ValueError(f'Missing finite effect: {schema_key}:{identity}')
                property_bindings.append(dict(schema=schema_key,key=prop['key'],binding=prop['binding'],
                    source='scripts/phi_v2_lattice/web_seed_presets.py',tests=refs))
    if set(effect_map) != {(r['scenarioId'],p['key']) for r in rows if r['capability']=='native-preparation'
                            for p in native[f"{r['scenarioId']}@17"]['properties']}:
        raise ValueError('Native effect inventory has orphaned bindings')
    tracked_sources=list((ROOT/'engine/src/scenarios').glob('*.cpp'))+[
        ROOT/'engine/src/scenarios/_helpers.h',ROOT/'engine/src/scenarios.cpp',
        ROOT/'engine/include/ftd/scenario_seed.h',ROOT/'scripts/phi_v2_lattice/web_seed_presets.py']
    hashes={str(p.relative_to(ROOT)).replace('\\','/'):sha256(p.read_text(encoding='utf8').encode('utf8')).hexdigest() for p in tracked_sources}
    return dict(schemaVersion=2,status='constructor-and-effect-bindings',
        boundaries=['143 native constructor defaults checked at L=17,32,33 against pre-edit receipts; effects tested at L=17.',
                    'All registered finite preparation/size variants compile through separate editable equivalents; frozen builders and evidence stay unchanged.',
                    'Direct and worker WASM use detached candidates. Native WebSocket requires advertised SeedRecipeV2 and commits with a source-epoch precondition.',
                    'Finite custom recipes and dynamic descriptions require the local preparation service.',
                    'Numeric write floors, lattice stencils, codec alphabets and fixed tick-law constants are read-only context.',
                    'Explicit edits are custom preparations; software coverage grants no transport or recovery claim.'],
        counts=dict(scenarios=len(rows),categories=len(categories),nativeDefaultReceipts=len(native),
                    finiteDefaultCheckpoints=sum(len(s.sizes) for s in W.scenarios()),
                    nativeEffectBindings=len(effect_map),describedPropertyInstances=len(property_bindings)),
        sourceSHA256=hashes,
        categories=[dict(name=k,scenarios=v,base=('nine finite component kinds' if k.startswith('Finite records') else 'ordered native constructor union')) for k,v in categories.items()],
        scenarios=rows,propertyBindings=property_bindings)

def serialized():
    data=inventory()
    bindings=data.pop('propertyBindings')
    head=json.dumps(data,ensure_ascii=False,indent=2)
    return head[:-2]+',\n  "propertyBindings": [\n'+',\n'.join('    '+json.dumps(row,ensure_ascii=False) for row in bindings)+'\n  ]\n}\n'

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    text=serialized()
    if args.check:
        if not OUTPUT.is_file() or OUTPUT.read_text(encoding='utf8')!=text:raise SystemExit('Seeding coverage is stale; regenerate with this script.')
        print('Every registered scenario and descriptor has a constructor and effect-test binding.')
    else:OUTPUT.write_text(text,encoding='utf8',newline='\n');print(OUTPUT)

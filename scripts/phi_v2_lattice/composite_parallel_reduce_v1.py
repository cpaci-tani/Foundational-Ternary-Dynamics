"""Versioned retained-range reader; the scientific law and V1 stay frozen.

Only wire-decoded builtin integers and an independently admitted immutable
BEGIN enter the private fast path. No validator is replaced or cached.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import types

ROOT = Path(__file__).resolve().parents[2]
FROZEN_DRIVER = ROOT / 'scripts/phi_v2_lattice/experiments/certify_composite_interaction_v1.py'
FROZEN_SHA256 = '3ff2d0ad4ecb45cddba0034ad7d4b743d7d6fbf841211b31ae68651b1db0b959'
REQUEST_CAP = 16 << 10
RESULT_CAP = 256 << 10
PARTIAL_CAP = 1 << 20
LOG_CAP = 64 << 10
_C = None


def frozen_driver():
    """Load the same owned source buffer that was checked, without evolution."""
    global _C
    if _C is None:
        with FROZEN_DRIVER.open('rb') as source:
            raw = source.read((1 << 20) + 1)
        if len(raw) > 1 << 20 or hashlib.sha256(raw).hexdigest() != FROZEN_SHA256:
            raise ValueError('frozen composite decoder source changed')
        module = types.ModuleType('_ftd_composite_parallel_frozen_v1')
        module.__file__ = str(FROZEN_DRIVER)
        sys.modules[module.__name__] = module
        exec(compile(raw, str(FROZEN_DRIVER), 'exec'), module.__dict__)
        _C = module
    return _C


def _validated_payload_bytes(state):
    """Encode fields whose complete state validation has already succeeded."""
    out = bytearray()
    for carrier in state.carriers:
        word = (carrier.site | carrier.slot << 18 | carrier.direction << 19
                | carrier.credit << 22 | carrier.attempted << 23)
        out += word.to_bytes(3, 'little')
    out.append(len(state.edges))
    for edge in state.edges:
        out += (edge.owner | edge.axis << 18 | (edge.q == 1) << 20).to_bytes(3, 'little')
    return bytes(out)


def _head(L, site, axis):
    stride = (L * L, L, 1)[axis]
    coordinate = (site // stride) % L
    return site + stride if coordinate < L - 1 else site - (L - 1) * stride


def _payload(r, begin_state, offset, S):
    """Complete validation once for fresh decoded fields and admitted context.

    Bit extraction supplies exact builtin integer types and one-bit domains.
    The admitted BEGIN supplies fixed valid identity/context fields. Every
    remaining S._normalized condition is checked below, including Gauss.
    """
    start = r.pos
    L = begin_state.L
    site_count = L ** 3
    carriers = []
    previous = (-1, -1)
    plus = credits = 0
    residual = {}
    for _ in range(4):
        word = int.from_bytes(r.take(3), 'little')
        site, slot = word & 0x3ffff, (word >> 18) & 1
        direction, credit, attempted = (word >> 19) & 7, (word >> 22) & 1, (word >> 23) & 1
        if site >= site_count or not 1 <= direction <= 6 or (site, slot) <= previous:
            raise ValueError('compact carrier domain/order')
        previous = site, slot
        plus += slot == 0
        credits += credit
        residual[site] = residual.get(site, 0) - (1 if slot == 0 else -1)
        carriers.append(S.Carrier(site, slot, direction, credit, attempted))
    count = r.one('B')
    if count > 4 or plus != 2 or count + credits != 4:
        raise ValueError('compact Q4 sector/edge count')
    edges = []
    previous = (-1, -1)
    for _ in range(count):
        word = int.from_bytes(r.take(3), 'little')
        owner, axis, charge = word & 0x3ffff, (word >> 18) & 3, 2 * ((word >> 20) & 1) - 1
        if word >> 21 or axis == 3 or owner >= site_count or (owner, axis) <= previous:
            raise ValueError('compact edge domain/order/reserved bits')
        previous = owner, axis
        edges.append(S.Edge(owner, axis, charge))
        head = _head(L, owner, axis)
        residual[owner] = residual.get(owner, 0) + charge
        residual[head] = residual.get(head, 0) - charge
    if any(residual.values()):
        raise ValueError('compact Gauss incidence mismatch')
    state = S.SparseQ4State(L, begin_state.microtick + offset, tuple(carriers), tuple(edges),
                          begin_state.origin_code, begin_state.charge_frame)
    if _validated_payload_bytes(state) != r.data[start:r.pos].tobytes():
        raise ValueError('noncanonical compact payload')
    return state


def _lift(r, state, I):
    """One complete lift validation, then exact registered no-wrap support.

    The state was validated above; struct decoding produces exact immutable
    triples. This retains validate_lift's modulo, charge/flux and every
    shared endpoint check, lift_wire's coordinate bound/re-encoding, and
    check_no_wrap's 2..62 bound including the positive edge endpoints.
    """
    start = r.pos
    carriers = tuple(r.unpack('hhh') for _ in range(4))
    edges = tuple(r.unpack('hhh') for _ in state.edges)
    points = carriers + edges
    if any(v < -512 or v > 512 for point in points for v in point):
        raise ValueError('lift coordinate outside wire domain')
    L = state.L
    def site(point):
        return ((point[0] % L) * L + point[1] % L) * L + point[2] % L
    if any(site(point) != carrier.site for point, carrier in zip(carriers, state.carriers)):
        raise ValueError('carrier lift does not match state')
    if any(site(point) != edge.owner for point, edge in zip(edges, state.edges)):
        raise ValueError('edge lift does not match state')
    charge = tuple(sum((1 if c.slot == 0 else -1) * point[a] for c, point in zip(state.carriers, carriers)) for a in range(3))
    flux = tuple(-sum(e.q for e in state.edges if e.axis == a) for a in range(3))
    if charge != flux:
        raise ValueError('unwrapped charge-dipole incidence mismatch')
    assigned = {}
    for carrier, point in zip(state.carriers, carriers):
        if carrier.site in assigned and assigned[carrier.site] != point:
            raise ValueError('inconsistent colocated lift')
        assigned[carrier.site] = point
    heads = []
    for edge, point in zip(state.edges, edges):
        endpoint = tuple(value + (axis == edge.axis) for axis, value in enumerate(point))
        heads.append(endpoint)
        for index, position in ((edge.owner, point), (_head(L, edge.owner, edge.axis), endpoint)):
            if index in assigned and assigned[index] != position:
                raise ValueError('inconsistent edge endpoint lift')
            assigned[index] = position
    if b''.join(struct.pack('<hhh', *point) for point in points) != r.data[start:r.pos].tobytes():
        raise ValueError('noncanonical lift wire')
    if L != 64 or any(value < 2 or value > 62 for point in (*points, *heads) for value in point):
        raise ValueError('registered no-wrap support bound failed')
    return I.IntegerLift(carriers, edges)


def _read_tick(r, begin_state, C, S, I):
    offset = C.integer(r.one('H'), 'Tick offset', 1, C.H)
    state = _payload(r, begin_state, offset, S)
    lift = _lift(r, state, I)
    events = C.decode_events(r.blob(112, 'H'))
    chain = r.take(32)
    flags = C.integer(r.one('B'), 'Tick flags', 0, 15)
    return offset, state, lift, events, chain, flags


def _stored_progress(raw, case, C, S, I):
    # Same V1 stored-prefix algorithm and diagnostic checks; only the private
    # Tick read fuses validation of its freshly decoded immutable records.
    case = {key: list(value) if isinstance(value, list) else value for key, value in case.items()}
    r = C.Reader(raw)
    representative, previous, completed = r.unpack('IHH')
    if (representative != case['begin'].representative or previous != len(case['chain'])
            or not 1 <= completed - previous <= 19 or completed > C.H):
        raise ValueError('stored PROGRESS inventory')
    newly_first = {}
    for expected in range(previous + 1, completed + 1):
        offset, state, lift, events, chain, flags = _read_tick(r, case['begin'].state, C, S, I)
        if offset != expected:
            raise ValueError('stored Tick offset gap')
        for bit in range(4):
            if flags & (1 << bit):
                case['counts'][bit] += 1
                if not case['first'][bit]:
                    case['first'][bit] = offset
                    newly_first[bit] = offset
        case['history'].append(state)
        case['lifts'].append(lift)
        case['chain'].append(chain.hex())
        case['last_chain'] = chain
    mask = r.one('B')
    if mask != sum(1 << bit for bit in newly_first):
        raise ValueError('stored first-event mask mismatch')
    for bit, offset in sorted(newly_first.items()):
        if r.one('H') != offset:
            raise ValueError('stored first-event offset mismatch')
        if bit == 0:
            count = C.integer(r.one('H'), 'first contact key count', 1, 2376)
            rows = []
            for _ in range(count):
                kind, site, column = r.unpack('BIB')
                C.integer(kind, 'contact key kind', 0, len(C.KEY_NAMES) - 1)
                rows.append((C.KEY_NAMES[kind], site, column))
            C.keys_wire(rows)
        elif bit == 2:
            C.decode_events(r.blob(112, 'H'))
        elif bit == 3:
            reason = C.integer(r.one('B'), 'conflict reason', 0, 1)
            keys = []
            for high in (1, 2):
                count = C.integer(r.one('B'), 'conflict count', 0, 2)
                rows = tuple(r.unpack('IB') for _ in range(count))
                if (rows != tuple(sorted(set(rows)))
                        or any(not 0 <= site < 64 ** 3 or not 0 <= column <= high for site, column in rows)):
                    raise ValueError('conflict key domain/order')
                keys.append(rows)
            if (reason == 0 and any(keys)) or (reason == 1 and not any(keys)):
                raise ValueError('conflict reason/key mismatch')
    case['controls'] = C._read_controls(r, case['history'][-1])
    r.done()
    case['blocks'] += 1
    if case['blocks'] > 16:
        raise ValueError('stored progress block count')
    return case


def reduce_range(path, expected_header, admitted_map, groups, complete, consume, check=lambda: None):
    """V1 range-prefix semantics with private fresh-record validation.

    admitted_map and groups must have passed the parent all-original alias
    admission. Each BEGIN is decoded/validated by the frozen map reader and
    compared byte-for-byte before any private Tick read is permitted.
    """
    C = frozen_driver()
    S, I = C._oracle()
    group_pos, case, terminal = 0, None, None
    completed_groups = durable_ticks = 0
    terminals = hashlib.sha256()
    with C.RangeStream(path, expected_header) as stream:
        while True:
            check()
            frame = stream.next_frame()
            if frame is None:
                break
            try:
                kind, raw = frame['kind'], frame['body']
                if kind == 1:
                    if case is not None or group_pos >= len(groups):
                        raise ValueError('unexpected owned BEGIN')
                    begin = admitted_map.begin(groups[group_pos])
                    if raw != C.begin_wire(begin):
                        raise ValueError('BEGIN differs from exact admitted execution group')
                    case = C._new_case(begin)
                    group_pos += 1
                elif kind == 2:
                    if case is None:
                        raise ValueError('PROGRESS without BEGIN')
                    try:
                        following = _stored_progress(raw, case, C, S, I)
                    except (ValueError, UnicodeError, struct.error):
                        # Rejected input takes the frozen diagnostic path, so
                        # even the authenticated failure-prefix receipt retains
                        # V1's exact rejection text. No law is reexecuted.
                        C._stored_progress(raw, case)
                        raise RuntimeError('private reader rejected a V1-admissible PROGRESS')
                    durable_ticks += len(following['chain']) - len(case['chain'])
                    case = following
                elif kind == 3:
                    if case is None:
                        raise ValueError('CASE_FINAL without BEGIN')
                    result = C._stored_final(raw, case)
                    terminals.update(frame['digest'])
                    if result['status'] == 0:
                        group = case['begin'].group
                        if complete[group]:
                            raise ValueError('duplicate completed execution group')
                        consume(case['begin'], result)
                        complete[group] = 1
                        completed_groups += 1
                    case = None
                else:
                    row = C.decode_range_final(raw)
                    if case is not None and row['status'] == 0:
                        raise ValueError('complete range terminal inside an unfinished case')
                    local = sum(bool(complete[admitted_map.original(index)['group']])
                                for index in range(expected_header['start'], expected_header['stop'])
                                if admitted_map.original(index)['owner_range'] == expected_header['range'])
                    expected = {'range': expected_header['range'], 'start': expected_header['start'], 'stop': expected_header['stop'],
                                'owned_groups': len(groups), 'completed_groups': completed_groups, 'verified_durable_ticks': durable_ticks,
                                'locally_covered_originals': local, 'unresolved_aliases': expected_header['stop'] - expected_header['start'] - local,
                                'case_terminal_chain': terminals.hexdigest(), 'group_map_sha256': expected_header['group_map_sha256']}
                    if any(row[name] != value for name, value in expected.items()):
                        raise ValueError('RANGE_FINAL inventory differs from authenticated records/map')
                    if row['status'] == 0 and (completed_groups != len(groups) or group_pos != len(groups)
                            or row['reported_executed_ticks'] != 2 * durable_ticks or durable_ticks != C.H * len(groups)):
                        raise ValueError('COMPLETE range lacks all owned real ticks')
                    terminal = row
                stream.commit(frame)
            except (ValueError, UnicodeError, struct.error) as error:
                stream.reject(error)
                break
        report = stream.finish(check)
    report.update(range=expected_header['range'], terminal=terminal, owned_groups=len(groups), completed_groups=completed_groups,
                  verified_durable_ticks=durable_ticks, begun_groups=group_pos,
                  status='COMPLETE' if terminal and terminal['status'] == 0 and not report['tail_error'] and not report['tail_bytes'] else 'INTERRUPTED')
    return report


REQUEST_KEYS = {'schema', 'expected_header', 'map', 'descriptor', 'range_stream', 'native_stdout',
                'native_stderr', 'native_exit_code', 'output', 'source_sha256'}
RESULT_KEYS = {'schema', 'request_sha256', 'module_sha256', 'frozen_decoder_sha256', 'range',
               'status', 'range_receipt', 'completed_groups', 'partial', 'error'}
REPORT_KEYS = {'completed_execution_groups', 'weighted_originals', 'unique_group_counts',
               'original_weighted_counts', 'complete_witness_inventory'}
_FIRST = tuple('first_' + name for name in ('contact', 'effect', 'event_difference', 'counterfactual_invalid'))
_TOTAL = tuple('total_' + name for name in ('contact', 'effect', 'event_difference', 'counterfactual_invalid'))
_BOOL = ('escape_applicable', 'escape_infinite_lift', 'escape_periodic_torus', 'capture_case')
_FIXED = (*_FIRST, *_TOTAL, *_BOOL, 'escape_reason', 'unordered_drifts', 'final_graph_component_accounts', 'classification')
_CAPTURE = ('capture_period', 'capture_translation', 'capture_maximum_diameter', 'capture_kind')


def _write_bounded(path, value, cap):
    """Canonical JSON; retain the first crossing chunk and report quota failure."""
    encoder = json.JSONEncoder(sort_keys=True, ensure_ascii=True, separators=(',', ':'), allow_nan=False)
    size = 0
    with Path(path).open('xb') as output:
        try:
            for part in encoder.iterencode(value):
                raw = part.encode('ascii')
                for offset in range(0, len(raw), 4096):
                    chunk = raw[offset:offset+4096]
                    output.write(chunk)
                    size += len(chunk)
                    if size > cap:
                        raise OSError('parallel reduction metadata quota exceeded')
        finally:
            output.flush()
            os.fsync(output.fileno())


def _request(raw, verify=True):
    C = frozen_driver()
    if type(raw) is not bytes or len(raw) > REQUEST_CAP:
        raise ValueError('owned reduction request exceeds16KiB')
    row = C.metadata_json(raw, REQUEST_KEYS)
    if row['schema'] != 'q4-composite-parallel-reduce-request-1':
        raise ValueError('parallel reduction request schema')
    header = C.validate_range_header(C.canonical(row['expected_header']))
    for name in ('map', 'descriptor', 'range_stream', 'native_stdout', 'native_stderr'):
        C.artifact(row[name], verify)
    C.absolute_path(row['output'])
    C.integer(row['native_exit_code'], 'native exit code', -(1 << 31), (1 << 32)-1)
    pins = row['source_sha256']
    if type(pins) is not dict or not pins:
        raise ValueError('complete source map required')
    for name, digest in pins.items():
        C.absolute_path(name)
        C.hash_text(digest)
    if pins.get(str(FROZEN_DRIVER)) != FROZEN_SHA256 or pins.get(str(Path(__file__).resolve())) != C.sha(__file__):
        raise ValueError('request omits actual module/frozen decoder identity')
    if row['map']['sha256'] != header['group_map_sha256'] or row['descriptor']['sha256'] != header['input_descriptor_sha256']:
        raise ValueError('request descriptor/map differ from authenticated header')
    if verify:
        C.check_pins(pins)
        descriptor = C.validate_descriptor(C.read_artifact(row['descriptor'], 65536))
        if any(pins.get(path) != digest for path, digest in descriptor['source_sha256'].items()) or descriptor['tier'] != header['tier']:
            raise ValueError('request source map or tier differs from descriptor')
    return row


def _ref(path):
    C = frozen_driver()
    return {'path': str(Path(path).resolve()), 'sha256': C.sha(path)}


def run_range(request_raw, check=lambda: None):
    """Reduce one owned range; process/deadline/Job ownership stays with caller.

    A nonzero native result or interrupted authenticated prefix is retained as
    INTERRUPTED. Unexpected exceptions/quota crossings retain failure evidence
    and propagate; they never synthesize a completed range.
    """
    C = frozen_driver()
    request = _request(request_raw)
    check()
    output = Path(request['output']).resolve()
    for name in ('map', 'descriptor', 'range_stream', 'native_stdout', 'native_stderr'):
        if Path(request[name]['path']).resolve().is_relative_to(output):
            raise ValueError('reduction output overlaps immutable input')
    output.mkdir(parents=True, exist_ok=False)
    try:
        descriptor = C.validate_descriptor(C.read_artifact(request['descriptor'], 65536))
        C.verify_import_closure()
        header = request['expected_header']
        with C.MapIndex(request['map']['path'], request['map']['sha256'], request['descriptor']['sha256'], descriptor['actions']['sha256']) as mapping:
            width = mapping.header['N'] // 64
            groups = []
            for group in range(mapping.header['G_execution']):
                if group % 256 == 0:
                    check()
                gid, representative, multiplicity, orbit = mapping.group_metadata(group)
                if representative // width == header['range']:
                    groups.append(gid)
            complete = bytearray(mapping.header['G_execution'])
            reduction = C.Reduction()
            report = reduce_range(request['range_stream']['path'], header, mapping, groups, complete, reduction.add, check)
            report['exit_code'] = request['native_exit_code']
            error = ''
            try:
                report['native_accounting'] = C.decode_native_terminal(C.read_artifact(request['native_stdout'], 65536), report)
            except (ValueError, UnicodeError, OSError) as failure:
                error = ('native accounting: ' + str(failure)).encode('ascii', 'backslashreplace').decode('ascii')[:1024]
            report['stderr_sha256'] = request['native_stderr']['sha256']
            completed = [group for group in groups if complete[group]]
            if report['raw_sha256'] != request['range_stream']['sha256']:
                raise ValueError('consumed range differs from pinned input')
            if report['completed_groups'] != len(completed) or reduction.groups != len(completed):
                raise ValueError('private completion bitmap and reduction disagree')
            if report['status'] != 'COMPLETE' and not error:
                error = 'authenticated range is interrupted'
            if request['native_exit_code'] and not error:
                error = 'native process exited unsuccessfully'
            partial_path = output/'partial.json'
            _write_bounded(partial_path, reduction.report(), PARTIAL_CAP)
            result = {'schema': 'q4-composite-parallel-reduce-result-1',
                      'request_sha256': hashlib.sha256(request_raw).hexdigest(), 'module_sha256': C.sha(__file__),
                      'frozen_decoder_sha256': FROZEN_SHA256, 'range': header['range'],
                      'status': 'INTERRUPTED' if error else 'COMPLETE', 'range_receipt': report,
                      'completed_groups': completed, 'partial': _ref(partial_path), 'error': error}
        check()
        _request(request_raw)  # Recheck every source/input identity after read.
        closure = C.verify_import_closure()
        closure.update({Path(path).resolve().relative_to(ROOT).as_posix(): digest
                        for path, digest in request['source_sha256'].items()
                        if Path(path).resolve().is_relative_to(ROOT) and Path(path).suffix == '.py'})
        C.verify_loaded_imports(closure)
        _write_bounded(output/'result.json', result, RESULT_CAP)
        check()
        return result
    except BaseException as error:
        failure = {'schema': 'q4-composite-parallel-reduce-failure-1', 'request_sha256': hashlib.sha256(request_raw).hexdigest(),
                   'error': repr(error).encode('ascii', 'backslashreplace').decode('ascii')[:1024], 'complete': False}
        if 'report' in locals():
            failure['retained_range_receipt'] = report
        if 'completed' in locals():
            failure['completed_groups'] = completed
        _write_bounded(output/'failure.json', failure, RESULT_CAP)
        raise


def _histogram_key(key, C):
    if type(key) is not str or len(key) > 4096:
        raise ValueError('reduction histogram key bound')
    row = json.loads(key)
    if type(row) is not list or len(row) != 2 or C.canonical(row).decode('ascii') != key:
        raise ValueError('noncanonical histogram key')
    name, value = row
    if name in _FIRST:
        if value is not None:
            C.integer(value, name, 1, 304)
    elif name in _TOTAL:
        C.integer(value, name, 0, 304)
    elif name in _BOOL:
        if type(value) is not bool:
            raise ValueError('histogram Boolean required')
    elif name in ('escape_reason', 'classification'):
        C.integer(value, name, 0, 3 if name == 'escape_reason' else 2)
    elif name == 'unordered_drifts':
        if type(value) is not list or len(value) not in (0, 2) or value != sorted(value):
            raise ValueError('unordered drift shape/order')
        for vector in value:
            if type(vector) is not list or len(vector) != 3 or any(type(v) is not int or abs(v) > 1 for v in vector) or sum(abs(v) for v in vector) != 1:
                raise ValueError('unit drift domain')
    elif name == 'final_graph_component_accounts':
        if type(value) is not list or not 1 <= len(value) <= 12:
            raise ValueError('component histogram shape')
        total_plus = total_minus = total_q = 0
        for component in value:
            if type(component) is not list or len(component) != 5 or type(component[0]) is not list or len(component[0]) != 2:
                raise ValueError('component account shape')
            populations, f, k, q, sites = component
            total_plus += C.integer(populations[0], 'plus population', 0, 2)
            total_minus += C.integer(populations[1], 'minus population', 0, 2)
            C.integer(f, 'component F', 0, 4); C.integer(k, 'component K', 0, 4)
            total_q += C.integer(q, 'component Q', 0, 4)
            C.integer(sites, 'component sites', 1, 12)
            if q != f + k:
                raise ValueError('component account conservation')
        if (total_plus, total_minus, total_q) != (2, 2, 4):
            raise ValueError('global component sector')
    elif name == 'capture_period':
        if C.integer(value, name, 19, 304) % 19:
            raise ValueError('capture period congruence')
    elif name == 'capture_translation':
        if type(value) is not list or len(value) != 3 or any(type(v) is not int or abs(v) > 60 or v % 2 for v in value):
            raise ValueError('capture translation domain')
    elif name == 'capture_maximum_diameter':
        C.integer(value, name, 0, 8)
    elif name == 'capture_kind':
        if value not in ('translation', 'trap'):
            raise ValueError('capture kind')
    else:
        raise ValueError('unknown reduction histogram name')
    return name


def _partial(raw, group_count, original_count, max_weight, C):
    if len(raw) > PARTIAL_CAP:
        raise ValueError('partial reduction bound')
    row = C.parse_json(raw, REPORT_KEYS)
    if (C.integer(row['completed_execution_groups'], 'completed groups') != group_count
            or C.integer(row['weighted_originals'], 'weighted originals') != original_count
            or row['complete_witness_inventory'] != C.Reduction().report()['complete_witness_inventory']):
        raise ValueError('partial group/original/provenance inventory')
    unique, weighted = row['unique_group_counts'], row['original_weighted_counts']
    if type(unique) is not dict or type(weighted) is not dict or set(unique) != set(weighted):
        raise ValueError('partial histogram identity set')
    sums_u = {name: 0 for name in (*_FIXED, *_CAPTURE)}
    sums_w = dict(sums_u)
    for key, count in unique.items():
        name = _histogram_key(key, C)
        upper = 2296 * group_count if name in _CAPTURE else group_count
        amount = C.integer(count, 'unique histogram count', 1, upper)
        weight = C.integer(weighted[key], 'weighted histogram count', amount, amount * max_weight)
        sums_u[name] += amount
        sums_w[name] += weight
    if any(sums_u[name] != group_count or sums_w[name] != original_count for name in _FIXED):
        raise ValueError('partial histogram omits or duplicates groups/originals')
    if len({sums_u[name] for name in _CAPTURE}) != 1 or len({sums_w[name] for name in _CAPTURE}) != 1:
        raise ValueError('capture histogram inventories differ')
    for counts, sums in ((unique, sums_u), (weighted, sums_w)):
        capture_cases = counts.get(C.canonical(['capture_case', True]).decode('ascii'), 0)
        if counts.get(C.canonical(['classification', 0]).decode('ascii'), 0) != capture_cases:
            raise ValueError('classification and capture inventory differ')
        if not capture_cases <= sums['capture_period'] <= 2296 * capture_cases:
            raise ValueError('capture witness count outside case bounds')
    return row


def merge_results(admitted_map, ownership, multiplicities, items, check=lambda: None):
    """Stream exactly64 ordered owned results; return frozen integer reduction.

    Ownership/multiplicities must come from frozen validate_all_aliases. The
    worker's pinned code admits individual witnesses; this merge checks exact
    transport, domains and global coverage without replaying trajectories.
    """
    C = frozen_driver()
    if len(ownership) != 64 or len(multiplicities) != admitted_map.header['G_execution']:
        raise ValueError('admitted ownership/multiplicity inventory')
    complete = bytearray(admitted_map.header['G_execution'])
    reduction = C.Reduction()
    receipts = []
    map_digest = C.sha(admitted_map.path)
    descriptor_source = None
    for index, (request_raw, result_ref) in enumerate(items):
        check()
        if index >= 64:
            raise ValueError('extra reduction range')
        request = _request(request_raw, False)
        if request['expected_header']['range'] != index:
            raise ValueError('reduction responses must follow exact range order')
        if (Path(request['map']['path']).resolve() != admitted_map.path or request['map']['sha256'] != map_digest
                or request['descriptor']['sha256'] != admitted_map.header['input_descriptor_sha256']
                or request['expected_header']['tier'] != admitted_map.header['tier']):
            raise ValueError('request differs from the parent admitted map')
        if descriptor_source is None:
            descriptor = C.validate_descriptor(C.read_artifact(request['descriptor'], 65536), False)
            descriptor_source = descriptor['source_sha256']
        if any(request['source_sha256'].get(path) != digest for path, digest in descriptor_source.items()):
            raise ValueError('request omits an admitted scientific source')
        result = C.parse_json(C.read_artifact(result_ref, RESULT_CAP), RESULT_KEYS)
        expected_path = Path(request['output']).resolve()/'result.json'
        if Path(result_ref['path']).resolve() != expected_path:
            raise ValueError('result outside owned output')
        if (result['schema'] != 'q4-composite-parallel-reduce-result-1' or result['status'] != 'COMPLETE' or result['error'] != ''
                or result['request_sha256'] != hashlib.sha256(request_raw).hexdigest()
                or result['module_sha256'] != C.sha(__file__) or result['frozen_decoder_sha256'] != FROZEN_SHA256
                or C.integer(result['range'], 'result range', 0, 63) != index):
            raise ValueError('incomplete/stale reduction result identity')
        ids = result['completed_groups']
        if type(ids) is not list or any(type(g) is not int for g in ids) or tuple(ids) != tuple(ownership[index]):
            raise ValueError('completed groups differ from admitted ownership')
        originals = 0
        for group in ids:
            C.integer(group, 'completed group', 0, len(complete)-1)
            if complete[group]:
                raise ValueError('duplicate global completed group')
            metadata = admitted_map.group_metadata(group)
            if metadata[0] != group or metadata[1] // (admitted_map.header['N']//64) != index or metadata[2] != multiplicities[group]:
                raise ValueError('completed group ownership/multiplicity changed')
            complete[group] = 1
            originals += multiplicities[group]
        report = result['range_receipt']
        if (type(report) is not dict or C.canonical(report.get('header')) != C.canonical(request['expected_header'])
                or report.get('raw_sha256') != request['range_stream']['sha256'] or report.get('path') != request['range_stream']['path']
                or report.get('status') != 'COMPLETE' or report.get('exit_code') != 0
                or report.get('stderr_sha256') != request['native_stderr']['sha256']):
            raise ValueError('range receipt input/completion identity')
        if (C.integer(report['range'], 'receipt range', 0, 63) != index or C.integer(report['exit_code'], 'receipt exit code') != 0
                or any(C.integer(report[name], name) != len(ids) for name in ('completed_groups', 'begun_groups', 'owned_groups'))
                or C.integer(report['verified_durable_ticks'], 'durable ticks') != 304 * len(ids)):
            raise ValueError('range completion accounting')
        if (report['terminal_frame'] is not True or report['tail_error'] is not None
                or C.integer(report['tail_bytes'], 'tail bytes') != 0
                or any(C.integer(report[name], name) != C.integer(report['raw_bytes'], 'raw bytes') for name in ('accepted_bytes', 'tail_offset'))
                or C.integer(report['accepted_frames'], 'accepted frames') != 18 * len(ids) + 1
                or report['tail_sha256'] != hashlib.sha256(b'').hexdigest()):
            raise ValueError('complete receipt has missing/unaccepted frames')
        terminal = report['terminal']
        for name in ('range', 'status', 'start', 'stop', 'owned_groups', 'completed_groups', 'reported_executed_ticks',
                     'verified_durable_ticks', 'locally_covered_originals', 'unresolved_aliases', 'error_code'):
            C.integer(terminal[name], 'terminal ' + name)
        C.range_final_wire(terminal)
        local = 0
        for original in range(request['expected_header']['start'], request['expected_header']['stop']):
            row_original = admitted_map.original(original)
            local += row_original['owner_range'] == index and bool(complete[row_original['group']])
        for name, value in {'range': index, 'status': 0, 'start': request['expected_header']['start'], 'stop': request['expected_header']['stop'],
                            'owned_groups': len(ids), 'completed_groups': len(ids), 'reported_executed_ticks': 608 * len(ids),
                            'verified_durable_ticks': 304 * len(ids), 'locally_covered_originals': local,
                            'unresolved_aliases': request['expected_header']['stop'] - request['expected_header']['start'] - local,
                            'group_map_sha256': map_digest}.items():
            if terminal[name] != value:
                raise ValueError('range terminal differs from admitted coverage')
        native = C.decode_native_terminal(C.read_artifact(request['native_stdout'], 65536), report)
        if report.get('native_accounting') != native:
            raise ValueError('range native invocation accounting differs')
        partial = result['partial']
        if Path(partial['path']).resolve() != expected_path.with_name('partial.json'):
            raise ValueError('partial outside owned output')
        row = _partial(C.read_artifact(partial, PARTIAL_CAP), len(ids), originals, max((multiplicities[g] for g in ids), default=1), C)
        reduction.groups += row['completed_execution_groups']
        reduction.originals += row['weighted_originals']
        for target, source in ((reduction.unique, row['unique_group_counts']), (reduction.weighted, row['original_weighted_counts'])):
            for key, count in source.items():
                target[key] = target.get(key, 0) + count
        receipts.append(report)
    if len(receipts) != 64 or not all(complete) or reduction.groups != len(complete) or reduction.originals != admitted_map.header['N'] or sum(multiplicities) != admitted_map.header['N']:
        raise ValueError('global range/group/original coverage incomplete')
    check()
    return reduction, complete, tuple(receipts)

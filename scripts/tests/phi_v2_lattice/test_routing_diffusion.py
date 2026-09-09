from dataclasses import replace
from itertools import product
import hashlib
import json

import numpy as np
import pytest

from phi_v2_lattice import routing_diffusion as R
from phi_v2_lattice import routing_diffusion_response as Q


def blank(L):
    return np.zeros((L, L, L, 2, 6), dtype=bool), np.zeros((L, L, L), dtype=np.uint16)


def test_every_permutation_every_one_polarity_pattern_through_runtime():
    # All 46,080 local records; the second polarity is the complementary pattern.
    bank, routers = blank(36)
    patterns = ((np.arange(64)[:, None] >> np.arange(6)) & 1).astype(bool)
    flat = bank.reshape(-1, 2, 6)
    flat[:46080, 0] = np.tile(patterns, (720, 1))
    flat[:46080, 1] = ~flat[:46080, 0]
    routers.reshape(-1)[:46080] = np.repeat(np.arange(720, dtype=np.uint16), 64)
    state = R.initialize(bank, routers)
    output = R.step(state)
    expected = np.empty((46080, 2, 6), dtype=bool)
    for p, permutation in enumerate(R.PERMUTATIONS):
        for source, target in enumerate(permutation):
            expected[p*64:(p+1)*64, :, target] = flat[p*64:(p+1)*64, :, source]
    assert np.array_equal(output.bank.reshape(-1, 2, 6)[:46080], expected)
    assert np.array_equal(output.bank.sum(axis=-1), state.bank.sum(axis=-1))
    assert all(sum(p[i] == j for p in R.PERMUTATIONS) == 120 for i in range(6) for j in range(6))


@pytest.mark.parametrize("L", [3, 4, 7])
def test_scalar_streaming_and_router_transport_at_all_channels_and_seams(L):
    bank, routers = blank(L)
    for p, c in product(range(2), range(6)):
        bank[L-1, 0, L-1, p, c] = True
    routers[:] = (np.arange(L**3).reshape(L, L, L) * 17) % 720
    state = replace(R.initialize(bank, routers), microtick=1)
    output = R.step(state)
    expected_bank, expected_routers = blank(L)
    for x in product(range(L), repeat=3):
        for p, c in product(range(2), range(6)):
            y = tuple((a+b) % L for a, b in zip(x, R.VELOCITIES[c]))
            expected_bank[y+(p,c)] = bank[x+(p,c)]
        y = tuple((a+1) % L for a in x)
        expected_routers[y] = routers[x]
    assert np.array_equal(output.bank, expected_bank)
    assert np.array_equal(output.routers, expected_routers)


def test_all_216_three_step_paths_execute_with_exact_counting_weights():
    # Representative router assignments; each chosen output leaves exactly5! local permutations.
    L, cycles, center = 11, 3, (5, 5, 5)
    counts = {}
    for word in product(range(6), repeat=cycles):
        bank, routers = blank(L)
        bank[center+(0,0)] = True
        point, incoming, origins = center, 0, set()
        for k, outgoing in enumerate(word):
            origin = tuple((a-2*k) % L for a in point)
            assert origin not in origins
            origins.add(origin)
            choices = [i for i,p in enumerate(R.PERMUTATIONS) if p[incoming] == outgoing]
            assert len(choices) == 120
            routers[origin] = choices[0]
            point = tuple(a+b for a,b in zip(point, R.VELOCITIES[outgoing]))
            incoming = outgoing
        state = R.initialize(bank, routers)
        for _ in range(2*cycles):
            state = R.step(state)
        assert state.bank[point+(0,incoming)] and int(state.bank.sum()) == 1
        offset = tuple(a-b for a,b in zip(point, center))
        counts[offset] = counts.get(offset, 0)+1
    assert counts == Q.endpoint_counts(3)
    assert sum(counts.values()) * Q.path_probability(L, cycles) == 1
    assert all(sum(count*x[j] for x,count in counts.items()) == 0 for j in range(3))
    assert all(sum(count*x[j]**2 for x,count in counts.items()) == 216 for j in range(3))


@pytest.mark.parametrize("phase", [0, 1])
def test_complete_replay_huge_clock_and_owned_outputs(phase):
    bank, routers = blank(4)
    bank[:] = (np.arange(bank.size).reshape(bank.shape) % 7) < 3
    routers[:] = np.arange(64).reshape(4,4,4)*9
    state = replace(R.initialize(bank, routers), microtick=10**5000+phase)
    original = R.checkpoint(state)
    population = R.populations(state)
    histogram = np.bincount(state.routers.ravel(), minlength=720)
    replay = R.restore(original)
    for _ in range(8):
        before = R.checkpoint(state)
        output, replay = R.step(state), R.step(replay)
        assert R.checkpoint(state) == before
        assert R.checkpoint(output) == R.checkpoint(replay)
        assert R.populations(output) == population
        assert np.array_equal(np.bincount(output.routers.ravel(), minlength=720), histogram)
        for a in (state.bank, state.routers):
            for b in (output.bank, output.routers):
                assert not np.shares_memory(a,b)
        state = output
    observation = R.densities(state)
    observation[:] = 0
    assert R.populations(state) == population
    assert np.array_equal(bank, R.restore(original).bank)


@pytest.mark.parametrize("phase", [0, 1])
def test_complete_record_causal_support(phase):
    bank, routers = blank(7)
    bank[3,3,3,0,0] = True
    state = replace(R.initialize(bank, routers), microtick=phase)
    perturbed = R.restore(R.checkpoint(state))
    perturbed.routers[3,3,3] = 719
    perturbed.bank[3,3,3,1,1] = True
    a,b = R.step(state),R.step(perturbed)
    changed = np.any(a.bank != b.bank,axis=(-1,-2)) | (a.routers != b.routers)
    points = np.argwhere(changed)
    assert len(points) and np.all(np.abs(points-3) <= 1)


def rewrite_header(blob, mutate, payload_mutate=lambda x:x):
    body = blob[len(R.MAGIC)+32:]
    n = int.from_bytes(body[:4],"little")
    header = json.loads(body[4:4+n])
    mutate(header)
    raw = json.dumps(header,sort_keys=True,separators=(",",":")).encode()
    body = len(raw).to_bytes(4,"little")+raw+payload_mutate(body[4+n:])
    return R.MAGIC+hashlib.sha256(body).digest()+body


@pytest.mark.parametrize("field,value", [("L",True),("L",2),("law","foreign"),("tick_hex","00"),
    ("tick_hex","A"),("tick_hex","-1"),("tick_hex",1),("tick_hex","0x1")])
def test_invalid_metadata_rejected(field,value):
    blob = R.checkpoint(R.initialize(*blank(3)))
    bad = rewrite_header(blob,lambda h:h.__setitem__(field,value))
    with pytest.raises(ValueError):R.restore(bad)


def test_corruption_router_alphabet_and_padding_rejected():
    blob = R.checkpoint(R.initialize(*blank(3)))
    bad = bytearray(blob);bad[-1]^=1
    for value in (bytes(bad),bytearray(blob),blob[:-1],b"OTHER"+blob):
        with pytest.raises(ValueError):R.restore(value)
    # 324 bits =>41bankbytes with four zero padding bits.
    bad = rewrite_header(blob,lambda h:None,lambda p:p[:40]+bytes([p[40]|128])+p[41:])
    with pytest.raises(ValueError):R.restore(bad)
    bad = rewrite_header(blob,lambda h:None,lambda p:p[:41]+(720).to_bytes(2,"little")+p[43:])
    with pytest.raises(ValueError):R.restore(bad)


@pytest.mark.parametrize("kind", ["clock", "router", "boolbytes", "shape", "law"])
def test_invalid_state_fails_without_mutation(kind):
    state = R.initialize(*blank(3))
    if kind == "clock":state=replace(state,microtick=True)
    if kind == "router":state.routers.flat[0]=720
    if kind == "boolbytes":state.bank.view(np.uint8).flat[0]=2
    if kind == "shape":state=replace(state,bank=state.bank[:2])
    if kind == "law":state=replace(state,law_id="foreign")
    before=(state.bank.tobytes(),state.routers.tobytes())
    with pytest.raises(ValueError):R.step(state)
    assert before==(state.bank.tobytes(),state.routers.tobytes())


def test_prediction_is_owned_and_scope_checked():
    initial=np.zeros((11,11,11,2),dtype=np.int64);initial[5,5,5,0]=1
    output=R.predict_densities(initial,3)
    for offset,count in Q.endpoint_counts(3).items():
        point=tuple(5+x for x in offset)
        assert output[point+(0,)] == pytest.approx(count/216)
    assert initial.sum()==1 and output.sum()==pytest.approx(1)
    with pytest.raises(ValueError):R.predict_densities(initial,4)
    with pytest.raises(ValueError):R.predict_densities(initial.astype(complex),1)
    with pytest.raises(ValueError):R.predict_densities(initial.astype("timedelta64[D]"),1)
    with pytest.raises(ValueError):Q.path_probability(9,3)

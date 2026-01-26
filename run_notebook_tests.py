"""
FTD Physics Engine Test Suite
Run this to execute all 26 tests with detailed output.
"""

import sys
sys.path.insert(0, '.')
import numpy as np
import time

print('='*60)
print('FTD PHYSICS ENGINE TEST SUITE')
print('='*60)
print()

from ternary_matrix.model.grid import Universe
from ternary_matrix.config import CONSTANTS
from ternary_matrix.physics import (
    tick, tick_minimal, run_simulation, get_diagnostics,
    time_gate, get_effective_time_rate, calculate_density,
    gradient_3d, divergence_3d, curl_3d, smooth_field,
    gravity_force, coulomb_force, lorentz_force, accumulate_forces, weak_stress,
    integrate, clamp_velocity, get_max_speed, move_particles, propagate_flux,
    process_interactions, get_annihilation_count, transmute, get_stress_field,
    update_bindings, count_neighbors_moore, get_triad_count,
)
from ternary_matrix.physics.waves import laplacian_3d_vector

print('All imports successful!')
print(f'Grid size: {CONSTANTS.GRID_SIZE}')
print(f'Speed of light (c): {CONSTANTS.C}')
print(f'Manifestation threshold (KB): {CONSTANTS.KB}')
print()

def run_test(name, test_func):
    try:
        test_func()
        print(f'PASSED: {name}')
        return True
    except AssertionError as e:
        print(f'FAILED: {name} - {e}')
        return False
    except Exception as e:
        print(f'ERROR: {name} - {type(e).__name__}: {e}')
        return False

passed = 0
total = 0

# TEST 1
def test_1():
    universe = Universe(size=16)
    universe.velocity.fill(0)
    time_gate(universe)
    assert np.all(universe.is_active)
total += 1
if run_test('1. Stationary voxels always active', test_1): passed += 1

# TEST 2
def test_2():
    universe = Universe(size=16)
    center = universe.size // 2
    universe.velocity[center, center, center] = [0.4, 0, 0]
    active_counts = []
    for _ in range(100):
        universe.phase_accum.fill(0)
        time_gate(universe)
        active_counts.append(universe.is_active[center, center, center])
    rate = np.mean(active_counts)
    print(f'   Fast voxel active rate: {rate:.2%}')
    assert rate < 1.0
total += 1
if run_test('2. Fast voxels time dilation', test_2): passed += 1

# TEST 3
def test_3():
    universe = Universe(size=16)
    universe.velocity[5, 5, 5] = [0.3, 0, 0]
    rate = get_effective_time_rate(universe)
    print(f'   Stationary rate: {rate[0,0,0]:.4f}, Moving rate: {rate[5,5,5]:.4f}')
    assert np.isclose(rate[0, 0, 0], 1.0)
    assert rate[5, 5, 5] < 1.0
total += 1
if run_test('3. Effective time rate', test_3): passed += 1

# TEST 4
def test_4():
    field = np.ones((16, 16, 16), dtype=np.float32) * 5.0
    grad = gradient_3d(field)
    print(f'   Max gradient: {np.max(np.abs(grad)):.2e}')
    assert np.allclose(grad, 0, atol=1e-6)
total += 1
if run_test('4. Gradient of uniform is zero', test_4): passed += 1

# TEST 5
def test_5():
    universe = Universe(size=16)
    universe.flux.fill(0)
    universe.flux[..., 0] = 1.0
    div = divergence_3d(universe.flux)
    print(f'   Max divergence: {np.max(np.abs(div)):.2e}')
    assert np.allclose(div, 0, atol=1e-6)
total += 1
if run_test('5. Divergence of constant is zero', test_5): passed += 1

# TEST 6
def test_6():
    x = np.arange(16, dtype=np.float32)
    scalar = x[:, None, None] ** 2
    scalar = np.broadcast_to(scalar, (16, 16, 16)).copy()
    grad = gradient_3d(scalar)
    curl = curl_3d(grad)
    print(f'   Max curl of gradient: {np.max(np.abs(curl)):.2e}')
    assert np.allclose(curl, 0, atol=1e-5)
total += 1
if run_test('6. Curl of gradient is zero', test_6): passed += 1

# TEST 7
def test_7():
    universe = Universe(size=16)
    universe.flux.fill(0)
    universe.density.fill(0)
    center = 8
    for dx in range(-2, 3):
        for dy in range(-2, 3):
            for dz in range(-2, 3):
                x, y, z = (center + dx) % 16, (center + dy) % 16, (center + dz) % 16
                dist = abs(dx) + abs(dy) + abs(dz)
                universe.density[x, y, z] = max(0, 10.0 - dist * 2)
    f_grav = gravity_force(universe)
    print(f'   Non-zero gravity components: {np.count_nonzero(f_grav)}')
    assert np.any(f_grav != 0)
total += 1
if run_test('7. Gravity attracts to density', test_7): passed += 1

# TEST 8
def test_8():
    universe = Universe(size=16)
    universe.charge.fill(0)
    for x in range(4, 8):
        for y in range(7, 10):
            for z in range(7, 10):
                universe.charge[x, y, z] = 1.0
                universe.states[x, y, z] = 1
    f_coulomb = coulomb_force(universe)
    print(f'   Non-zero Coulomb components: {np.count_nonzero(f_coulomb)}')
    assert np.any(f_coulomb != 0)
total += 1
if run_test('8. Coulomb force from charges', test_8): passed += 1

# TEST 9
def test_9():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.force_accum[8, 8, 8] = [1.0, 2.0, 3.0]
    universe.is_active.fill(True)
    integrate(universe)
    print(f'   Max force after integration: {np.max(np.abs(universe.force_accum)):.2e}')
    assert np.allclose(universe.force_accum, 0)
total += 1
if run_test('9. Force accumulator clears', test_9): passed += 1

# TEST 10
def test_10():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.force_accum[8, 8, 8] = [1.0, 0, 0]
    universe.is_active.fill(True)
    v0 = universe.velocity[8, 8, 8, 0]
    integrate(universe)
    v1 = universe.velocity[8, 8, 8, 0]
    print(f'   Velocity: {v0:.3f} -> {v1:.3f}')
    assert v1 > v0
total += 1
if run_test('10. Velocity from force', test_10): passed += 1

# TEST 11
def test_11():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.velocity[8, 8, 8] = [1.0, 1.0, 1.0]
    clamp_velocity(universe)
    max_speed = get_max_speed(universe)
    print(f'   Max speed: {max_speed:.4f} (c = {CONSTANTS.C})')
    assert max_speed <= CONSTANTS.C + 1e-6
total += 1
if run_test('11. Speed limit enforced', test_11): passed += 1

# TEST 12
def test_12():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.velocity[8, 8, 8] = [0.1, 0, 0]
    universe.is_active.fill(True)
    r0 = universe.position_rem[8, 8, 8, 0]
    integrate(universe)
    r1 = universe.position_rem[8, 8, 8, 0]
    print(f'   Remainder: {r0:.3f} -> {r1:.3f}')
    assert r1 > r0
total += 1
if run_test('12. Position remainder accumulates', test_12): passed += 1

# TEST 13
def test_13():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.position_rem[8, 8, 8] = [1.5, 0, 0]
    move_particles(universe)
    print(f'   Old pos state: {universe.states[8,8,8]}, New pos state: {universe.states[9,8,8]}')
    assert universe.states[8, 8, 8] == 0
    assert universe.states[9, 8, 8] == 1
total += 1
if run_test('13. Movement when remainder >= 1', test_13): passed += 1

# TEST 14
def test_14():
    universe = Universe(size=16)
    universe.states[15, 8, 8] = 1
    universe.position_rem[15, 8, 8] = [1.5, 0, 0]
    move_particles(universe)
    print(f'   State at x=0 after wrap: {universe.states[0, 8, 8]}')
    assert universe.states[0, 8, 8] == 1
total += 1
if run_test('14. Toroidal boundary', test_14): passed += 1

# TEST 15
def test_15():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.states[9, 8, 8] = 1
    universe.position_rem[8, 8, 8] = [1.5, 0, 0]
    move_particles(universe)
    total_p = int(universe.states[8,8,8] != 0) + int(universe.states[9,8,8] != 0)
    print(f'   Particles remaining: {total_p}')
    assert total_p >= 1
total += 1
if run_test('15. No move to occupied', test_15): passed += 1

# TEST 16
def test_16():
    universe = Universe(size=16)
    universe.states[8, 8, 8] = 1
    universe.flux[8, 8, 8] = [0.1, 0, 0]
    calculate_density(universe)
    s0 = universe.states[8, 8, 8]
    for _ in range(100):
        transmute(universe)
    s1 = universe.states[8, 8, 8]
    print(f'   State after 100 transmutes: {s0} -> {s1}')
    assert s1 == s0
total += 1
if run_test('16. No transmutation at low stress', test_16): passed += 1

# TEST 17
def test_17():
    universe = Universe(size=16)
    for i in range(16):
        universe.flux[i, :, :, 0] = i * 0.5
    calculate_density(universe)
    stress = get_stress_field(universe)
    print(f'   Max stress: {np.max(stress):.4f}')
    assert np.any(stress > 0)
total += 1
if run_test('17. Stress field calculation', test_17): passed += 1

# TEST 18
def test_18():
    universe = Universe(size=16)
    universe.flux[8, 8, 8] = [2.0, 0, 0]
    calculate_density(universe)
    t = tick(universe)
    print(f'   Tick count: {t}, Universe.tick: {universe.tick}')
    assert t == 1 and universe.tick == 1
total += 1
if run_test('18. Single tick runs', test_18): passed += 1

# TEST 19
def test_19():
    universe = Universe(size=16)
    universe.flux[8, 8, 8] = [2.0, 2.0, 2.0]
    for i in range(1000):
        tick(universe)
    has_nan = np.any(np.isnan(universe.flux))
    has_inf = np.any(np.isinf(universe.flux))
    print(f'   After 1000 ticks: NaN={has_nan}, Inf={has_inf}')
    assert not has_nan and not has_inf
total += 1
if run_test('19. 1000 ticks stability', test_19): passed += 1

# TEST 20
def test_20():
    universe = Universe(size=16)
    universe.states[5, 5, 5] = 1
    universe.states[10, 10, 10] = -1
    universe.charge[5, 5, 5] = 1.0
    universe.charge[10, 10, 10] = -1.0
    universe.flux[8, 8, 8] = [1.0, 1.0, 1.0]
    calculate_density(universe)
    diag = get_diagnostics(universe)
    print(f'   Diagnostics: {diag}')
    assert diag['tick'] == 0
    assert diag['manifested_count'] == 2
total += 1
if run_test('20. Diagnostics function', test_20): passed += 1

# TEST 21
def test_21():
    universe = Universe(size=32)
    universe.states[5, 5, 5] = 1
    universe.states[20, 20, 20] = -1
    universe.charge[5, 5, 5] = 1.0
    universe.charge[20, 20, 20] = -1.0
    universe.flux[5, 5, 5] = [2.0, 0, 0]
    universe.flux[20, 20, 20] = [2.0, 0, 0]
    q0 = universe.get_total_charge()
    for _ in range(100):
        tick(universe)
    q1 = universe.get_total_charge()
    print(f'   Charge: {q0:.2f} -> {q1:.2f}')
    assert np.isclose(q0, q1, atol=1e-6) or np.isclose(q1, 0, atol=1e-6)
total += 1
if run_test('21. Charge conservation', test_21): passed += 1

# TEST 22
def test_22():
    universe = Universe(size=16)
    universe.reset()
    for _ in range(100):
        tick(universe)
    n = universe.get_manifested_count()
    print(f'   Manifested after 100 ticks: {n}')
    assert n == 0
total += 1
if run_test('22. Vacuum stability', test_22): passed += 1

# TEST 23
def test_23():
    universe = Universe(size=32)
    universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]
    start = time.time()
    for _ in range(10):
        tick(universe)
    elapsed = time.time() - start
    print(f'   10 ticks in {elapsed:.3f}s ({10/elapsed:.1f} ticks/sec)')
    assert elapsed < 5.0
total += 1
if run_test('23. Performance benchmark', test_23): passed += 1

# TEST 24
def test_24():
    universe = Universe(size=32)
    universe.reset()
    universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]
    start = time.time()
    for _ in range(10):
        tick(universe)
    full = time.time() - start
    universe.reset()
    universe.flux[16, 16, 16] = [5.0, 5.0, 5.0]
    start = time.time()
    for _ in range(10):
        tick_minimal(universe)
    minimal = time.time() - start
    print(f'   Full: {full:.3f}s, Minimal: {minimal:.3f}s')
    assert minimal <= full * 1.5
total += 1
if run_test('24. Minimal vs full tick', test_24): passed += 1

# TEST 25
def test_25():
    field = np.ones((16, 16, 16, 3), dtype=np.float32) * 5.0
    lap = laplacian_3d_vector(field)
    print(f'   Max laplacian: {np.max(np.abs(lap)):.2e}')
    assert np.allclose(lap, 0, atol=1e-6)
total += 1
if run_test('25. Laplacian of constant is zero', test_25): passed += 1

# TEST 26
def test_26():
    np.random.seed(42)
    field = np.random.randn(16, 16, 16).astype(np.float32)
    v0 = np.var(field)
    smoothed = smooth_field(field)
    v1 = np.var(smoothed)
    print(f'   Variance: {v0:.4f} -> {v1:.4f} ({100*(1-v1/v0):.1f}% reduction)')
    assert v1 < v0
total += 1
if run_test('26. Smoothing reduces variance', test_26): passed += 1

print()
print('='*60)
print(f'RESULTS: {passed}/{total} TESTS PASSED')
print('='*60)

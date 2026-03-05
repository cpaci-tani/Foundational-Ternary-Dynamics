# Phase 2.4: Simulation Engine Completion Plan

## Executive Summary

**Objective:** Complete the FTD simulation engine to implement all 12 update cycle phases as specified in CLAUDE.md Chapter 5.

**Current State:** 6 of 12 phases implemented (50%)
**Target State:** 12 of 12 phases implemented (100%)
**Estimated Effort:** 8-12 hours

---

## Current Implementation Status

| Phase | Name | Status | Notes |
|-------|------|--------|-------|
| 1 | Time Gating | ❌ Missing | Relativistic lag proxy |
| 2 | Entropy | ✅ Done | `apply_decay()` |
| 3 | Existence Transitions | ✅ Done | `update_manifestation()` |
| 4 | Wave Propagation | ✅ Done | `waves.propagate_flux()` |
| 5 | Field Computation | ✅ Done | `calculate_density()` |
| 6 | Force Accumulation | ⚠️ Partial | Only density; missing 5 forces |
| 7 | Integration | ❌ Missing | Velocity/position updates |
| 8 | Movement | ❌ Missing | Lattice movement |
| 9 | Collisions | ✅ Done | `process_interactions()` |
| 10 | Transmutation | ❌ Missing | Weak-force polarity flips |
| 11 | Binding | ✅ Done | `update_bindings()` |
| 12 | Increment | ❌ Missing | Tick counter (trivial) |

---

## Implementation Plan

### Task 1: Extend Voxel Data Structure (1 hour)

**File:** `ternary_matrix/model/grid.py`

Add arrays to Universe class:
```python
# Current structure
self.state        # {-1, 0, +1}
self.flux         # (N, N, N, 3) R³ vector field
self.wave_vel     # (N, N, N, 3) wave equation scratch
self.density      # (N, N, N) |J|
self.locked       # (N, N, N) bool

# NEW: Add these
self.velocity     # (N, N, N, 3) particle velocities
self.position_rem # (N, N, N, 3) sub-lattice position remainders
self.force_accum  # (N, N, N, 3) force accumulator per tick
self.phase_accum  # (N, N, N) phase accumulator for time gating
self.is_active    # (N, N, N) bool - passed time gate this tick
self.charge       # (N, N, N) fractional charge values
self.tick         # int - global tick counter
```

**Acceptance Criteria:**
- [ ] All new arrays initialized with proper shapes/dtypes
- [ ] Memory overhead acceptable (estimate: +50% from current ~500MB for 256³)
- [ ] Accessors/mutators added to Universe class

---

### Task 2: Implement Phase 1 - Time Gating (1.5 hours)

**File:** `ternary_matrix/physics/time_gating.py` (new)

**Specification (from CLAUDE.md §5.1):**
- Check phase accumulators (relativistic lag proxy)
- Mark active voxels

**Implementation:**
```python
def time_gate(universe):
    """
    Phase 1: Time Gating

    Voxels with higher velocity accumulate phase slower,
    simulating time dilation.

    phase_rate = 1 / gamma = sqrt(1 - v²/c²)
    """
    v_squared = np.sum(universe.velocity**2, axis=-1)
    c_squared = config.C**2

    # Lorentz factor (clamped to avoid division issues)
    gamma = 1 / np.sqrt(np.maximum(1 - v_squared/c_squared, 0.01))

    # Accumulate phase (slower for faster particles)
    universe.phase_accum += 1.0 / gamma

    # Mark active: phase crossed integer threshold
    universe.is_active = universe.phase_accum >= 1.0

    # Reset phase for active voxels
    universe.phase_accum[universe.is_active] -= 1.0
```

**Acceptance Criteria:**
- [ ] Stationary voxels always active (phase_rate = 1)
- [ ] Fast voxels update less frequently (time dilation)
- [ ] Numerical stability near c

---

### Task 3: Implement Phase 6 - Force Accumulation (3 hours)

**File:** `ternary_matrix/physics/forces.py` (extend)

#### 3.1 Gravity-like Force
```python
def gravity_force(universe):
    """
    F_grav(v) = G_N × ∇ρ̄(v)

    where ρ̄ is smoothed density (6-neighbor average)
    """
    rho_smoothed = smooth_field(universe.density)
    grad_rho = gradient_3d(rho_smoothed)
    return config.GRAVITY_BIAS * grad_rho
```

#### 3.2 Coulomb-like Force
```python
def coulomb_force(universe):
    """
    F_elec(v) = -q(v) × ∇q̄(v)

    Like charges repel, opposite attract
    """
    q_smoothed = smooth_field(universe.charge)
    grad_q = gradient_3d(q_smoothed)
    return -universe.charge[..., np.newaxis] * grad_q
```

#### 3.3 Lorentz-like Force
```python
def lorentz_force(universe):
    """
    F_mag(v) = β × (∇×J) × ĵ(v)

    Requires curl operator implementation
    """
    curl_J = curl_3d(universe.flux)
    j_hat = normalize_field(universe.flux)
    return config.BETA * np.cross(curl_J, j_hat)
```

#### 3.4 Strong-like Force (Yukawa)
```python
def strong_force(universe):
    """
    F_strong(r) = g_s² × exp(-m_π r)/r² × (1 + m_π r)

    Short-range, acts only on adjacent manifested voxels
    """
    # Identify manifested neighbor pairs
    # Apply Yukawa potential between them
    ...
```

#### 3.5 Weak-like Force (Stress Calculation)
```python
def weak_stress(universe):
    """
    stress(v) = |∇·J| + |∇×J| + |∇ρ|

    Used in Phase 10 for transmutation trigger
    """
    div_J = divergence_3d(universe.flux)
    curl_J = curl_3d(universe.flux)
    grad_rho = gradient_3d(universe.density)

    return (np.abs(div_J) +
            np.linalg.norm(curl_J, axis=-1) +
            np.linalg.norm(grad_rho, axis=-1))
```

#### 3.6 Helper Functions Needed
```python
def gradient_3d(field):
    """Discrete gradient using central differences"""
    ...

def divergence_3d(vector_field):
    """Discrete divergence"""
    ...

def curl_3d(vector_field):
    """Discrete curl - NEW, not currently implemented"""
    ...

def smooth_field(field):
    """6-neighbor averaging"""
    ...

def normalize_field(vector_field):
    """Return unit vectors, handling zeros"""
    ...
```

**Acceptance Criteria:**
- [ ] All 5 force types implemented
- [ ] Curl operator correctly implemented (verify ∇×(∇f) = 0)
- [ ] Force accumulator properly summed
- [ ] Unit tests for each force type

---

### Task 4: Implement Phase 7 - Integration (1 hour)

**File:** `ternary_matrix/physics/integration.py` (new)

```python
def integrate(universe, dt=1.0):
    """
    Phase 7: Integration

    Update velocities from forces, accumulate position remainders.
    Only process active voxels.
    """
    active = universe.is_active & (universe.state != 0)

    # Update velocity: v += F × dt (assuming unit mass)
    universe.velocity[active] += universe.force_accum[active] * dt

    # Clamp to speed of light
    speed = np.linalg.norm(universe.velocity, axis=-1, keepdims=True)
    too_fast = speed > config.C
    universe.velocity[too_fast[..., 0]] *= config.C / speed[too_fast]

    # Accumulate position remainder
    universe.position_rem[active] += universe.velocity[active] * dt

    # Clear force accumulator for next tick
    universe.force_accum.fill(0)
```

**Acceptance Criteria:**
- [ ] Velocity updates respect speed limit
- [ ] Position remainder accumulates correctly
- [ ] Force accumulator cleared after integration

---

### Task 5: Implement Phase 8 - Movement (1.5 hours)

**File:** `ternary_matrix/physics/movement.py` (new)

```python
def move_particles(universe):
    """
    Phase 8: Movement

    When position remainder >= 1, move voxel state to new position.
    Handle collisions (empty, same-sign, opposite-sign targets).
    """
    manifested = np.where(universe.state != 0)

    for idx in zip(*manifested):
        rem = universe.position_rem[idx]

        # Check if movement threshold reached
        for axis in range(3):
            if abs(rem[axis]) >= 1.0:
                direction = int(np.sign(rem[axis]))
                new_idx = list(idx)
                new_idx[axis] = (idx[axis] + direction) % universe.size
                new_idx = tuple(new_idx)

                # Check target state
                target_state = universe.state[new_idx]
                source_state = universe.state[idx]

                if target_state == 0:
                    # Empty: move
                    universe.state[new_idx] = source_state
                    universe.state[idx] = 0
                    transfer_properties(universe, idx, new_idx)

                elif target_state == source_state:
                    # Same sign: elastic collision
                    # Reverse velocities, exchange momentum
                    elastic_collision(universe, idx, new_idx)

                else:
                    # Opposite sign: annihilation (handled in Phase 9)
                    pass

                # Consume movement
                universe.position_rem[idx][axis] -= direction
```

**Acceptance Criteria:**
- [ ] Particles move when remainder exceeds threshold
- [ ] Boundary conditions (toroidal wrap) handled
- [ ] Empty, same-sign, opposite-sign cases correct
- [ ] Properties transferred with movement

---

### Task 6: Implement Phase 10 - Transmutation (1 hour)

**File:** `ternary_matrix/physics/transmutation.py` (new)

```python
def transmute(universe):
    """
    Phase 10: Transmutation

    High field stress enables polarity flip (+1 ↔ -1).
    This is the weak-force analog.
    """
    # Calculate stress from Phase 6
    stress = weak_stress(universe)

    # Find manifested voxels exceeding threshold
    candidates = (universe.state != 0) & (stress > config.WEAK_THRESHOLD)

    # Probabilistic flip (not deterministic)
    flip_prob = (stress[candidates] - config.WEAK_THRESHOLD) / config.WEAK_THRESHOLD
    flip_prob = np.clip(flip_prob, 0, 0.5)  # Max 50% chance per tick

    do_flip = np.random.random(np.sum(candidates)) < flip_prob

    # Apply flips
    flip_indices = np.where(candidates)
    for i, idx in enumerate(zip(*flip_indices)):
        if do_flip[i]:
            universe.state[idx] *= -1  # +1 → -1 or -1 → +1
```

**Config Addition:**
```python
WEAK_THRESHOLD = 10.0  # Stress threshold for transmutation
```

**Acceptance Criteria:**
- [ ] Only high-stress voxels can transmute
- [ ] Probability scales with excess stress
- [ ] State properly flips between +1 and -1

---

### Task 7: Implement Phase 12 - Increment (0.5 hour)

**File:** `ternary_matrix/physics/master_equation.py` (update)

```python
def tick(universe):
    """Complete update cycle"""
    # Phase 1: Time Gating
    time_gate(universe)

    # Phase 2: Entropy
    apply_decay(universe)

    # Phase 3: Existence Transitions
    update_manifestation(universe)

    # Phase 4: Wave Propagation
    propagate_flux(universe)

    # Phase 5: Field Computation
    calculate_density(universe)

    # Phase 6: Force Accumulation
    accumulate_forces(universe)

    # Phase 7: Integration
    integrate(universe)

    # Phase 8: Movement
    move_particles(universe)

    # Phase 9: Collisions
    process_interactions(universe)

    # Phase 10: Transmutation
    transmute(universe)

    # Phase 11: Binding
    update_bindings(universe)

    # Phase 12: Increment
    universe.tick += 1

    return universe.tick
```

**Acceptance Criteria:**
- [ ] All 12 phases called in correct order
- [ ] Tick counter properly incremented
- [ ] Return value useful for external tracking

---

### Task 8: Fix Neighborhood Topology (1 hour)

**Files:** `binding.py`, `interactions.py`

**Issue:** Code claims Moore (26-connected) but uses Von Neumann (6-connected)

**Resolution:**
1. Define clear constants:
```python
# In config.py
MOORE_SHIFTS = list(itertools.product([-1, 0, 1], repeat=3))
MOORE_SHIFTS.remove((0, 0, 0))  # 26 neighbors

VON_NEUMANN_SHIFTS = [
    (-1, 0, 0), (1, 0, 0),
    (0, -1, 0), (0, 1, 0),
    (0, 0, -1), (0, 0, 1)
]  # 6 neighbors
```

2. Update `binding.py` to use `MOORE_SHIFTS` (26-connected) for triad detection
3. Document which operations use which topology

**Acceptance Criteria:**
- [ ] Binding uses Moore (26) for triad detection
- [ ] Annihilation uses Von Neumann (6) for adjacency
- [ ] Wave propagation uses Von Neumann (6) for Laplacian
- [ ] Each usage documented

---

### Task 9: Comprehensive Tests (1.5 hours)

**File:** `ternary_matrix/tests/test_physics.py` (new)

```python
def test_time_gating():
    """Verify time dilation for moving particles"""
    ...

def test_gravity_attraction():
    """Two masses should attract"""
    ...

def test_coulomb_repulsion():
    """Same charges should repel"""
    ...

def test_coulomb_attraction():
    """Opposite charges should attract"""
    ...

def test_speed_limit():
    """Particles cannot exceed c"""
    ...

def test_movement_boundary():
    """Toroidal boundary conditions"""
    ...

def test_transmutation():
    """High stress causes polarity flip"""
    ...

def test_full_cycle():
    """Run 1000 ticks, verify conservation laws"""
    ...
```

**Acceptance Criteria:**
- [ ] All force types have unit tests
- [ ] Integration tests for each phase
- [ ] Conservation laws verified over long runs
- [ ] Performance acceptable (target: 10 ticks/second for 64³ grid)

---

## File Summary

### Files to Create
- `ternary_matrix/physics/time_gating.py`
- `ternary_matrix/physics/integration.py`
- `ternary_matrix/physics/movement.py`
- `ternary_matrix/physics/transmutation.py`
- `ternary_matrix/tests/test_physics.py`

### Files to Modify
- `ternary_matrix/model/grid.py` (extend data structure)
- `ternary_matrix/physics/forces.py` (add 5 force types + curl)
- `ternary_matrix/physics/master_equation.py` (wire up all phases)
- `ternary_matrix/physics/binding.py` (fix Moore topology)
- `ternary_matrix/physics/interactions.py` (clarify topology)
- `ternary_matrix/config.py` (add WEAK_THRESHOLD, neighborhoods)

---

## Verification Checklist

After implementation:

- [ ] All 12 phases implemented
- [ ] Unit tests pass
- [ ] `pytest ternary_matrix/tests/` passes
- [ ] Conservation laws verified (flux, charge, momentum)
- [ ] No NaN or Inf values after 10,000 ticks
- [ ] Memory usage stable over long runs
- [ ] Performance acceptable for 64³ grid

---

## Dependencies

**Python packages (already in requirements.txt):**
- numpy
- scipy (optional, for optimized operations)

**No additional dependencies required.**

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Numerical instability | Medium | High | Clamp values, use float64 for accumulation |
| Performance issues | Medium | Medium | Profile, vectorize operations, consider Numba |
| Memory overflow | Low | High | Lazy allocation, sparse representation for large grids |
| Physics inconsistencies | Medium | Medium | Validate against CLAUDE.md specifications |

---

## Schedule

| Task | Estimated Time | Priority |
|------|----------------|----------|
| Task 1: Data Structure | 1 hour | Critical |
| Task 2: Time Gating | 1.5 hours | High |
| Task 3: Forces | 3 hours | Critical |
| Task 4: Integration | 1 hour | Critical |
| Task 5: Movement | 1.5 hours | Critical |
| Task 6: Transmutation | 1 hour | High |
| Task 7: Increment | 0.5 hour | Low |
| Task 8: Topology Fix | 1 hour | Medium |
| Task 9: Tests | 1.5 hours | High |
| **TOTAL** | **12 hours** | |

---

*Plan Version: 1.0*
*Created: 2026-01-24*
*For: FTD v1.0 Remediation Phase 2.4*

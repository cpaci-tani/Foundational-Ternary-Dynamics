# The Stellar Lifecycle on the Lattice

## From Mass to Black Hole Evaporation in Lattice Terms

**Date:** April 8, 2026
**Status:** [SELECTION] overall; individual claims tagged below
**Depends on:** SPEC_FTD.md, FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md, DERIV_BLACK_HOLE_PHYSICS.md, DERIV_DARK_SECTOR_DYNAMICS.md, DERIV_ELECTRON_MASS_MOTIVATION.md, SPEC_FTD_LAGRANGIAN.md

---

## Purpose

This document builds an intuitive, ground-up understanding of how the entire stellar lifecycle emerges from the FTD lattice. Each stage is told from two perspectives simultaneously:

- **Spatial picture** -- what you would see if you could watch individual voxels
- **Energy budget** -- where the G*^2 per-tick-per-DoF processing capacity goes

The goal is not to introduce new formalism but to make the existing FTD machinery *feel* inevitable. Every concept -- mass, gravity, fusion, collapse, horizons, evaporation -- should be understandable as a natural consequence of voxels processing flux under a finite tick budget.

---

## The One-Sentence Summary

A star is born when flux accumulates past the Jeans threshold, lives by balancing gravitational budget drain against fusion income, dies when the income stops, and either freezes into a degenerate remnant or collapses into a black hole -- a region where the lattice's tick budget has been entirely consumed -- which slowly repays its debt through quantum boundary fluctuations until nothing is left.

---

# Stage 1: Mass -- The Cost of Existing

## The Spatial Picture

Consider a single voxel on the Z^3 lattice. It has a flux field J in R^3 (a continuous vector encoding potential energy density) and a ternary state s in {-1, 0, +1}.

Most voxels are void: s = 0. The flux field ripples through them like waves on a calm ocean. These ripples cost nothing to maintain -- they are the void's natural dynamics, the lattice doing its thing.

Now suppose the flux density |J| at one voxel exceeds the manifestation threshold K_B = 0.511 MeV (the electron mass). Something qualitative happens: the voxel crystallizes from void into matter. The state field snaps from s = 0 to s = +1 or s = -1.

**This crystallized voxel IS a particle of mass m_e.** Not "represents" or "encodes" -- it IS. Mass is the lattice's way of recording that a voxel has been kicked above the void threshold. [AXIOM + THEOREM]

## The Energy Budget

Here is the key insight: **maintaining a manifested voxel costs energy every tick.**

The Born-Infeld core of the FTD Lagrangian is:

    L_BI = -K_B * sqrt(1 - v^2 - L^2)

where v = |dJ/dt|/K_B is the normalized velocity and L is the latency (gravitational) field. For a particle at rest in flat space (v = 0, L = 0):

    L_BI = -K_B

This is the rest energy. Every tick, the lattice must "spend" K_B of its processing budget to keep this voxel manifested. If the local budget drops below K_B (due to motion or gravity consuming the remainder), the manifestation becomes unstable.

**Mass = the per-tick cost of non-void existence.** [THEOREM, from the Lagrangian]

## Why Mass Resists Acceleration (Inertia)

To accelerate a manifested voxel, you must increase v. But the Lagrangian is:

    L_BI = -K_B * sqrt(1 - v^2 - L^2)

The effective momentum is p = dL/dv = K_B * v / sqrt(1 - v^2 - L^2). As v increases, the denominator shrinks. Each increment of velocity costs *more* budget than the last. This is the Born-Infeld nonlinearity -- the same mechanism that prevents the electric field from diverging at a point charge.

**Inertia = the nonlinear increase in budget cost per unit acceleration.** [THEOREM]

A heavier particle (larger K_B) costs more per tick at rest, and the nonlinear resistance scales with K_B. This is why heavier things are harder to accelerate: their baseline budget consumption is already high, leaving less room for velocity.

## Connection to Standard Physics

E = mc^2 is the statement that the rest mass m IS the rest energy cost per c^2 (the square of the lattice speed). In FTD terms: the energy locked into maintaining the manifested state IS the mass, and the speed of light c = 1/sqrt(3) is the lattice's maximum information propagation rate.

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Mass | Voxel with |J| > K_B crystallized to s = +/-1 | Per-tick cost: K_B |
| Inertia | Born-Infeld nonlinearity in v | Nonlinear budget increase with velocity |
| Rest energy | L_BI(v=0, L=0) = -K_B | Minimum cost of existence |

---

# Stage 2: Gravity -- The Latency Shadow

## The Spatial Picture

A manifested voxel doesn't just sit there costing budget. It casts a shadow.

The coupling term in the Lagrangian:

    L_coupling = -g_c * s * (div J)

means that every tick, the manifested particle (s = +/-1) injects new flux into its 6 face-neighbors via grad(s). This injected flux doesn't stay local -- it propagates outward at c = 1/sqrt(3), creating a halo of elevated flux density around the particle.

Near the particle (within 1 lattice hop), this flux is damped by Rayleigh dissipation. But beyond that -- in the "far field" -- the flux propagates losslessly through the vacuum. The result: every manifested particle is surrounded by a long-range flux envelope.

Now comes gravity. The flux density around a mass contributes to the **latency field** L. Latency measures how much of the local computational budget is consumed by the nearby mass's gravitational influence. The Poisson equation:

    laplacian(L) = 4*pi*G*rho

gives L^2 = r_s/r for a mass M at distance r, where r_s = 2GM/c^2 is the Schwarzschild radius. The availability factor is then f(r) = 1 - L^2 = 1 - r_s/r. [THEOREM]

**The latency shadow is the region around a mass where voxels tick slower because the mass's flux halo is consuming their processing bandwidth.** [SELECTION for "consuming bandwidth" interpretation; THEOREM for the metric]

## The Energy Budget

The availability factor is:

    f(r) = 1 - L^2 = 1 - r_s/r

At each voxel, the effective tick budget is not the full G*^2 per DoF -- it's G*^2 * f(r). Near the mass, f < 1, so voxels process less energy per tick. Further away, f -> 1, and voxels tick at full speed.

**Gravitational force = the gradient of the availability factor.** [THEOREM]

    F_grav = G_N * grad(rho_bar)

where G_N = 1/(b_3 + N_c)^2 = 1/100 = 0.01 on the lattice. Objects drift toward regions where the gradient steepens -- not because they're "attracted," but because the budget landscape funnels them downhill.

Think of it this way: a freely falling body follows the path that maximizes its proper time (geodesic). In FTD, proper time IS the number of ticks you actually process. The body drifts toward the mass because that's the direction where the budget gradient carries it.

## Time Dilation = Tick Rate Reduction

The proper time formula:

    dtau/dt = sqrt(f(r) - v^2/f(r))

For a stationary observer (v = 0):

    dtau/dt = sqrt(f(r)) = sqrt(1 - r_s/r)

A clock near a mass ticks slower because the voxels it's made of have less budget available per universal tick. This is not a metaphor -- it IS the Schwarzschild time dilation, exact. [THEOREM]

## What Gravity Is NOT

Gravity is not spacetime curvature. The lattice Z^3 is flat and stays flat. What "curves" is the temporal experience of embedded observers. Space doesn't bend; clocks slow down. The spatial stretching in the Schwarzschild metric (dr^2/f(r)) is a *consequence* of using slower clocks to measure distances. The ruler itself is a temporal process, and temporal processes are slowed by the latency shadow. [SELECTION]

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Latency field L | Flux halo from manifested particles | Fraction of budget consumed by nearby mass |
| Availability f | 1 - L^2 at each voxel | Remaining budget for dynamics |
| Time dilation | dtau/dt = sqrt(f) | Fewer ticks processed near mass |
| Gravitational force | grad(rho_bar) scaled by G_N | Budget gradient pushes things downhill |
| G_N = 0.01 | 1/(b_3 + N_c)^2 = 1/100 | Coupling strength of the budget shadow |

---

# Stage 3: Cloud -- The Gathering

## The Spatial Picture

Now scale up. Instead of one manifested voxel, consider a diffuse region of elevated flux density -- a molecular cloud in lattice terms. The region contains many voxels with |J| fluctuating near or above K_B, with some manifested particles scattered throughout.

Each manifested particle casts its latency shadow (Stage 2). The shadows overlap. Where they overlap, the combined latency is higher, which means:

1. Local tick rates are lower (more budget consumed)
2. The flux density gradient steepens (gradients point inward)
3. Surrounding flux drifts inward along these gradients

This is positive feedback: more density -> stronger gradient -> more accumulation -> more density.

**The cloud is self-gravitating.** [EMERGENT]

## The Energy Budget: Jeans Instability

But there's a competing effect. The manifested particles have kinetic energy -- they're moving, vibrating, jostling. This thermal motion tends to spread the cloud out, countering gravitational contraction.

The Jeans criterion asks: which wins?

- **Gravitational PE release** from contraction: ~G_N * M^2 / R (grows as M^2)
- **Thermal KE** resisting contraction: ~N * k_T (grows as N = M/m)

For small clouds, thermal energy wins -- the cloud is stable. For large enough clouds, gravitational PE dominates:

    M > M_Jeans ~ c_s^3 / (G_N^(3/2) * rho^(1/2))

where c_s is the sound speed (information propagation speed in the medium, bounded above by c = 1/sqrt(3)).

**Jeans instability = the budget tipping point.** [THEOREM for the criterion; EMERGENT for the actual collapse dynamics]

Below Jeans mass: thermal budget redistribution keeps up with gravitational drain. The cloud puffs and breathes but doesn't collapse.

Above Jeans mass: gravitational budget drain overwhelms thermal redistribution. The cloud contracts. And as it contracts, density rises, making the drain steeper, making contraction faster. Runaway.

## The Spatial Cascade

As the cloud contracts:

1. Voxel density increases (more manifested particles per volume)
2. Flux field amplitude rises everywhere inside the cloud
3. Latency shadows merge into a continuous latency basin
4. The basin deepens, pulling in even more flux from the surroundings
5. The outer envelope falls inward at velocities approaching c_s

This is not gentle. Once the Jeans threshold is crossed, the collapse accelerates until something stops it.

## What Stops It?

Two possible brakes:

1. **Radiation pressure** -- as the core heats up (kinetic energy increases from compression), manifested voxels emit flux outward. If this outward flux pressure balances gravitational infall, the cloud stabilizes.

2. **Degeneracy pressure** -- the Pauli exclusion principle on the lattice: you cannot place two identical fermion states at the same site. As density increases, the available states fill up, creating an outward quantum pressure. (More on this in Stage 6.)

If radiation pressure achieves balance, you get a **star** (Stage 5). If it can't, you skip ahead to collapse.

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Molecular cloud | Diffuse region of elevated |J| near K_B | Low-density, budget-rich environment |
| Self-gravitation | Overlapping latency shadows create inward gradient | Budget drain exceeds thermal redistribution |
| Jeans instability | Gradient steepening faster than thermal smoothing | The budget tipping point |
| Collapse | Runaway density increase, deepening latency basin | Accelerating budget concentration |

---

# Stage 4: Dark Matter Scaffolding -- The Invisible Architecture

## The Spatial Picture

Before we get to the star, there's a crucial ingredient we haven't discussed: dark matter.

In FTD, dark matter is not a mysterious new particle. It is the **lossless far-field flux** that every manifested particle emits. Recall from Stage 2:

- The coupling term injects flux into the 6 face-neighbors every tick
- Rayleigh dissipation damps flux **only within 1 lattice hop** of a manifested particle
- Beyond 1 hop, flux propagates losslessly through the vacuum

This means every manifested particle is surrounded by a self-field halo extending ~15 voxels (with a 1% boundary at ~23 voxels). This halo:

- **IS gravitationally active** -- it contributes to the latency field L
- **IS NOT electromagnetically active** -- it doesn't change ternary states (no s -> +/-1 transitions)
- **Cannot be seen** by EM observations -- it's invisible to photon-mediated detection

**Dark matter = the far-field flux halo of ordinary matter.** [SELECTION]

## The Energy Budget

The dark halo is *energetically free to maintain*. Once flux has propagated beyond the 1-hop damping radius, it costs no additional budget per tick -- it just propagates at c through the lossless vacuum. The energy was already spent at injection; the halo is the afterglow.

But the halo's gravitational contribution is real. It deepens the latency basin, making the combined (visible + dark) gravitational well significantly deeper than visible matter alone would produce.

## Why Dark Matter Matters for Stars

Dark matter halos provide the **gravitational scaffolding** for structure formation:

1. Early in the universe, dark halos accumulate first (they don't radiate, so they can't lose energy and bounce back)
2. These halos create deep latency basins -- potential wells
3. Baryonic (visible) matter falls into these pre-existing wells
4. The combined density crosses the Jeans threshold faster than visible matter alone could manage

**Without the dark scaffolding, many molecular clouds would never reach Jeans mass.** The dark halo tips the budget balance, enabling collapse where thermal pressure alone would have prevented it.

## The Numbers

From the dark sector dynamics (DERIV_DARK_SECTOR_DYNAMICS.md):

- Injection rate per particle: ~alpha per tick (O(0.007) * K_B^2 of flux energy)
- Halo effective radius: ~15 voxels (r_eff)
- 1% boundary: ~23 voxels
- Energy density profile: falls off as ~1/r^2 beyond the core

The ratio of dark-to-visible energy in the halo gives the universal dark-to-visible matter ratio when summed cosmologically.

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Dark matter halo | Lossless far-field flux around each particle | Free to maintain (energy already spent at injection) |
| Scaffolding | Dark halos create potential wells | Budget wells that seed baryonic collapse |
| Invisibility | No ternary state changes in the halo | No EM budget interactions |
| Structure formation | Dark wells + baryonic infall = Jeans threshold | Budget shortcut to gravitational collapse |

---

# Stage 5: Star -- The Balanced Budget

## The Spatial Picture

The cloud has collapsed past the Jeans threshold. The core is compressing. Density rises. Temperature (average kinetic energy per voxel) increases as gravitational PE converts to KE.

At some critical density and temperature, a new process ignites: **fusion**.

### What Is Fusion on the Lattice? [CONJECTURE]

In standard physics, fusion is the merging of atomic nuclei -- overcoming Coulomb repulsion to reach the strong-force-binding regime. On the FTD lattice, this translates to:

**Fusion = high-energy ternary state rearrangements that release stored flux energy.**

More specifically: when multiple manifested voxels (+1 and -1 states) are compressed to high enough density, their flux fields overlap strongly. The system finds lower-energy configurations by rearranging which voxels are manifested and how their flux is distributed. The difference in energy between the initial compressed state and the final rearranged state is released as outward-propagating flux -- radiation.

Think of it like this:
- Four hydrogen voxels (each costing K_B to maintain) are squeezed together
- The system discovers that one helium configuration (different flux pattern, fewer effective voxels) costs less total budget
- The budget difference is released as kinetic flux energy radiating outward

The released energy per fusion event is small compared to the rest mass: ~0.7% for hydrogen -> helium. But there are *many* fusion events per tick in a dense core.

### Hydrostatic Equilibrium

The star achieves stability when:

    Inward gravitational force = Outward radiation + thermal pressure

At every shell radius inside the star, this balance holds. The core fuses, releasing energy. That energy propagates outward as flux, pushing against the infalling envelope. The envelope's weight presses inward.

**A stable star is a lattice region in perfect energy budget balance.** [EMERGENT]

## The Energy Budget

Here's the star's balance sheet, per tick:

| Income | Expense |
|--------|---------|
| Fusion energy release in core | Gravitational budget drain (maintaining the latency basin) |
| | Radiation losses from the surface (flux escaping to infinity) |
| | Thermal maintenance of the envelope |

In equilibrium: Income = Expenses. The star's luminosity (surface radiation loss) equals its core fusion rate minus the gravitational maintenance cost.

**The main sequence IS the balanced budget era.** A star on the main sequence has found its equilibrium: it fuses just enough fuel to counteract its own weight. If fusion increases slightly, the star expands (reducing core density and cooling fusion). If fusion decreases, the star contracts (increasing density and heating fusion). This self-regulation is inherent in the budget coupling between gravity and fusion.

## Stellar Structure on the Lattice

The star has concentric zones, each with a different budget profile:

| Zone | Density | Temperature | Budget Status |
|------|---------|-------------|---------------|
| Core | Maximum | Maximum | Fusion income = gravitational drain |
| Radiative zone | High | Decreasing outward | Flux transport (photon diffusion) |
| Convective zone | Moderate | Decreasing | Bulk flux transport (convection cells) |
| Photosphere | Low | ~surface temp | Budget leak to infinity (luminosity) |

The entire structure is held together by the budget balance at each shell.

## Connection to Standard Physics

- **Luminosity**: L = energy radiated per universal tick from the photosphere
- **Main sequence lifetime**: t_ms ~ M * efficiency / L (total fuel budget / burn rate)
- **Hertzsprung-Russell diagram**: plots luminosity vs surface temperature -- each point is a different budget equilibrium state
- **Mass-luminosity relation**: L ~ M^3.5 (heavier stars have steeper latency basins, requiring more fusion to counterbalance)

---

# Stage 6: Death -- Budget Deficit

## The Spatial Picture

The star has been fusing hydrogen to helium for millions to billions of ticks. Eventually, the hydrogen in the core runs out.

What happens next depends on the star's mass:

### Low Mass (< ~8 solar masses): Gentle Death

1. **Core hydrogen exhausted**: No more fusion income from the primary fuel
2. **Core contracts**: Without fusion pressure, gravity wins temporarily
3. **Shell burning**: Hydrogen in a shell around the core ignites (the contraction heats it)
4. **Envelope expands**: The extra shell energy pushes the outer layers outward -- red giant
5. **Helium flash/burning**: If the core gets hot enough, helium fuses to carbon (temporary reprieve)
6. **Final exhaustion**: Eventually all fusible fuel is consumed
7. **Envelope ejected**: Outer layers drift off (planetary nebula)
8. **Remnant**: A carbon-oxygen core supported by electron degeneracy pressure -- white dwarf

### High Mass (> ~8 solar masses): Violent Death

1. **Fuel ladder**: The star burns through H -> He -> C -> O -> Ne -> Mg -> Si -> Fe
2. **Iron trap**: Iron (Fe-56) is the minimum of the nuclear binding energy curve. Fusing iron *costs* energy rather than releasing it. The income stream doesn't just stop -- it reverses.
3. **Core collapse**: With zero income and a massive gravitational expense, the core free-falls
4. **Bounce or not**: If the core is below ~3 solar masses, neutron degeneracy pressure halts collapse (neutron star). If above, nothing stops it (black hole).
5. **Supernova**: The bounce (or collapse energy release) blows off the outer layers in an explosion.

## The Energy Budget: What "Fuel Exhaustion" Really Means

Each fusion stage is a budget equilibrium at a higher temperature and density:

| Fuel | Income per event | Core temp needed | Duration |
|------|-----------------|-----------------|----------|
| Hydrogen | 0.7% of rest mass | ~15 million K | Millions of years |
| Helium | 0.07% | ~100 million K | Thousands of years |
| Carbon | 0.05% | ~500 million K | Centuries |
| Silicon | 0.003% | ~3 billion K | Days |
| Iron | **NEGATIVE** | N/A | Budget bankruptcy |

Each stage is shorter because:
1. Less energy per event = need more events to balance gravity
2. Higher temperature = higher radiation losses (budget leak accelerates)
3. The budget deficit deepens with each transition

**Iron is the budget wall.** When the core reaches iron, fusion income goes negative. The star is spending more than it earns AND losing budget to radiation AND losing budget to neutrino emission. Triple deficit. Collapse is immediate.

## Degeneracy Pressure on the Lattice [SELECTION]

What holds up a white dwarf or neutron star against gravity when fusion has stopped?

On the FTD lattice, the Pauli exclusion principle emerges from the ternary state constraint combined with fermion statistics (pi_1(SO(3)) = Z_2, the topological origin of spin-1/2 in FTD). Two identical fermions cannot occupy the same voxel state.

As density increases and voxels are packed tighter, the available quantum states fill up. Each electron (or neutron) must occupy a higher-energy state than the one below it. This creates an outward pressure -- not from thermal motion, but from the *exclusion of lower states*.

**Degeneracy pressure = the lattice running out of distinct low-energy configurations.** [SELECTION]

In budget terms: the lattice literally cannot pack more particles without promoting them to higher-energy (higher budget-cost) states. The budget cost of adding one more particle to the degenerate core increases steeply with density.

### The Chandrasekhar Limit

There's a mass above which electron degeneracy pressure fails: ~1.4 solar masses. At this point, electrons are pushed to relativistic speeds (v -> c), where the degeneracy pressure scaling changes from p ~ rho^(5/3) to p ~ rho^(4/3), which can't support the weight.

In FTD terms: when the electron velocity approaches c = 1/sqrt(3), the Born-Infeld nonlinearity saturates -- the budget cost of maintaining the degenerate state approaches the total available budget. The state becomes unsustainable.

**The Chandrasekhar limit = the mass where degeneracy budget meets total available budget.** [SELECTION]

Whether this limit can be expressed purely in terms of alpha, N_c, and lattice integers is an [OPEN] question.

---

# Stage 7: Black Hole Formation -- Bandwidth Bankruptcy

## The Spatial Picture

The iron core of a massive star (or a white dwarf pushed over the Chandrasekhar limit by accretion) has begun to collapse. Neither fusion nor degeneracy pressure can halt it.

What happens at the lattice level:

1. **Density surge**: Manifested voxels pack tighter and tighter. The flux field amplitude at each voxel increases steeply.

2. **Latency climbs**: As mass density rises, the latency field L(r) grows. The availability factor f = 1 - L^2 drops. Voxels near the center process fewer ticks.

3. **The critical surface**: At some radius r = r_s = 2GM/c^2, the latency reaches L = 1 and f = 0. This is the **horizon**.

4. **Escape velocity = c**: At the horizon, the escape velocity equals the speed of light. No signal -- no flux, no state change, no information of any kind -- can propagate outward from inside this surface.

5. **Formation**: The horizon doesn't appear at the center and expand outward. It appears at the radius where the accumulated enclosed mass first satisfies r = 2GM/c^2, and then it grows as more matter falls in.

## The Energy Budget: Total Bankruptcy

At the horizon, f = 0 means **zero budget remains for dynamics**. Inside the horizon:

- No ticks can execute (zero processing capacity)
- No information can propagate outward (outward speed limit = f * c = 0)
- Flux is trapped: once inside, it stays inside

**The horizon is the surface of budget bankruptcy.** [THEOREM]

Nothing exotic happens *at* the horizon locally. A freely falling observer crossing the horizon notices nothing special -- from their perspective, they're just following the budget gradient. But from outside, the horizon is an absolute information boundary.

The key formula:

    dtau/dt = sqrt(f - v^2/f) -> 0 as f -> 0

Time freezes at the horizon. An external observer watching something fall in sees it asymptotically approach the horizon, redshift toward infinity, and fade from view. The infalling object reaches the horizon in finite proper time, but infinite external time.

## The Engine Connection

In the FTD engine simulation, black hole formation is detected by checking:

    v_escape = sqrt(2 * G_N * M_enclosed / r) > C_SPEED = 1/sqrt(3)

When this threshold is crossed for a sufficient enclosed mass (M_enc > 50 in lattice units), the densest body converts to BLACK_HOLE type. This is the discrete analog of the continuous horizon formation described above.

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Core collapse | Density surge, L -> 1, f -> 0 | Budget drains faster than anything replenishes |
| Horizon | Surface where f = 0 | The budget = 0 boundary |
| Escape velocity = c | v_esc = sqrt(2GM/r) >= 1/sqrt(3) | No budget left for outward propagation |
| Time freeze | dtau/dt = sqrt(f) -> 0 | Zero ticks processed at the horizon |

---

# Stage 8: The Black Hole Interior -- The Frozen Crystal

## The Central Question

What's inside a black hole?

In general relativity, the interior contains a singularity -- a point of infinite density where the equations break down. GR admits this is a failure: the theory cannot describe what happens at the singularity.

**FTD has no singularity.** [THEOREM, from the lattice UV cutoff]

## The Spatial Picture

Inside the horizon (r < r_s), the lattice continues. Z^3 is a uniform cubic lattice -- it doesn't end or tear or crumple. The voxels are still there, each with a ternary state and a flux field.

But the availability factor f < 0 inside the horizon (from the Schwarzschild solution f = 1 - r_s/r < 0 when r < r_s). What does negative f mean?

In GR, f < 0 means the roles of space and time interchange -- the radial coordinate becomes timelike, and the time coordinate becomes spacelike. Infall is no longer a choice; it's as inevitable as the passage of time.

In FTD: **f <= 0 means the tick budget is fully consumed.** The lattice sites inside the horizon have no processing capacity. They are frozen.

### The Frozen State

Picture the interior as a crystal of frozen information:

1. **Each interior voxel retains its last state.** The ternary values s and flux vectors J at the moment the horizon swept over them are preserved. They don't update because there are no ticks to update during.

2. **The density is bounded.** The maximum possible density is one ternary state per voxel -- either all +1 or all -1. This is the lattice's UV cutoff in action. There is no infinity, no divergence, no singularity. Just a maximally packed lattice.

3. **The lattice spacing provides the minimum length.** No structure can exist below the lattice scale. The "singularity" of GR is regularized into a finite, maximally dense core.

### Maximum Density Configuration [CONJECTURE]

What does the interior look like at maximum compression? Several possibilities:

**Option A: Uniform state crystal.** All interior voxels are s = +1 (or -1). The flux field is maximal and uniform. This is the simplest maximum-density configuration but may not be stable against domain formation.

**Option B: Antiferromagnetic crystal.** Alternating +1 and -1 states in a checkerboard pattern, minimizing nearest-neighbor energy. The flux field forms a standing-wave pattern at the lattice scale. This might be energetically preferred if the coupling term favors alternating states.

**Option C: Disordered glass.** A random configuration of +1 and -1 states, frozen at the moment of collapse. The interior preserves the *history* of what fell in, encoded in the disorder pattern.

Option C is most consistent with information preservation and the unitary evolution guarantee. The frozen crystal is not a featureless blob -- it's a *record* of everything that fell in, written in the language of ternary states.

## The Energy Budget: Zero Processing

Inside the horizon:

    Budget per tick = G*^2 * f(r) = G*^2 * (1 - r_s/r) <= 0

Zero or negative budget means:
- No state updates possible
- No flux propagation
- No interactions between neighboring voxels
- No time passes (dtau = 0)

**The interior is frozen in its last moment.** [SELECTION]

This is profoundly different from the GR picture. GR says the interior is dynamic -- matter falls to the center in finite proper time and hits the singularity. FTD says: the "falling to the center" description assumes continuous spacetime. On the discrete lattice, once the horizon forms, the interior voxels immediately (from the external perspective) or asymptotically (from the infalling perspective) lose their processing capacity. There is no "afterward" inside the horizon -- not because something dramatic happens, but because "afterward" requires ticks, and there are no ticks.

## The Brillouin Zone Argument [THEOREM]

Why GR's singularity doesn't arise:

In continuum physics, gravitational collapse produces arbitrarily high densities and momenta. A photon traced back from the horizon blue-shifts without bound: p ~ p_inf * exp(t/r_s). After time t ~ r_s * ln(M/M_P), the momentum exceeds the Planck scale -- the trans-Planckian problem.

On the FTD lattice:

    |k_mu| <= pi   (Brillouin zone boundary)

**All momenta are bounded.** A blue-shifting mode hits the BZ edge at k = pi and reflects (the lattice dispersion relation saturates to a standing wave). No mode can ever exceed the lattice cutoff. Trans-Planckian physics literally doesn't exist.

This means:
- No infinite densities (max = 1 state per voxel)
- No infinite momenta (max = pi per lattice spacing)
- No breakdown of the theory (the lattice is always well-defined)
- No need for quantum gravity to "save" the theory at high energies

**The lattice IS the UV completion.** [THEOREM]

## Information Preservation [THEOREM]

The lattice evolution rule is deterministic and invertible (proven in DERIV_BLACK_HOLE_PHYSICS.md S4). The time evolution operator U(t) satisfies U^dag * U = 1. Von Neumann entropy is conserved.

This means:
- The frozen interior state is uniquely determined by the infalling matter
- Different infalling configurations produce different interior states
- The information is preserved -- just inaccessible from outside
- When the black hole evaporates (Stage 9), the information is eventually released

**There is no information paradox.** The paradox arises only in theories where unitarity is assumed but the evolution isn't manifestly invertible. On the lattice, invertibility is built in. [THEOREM]

## Growth: How the Frozen Crystal Expands

As more matter falls into the black hole:

1. The enclosed mass M increases
2. The Schwarzschild radius r_s = 2GM/c^2 grows
3. The horizon expands outward
4. Previously f > 0 voxels at the boundary now have f <= 0
5. These boundary voxels freeze, extending the crystal

**The black hole grows at its surface**, not at its center. The center was frozen at formation and stays frozen. New matter freezes as it reaches the expanding horizon.

In budget terms: as M grows, the budget bankruptcy zone expands. More voxels are absorbed into the zero-budget region. The horizon is the moving bankruptcy frontier.

## Time Inside the Black Hole

From the exterior perspective: the interior is frozen at the moment of horizon formation. No time passes there. Ever.

From the infalling perspective: the freely falling observer processes fewer and fewer ticks as they approach the horizon. Their proper time slows asymptotically. They never experience "hitting the singularity" because:
1. The singularity doesn't exist (lattice UV cutoff)
2. The ticks stop before they get there (f -> 0)

This resolves the "firewall paradox" -- the question of whether an infalling observer encounters high-energy radiation at the horizon. Answer: no. The horizon is smooth (nothing special happens locally), and the interior doesn't need a firewall because the lattice provides natural UV regulation. [SELECTION for the resolution; THEOREM that the lattice bounds momenta]

## Open Questions [OPEN]

1. **Compression history**: Do interior voxels retain their exact pre-collapse flux configuration, or does the collapse process adiabatically compress them before freezing? (Depends on the dynamics during the sub-r_s infall epoch)

2. **Domain structure**: Does the frozen crystal have internal structure -- domain walls between +1 and -1 regions, lattice defects, topological features? These would encode the angular momentum and charge of the infalling matter.

3. **Maximum BH mass**: Is there a lattice-scale limit on black hole mass? (Probably not -- r_s can grow without bound. But the horizon area might be constrained by the total lattice size in a finite simulation.)

4. **Kerr and Reissner-Nordstrom**: How does rotation and charge modify the frozen crystal structure? (See DERIV_LATTICE_BLACK_HOLES.md for the metric modifications.)

---

# Stage 9: Evaporation -- The Slow Repayment

## The Spatial Picture

The black hole sits in the lattice, a frozen crystal surrounded by the f = 0 horizon. Outside the horizon, the lattice is alive -- voxels tick normally, flux propagates, the universe continues.

At the horizon boundary, something subtle happens.

### Vacuum Fluctuations at the Boundary

Even in the "vacuum" outside the horizon, the flux field is not zero. Quantum fluctuations -- tiny, random excitations of the flux field -- occur everywhere in the lattice. Normally these fluctuations are virtual: a +1/-1 pair spontaneously manifests and immediately annihilates, costing and returning budget within a single tick.

But at the horizon, the geometry is special. A fluctuation that creates a pair straddling the boundary can split:

1. One member of the pair falls inside (into the f <= 0 zone)
2. The other member escapes outward (into the f > 0 zone)

The ingoing member is trapped. The outgoing member carries real energy away from the black hole. **This is Hawking radiation.** [THEOREM]

## The Energy Budget: Boundary Accounting Error

From the budget perspective:

- The pair creation "borrows" energy from the vacuum (standard quantum fluctuation)
- Normally, the pair annihilates and returns the energy (net: zero)
- At the horizon, the pair splits. One member crosses into budget bankruptcy (f <= 0)
- The ingoing member can't return its energy -- it's trapped
- The outgoing member carries real positive energy to infinity
- The energy must come from somewhere: it comes from the black hole's mass

**Each Hawking photon reduces the BH mass by a tiny amount.** The horizon contracts slightly. A thin layer of previously frozen voxels at the outermost boundary of the crystal now find themselves at f > 0 -- they unfreeze and can tick again.

## The Hawking Temperature [THEOREM]

The rate of this boundary leakage depends on the surface gravity at the horizon:

    T_H = c^3 / (8*pi*G*M*k_B) = hbar*kappa / (2*pi*c*k_B)

Key properties:
- **Smaller BHs are hotter**: T_H ~ 1/M. Less mass = smaller horizon = steeper gradient = more pair-splitting
- **Solar-mass BH**: T_H ~ 6 * 10^-8 K (essentially zero -- colder than the CMB)
- **Planck-mass BH**: T_H ~ 10^31 K (the lattice-scale maximum temperature)

In budget terms: T_H measures the rate of boundary accounting errors per unit horizon area. A smaller horizon has more "edge per volume" (higher curvature), so more fluctuations straddle the boundary per unit time.

## The Evaporation Timeline

For a black hole of mass M, the evaporation time is:

    t_evap ~ G^2 * M^3 / (hbar * c^4)

- **Solar-mass BH**: t_evap ~ 10^67 years (far longer than the age of the universe)
- **Mountain-mass BH** (~10^12 kg): t_evap ~ age of the universe (these would be evaporating NOW)
- **Planck-mass BH**: t_evap ~ Planck time (essentially instant)

## The Page Curve and Information Recovery [THEOREM + SELECTION]

How does the information from the frozen crystal get back out?

The Hawking radiation appears thermal (random) to any local detector. But the lattice evolution is deterministic and unitary. This means the radiation MUST carry subtle correlations that encode the interior state -- the frozen crystal's exact configuration.

The **Page curve** describes how this works:

1. **Early radiation** (first half of evaporation): Entanglement between the radiation and the remaining BH increases. Each emitted photon is entangled with the interior. The radiation looks thermal.

2. **Page time** (halfway through evaporation): The radiation's entanglement entropy peaks. From this point on, each new photon carries more information OUT than it adds in entanglement.

3. **Late radiation** (second half): The entanglement decreases. The radiation becomes increasingly non-thermal, encoding the interior state through multi-particle correlations.

4. **Final evaporation**: The last photons carry the remaining information. The frozen crystal is completely unfrozen and radiated away. **All information is returned.**

In FTD, this works because the lattice evolution is invertible. If you had the complete Hawking radiation (every emitted flux quantum with its exact state), you could in principle reconstruct the entire interior configuration. The information was never destroyed -- it was temporarily frozen and then slowly leaked out through boundary effects.

## The Endgame: The Final Pop [CONJECTURE]

As M -> 0, T_H -> infinity. The final stages of evaporation are explosive:

- The horizon shrinks rapidly
- The temperature climbs steeply
- The radiation intensity increases
- The last frozen voxels unfreeze in a burst of energy

What does the final moment look like? When the BH mass approaches the Planck mass (~1 lattice unit), the horizon is only a few voxels across. At this point:

- The distinction between "inside" and "outside" breaks down (the horizon is comparable to the lattice spacing)
- The remaining frozen crystal is just a few voxels
- These voxels unfreeze and radiate their stored flux in a single burst

**The black hole ends as it began: as ordinary lattice excitations.** The frozen crystal completely dissolves back into propagating flux, and the lattice returns to its normal dynamics. [CONJECTURE]

The final burst energy is of order M_P * c^2 -- the Planck energy -- which is the natural energy scale of a single lattice site. The black hole evaporation endpoint is not exotic; it's the lattice returning to its ground state.

## What We Have So Far

| Concept | Lattice meaning | Budget meaning |
|---------|----------------|----------------|
| Hawking radiation | Vacuum pair-splitting at the horizon boundary | Boundary accounting errors that cost BH mass |
| Temperature T_H | Rate of boundary leakage per unit area | Budget leak rate: T_H ~ 1/M |
| Evaporation | Horizon slowly contracting, crystal unfreezing at boundary | Budget gradually returning to external lattice |
| Page curve | Correlations in radiation encoding the interior | Information budget slowly repaid |
| Final pop | Last frozen voxels dissolve | Budget fully returned |

---

# The Complete Picture

## The Lifecycle in One Table

| Stage | Spatial | Budget | Duration |
|-------|---------|--------|----------|
| 1. Mass | Voxel crystallizes: s -> +/-1 | K_B per tick to maintain | Fundamental |
| 2. Gravity | Latency shadow: L ~ M/r | f = 1 - L^2 reduces neighbor budget | Fundamental |
| 3. Cloud | Overlapping shadows create inward gradient | Jeans: gravity drain > thermal redistribution | ~10^6 years |
| 4. Dark scaffolding | Lossless far-field flux halos | Free gravitational wells | ~10^8 years |
| 5. Star | Fusion balances gravity | Income = expenses | ~10^6 - 10^10 years |
| 6. Death | Fuel exhausted, iron wall | Triple deficit: no income, gravity + radiation drain | Days to billions of years |
| 7. BH formation | f -> 0 at core, horizon forms | Total budget bankruptcy | Milliseconds |
| 8. Interior | Frozen crystal, no singularity | Zero budget, information preserved | Indefinite |
| 9. Evaporation | Boundary pair-splitting, horizon shrinks | Slow budget repayment via T_H | ~10^67 years |

## The Ontological Claim

**Everything in this lifecycle is the same lattice doing the same thing: processing flux under a finite tick budget.**

Mass, gravity, stars, black holes, Hawking radiation -- none of these are separate phenomena grafted onto the theory. They are all manifestations of the single dynamics: Z^3 lattice sites with ternary states evolving via the Born-Infeld action under the budget constraint f >= 0, with the budget set by G*^2 per DoF per tick.

The lattice doesn't know about "stars" or "black holes." It just processes voxels. Stars are what budget balance looks like from far away. Black holes are what budget bankruptcy looks like. Hawking radiation is what budget recovery looks like. The concepts are ours; the dynamics are the lattice's.

---

## Epistemic Summary

| Claim | Tag | Notes |
|-------|-----|-------|
| Mass = K_B maintenance cost | [THEOREM] | From Born-Infeld Lagrangian |
| Inertia = BI nonlinearity | [THEOREM] | Follows from L_BI |
| Gravity = tick-rate variation | [THEOREM] for metric, [SELECTION] for interpretation | Schwarzschild exact |
| Jeans instability | [THEOREM] for criterion, [EMERGENT] for dynamics | Standard physics on lattice |
| Dark matter = far-field flux | [SELECTION] | From selective damping mechanism |
| Fusion = high-energy state rearrangement | [CONJECTURE] | Detailed mechanism not derived |
| Hydrostatic equilibrium | [EMERGENT] | Budget balance, not proven from action |
| Degeneracy pressure from exclusion | [SELECTION] | Pauli from pi_1(SO(3)), pressure is standard |
| Chandrasekhar limit from lattice constants | [OPEN] | Not yet derived in FTD terms |
| Horizon at f = 0 | [THEOREM] | From Born-Infeld + Schwarzschild |
| No singularity (BZ cutoff) | [THEOREM] | Lattice momenta bounded by pi |
| Information preservation | [THEOREM] | Deterministic invertible evolution |
| Frozen crystal interior | [SELECTION] | Consistent with f <= 0 but not uniquely determined |
| Hawking temperature | [THEOREM] | Euclidean periodicity, exact |
| Page curve | [THEOREM] | From lattice unitarity |
| Evaporation endpoint | [CONJECTURE] | Final-pop scenario not simulated |

---

## Simulation Implications

Each stage suggests what the engine simulation should show:

1. **Mass**: Flux density visualization, K_B threshold highlighting
2. **Gravity**: Latency field contour overlay, budget heatmap
3. **Cloud**: Jeans instability animation, positive feedback loop visible
4. **Dark matter**: Toggle for far-field halo visualization
5. **Star**: Energy balance overlay (income vs expense at each shell)
6. **Death**: Fuel depletion meter, sequential burning stages, iron-core flash
7. **BH formation**: Horizon surface appearing, interior darkening
8. **Interior**: Frozen voxel crystal visible through cutaway, domain structure
9. **Evaporation**: Horizon shrinking, temperature glow increasing, final burst

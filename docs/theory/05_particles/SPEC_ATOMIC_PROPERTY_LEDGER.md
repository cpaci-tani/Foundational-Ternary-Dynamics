# SPEC_ATOMIC_PROPERTY_LEDGER.md

**Title:** FTD Atomic Property Ledger and Scale Mapping
**Status:** `[ACTIVE SPEC]`
**Depends on:** `SPEC_SCALE_CONTEXT_READOUT.md`
**Engine implementation:** `engine/include/ftd/atomic_closure_context.h`,
`engine/include/ftd/atom_engine.h`, `engine/web/js/atomic-props.js`

---

## 0. Executive Statement

An individual atom is not defined solely by its atomic number ($Z$) or its "size." In FTD, an atom is the canonical example of a **finite self-confined cloud**.

The correct operational definition of an atom is a multi-layered contextual structure:
$$ \text{atom} = \text{nuclear source} + \text{electron cloud} + \text{shell boundary} + \text{state/readout context} $$

This maps directly onto the scale-context variables from the active scale spec ($R_{\rm cloud}$, $\delta_{\rm shell}$, $\xi$, $\tau_{\rm cloud}$). The proper atomic identity is not $Z$ alone, but a comprehensive contextual vector:
$$ Z + \text{isotope} + \text{charge state} + \text{electron configuration} + \text{scale context} + \text{environment} $$

---

## 1. The Three-Layer Atomic Architecture

An atom exhibits a strict structural hierarchy mapping chemistry language to FTD language:

$$ \text{source core} \rightarrow \text{confined cloud} \rightarrow \text{active boundary} $$
$$ \text{nucleus} \rightarrow \text{electron shell structure} \rightarrow \text{valence behavior} $$

1. **Nucleus:** Acts as the central source load / return-flux anchor. It sets the inward return force that confines the electron cloud.
2. **Electron Shell:** The finite cloud closure layer.
3. **Valence Shell:** The active boundary shell. Chemistry is fundamentally the physics of this boundary.

---

## 2. FTD Atomic Property Ledger

This ledger translates standard atomic and chemical properties into the native FTD ontology.

| Standard atomic property | FTD interpretation |
| :--- | :--- |
| **$Z$ (Atomic Number)** | Nuclear source load |
| **$N$ (Neutron Number)** | Nuclear stability/modulation load |
| **$N_e$ (Electron count)** | Cloud occupancy |
| **Electron configuration** | Finite shell-closure pattern |
| **Valence electrons** | Active boundary degrees of freedom |
| **Atomic radius** | Contextual cloud-scale readout, not one universal scalar |
| **Ionization energy** | Boundary escape threshold |
| **Electron affinity** | Boundary capture tendency |
| **Polarizability** | Cloud compliance |
| **Spectral lines** | Allowed finite transition readouts |
| **Magnetic moment** | Spin/orbital orientation residue |
| **Nuclear spin** | Central source orientation |
| **Isotope half-life** | Source instability timescale |
| **Chemical valence** | Boundary compatibility class |
| **Electronegativity** | Boundary pull strength |
| **Collision cross section**| Public interaction footprint |
| **Excited-state lifetime** | Relaxation timescale ($\tau_{\rm cloud}$) |
| **Hyperfine structure** | Nuclear-electron coupling residue |

---

## 3. Formal State Vector

The strongest formal package to describe a fully contextualized atom in the FTD framework is the atomic state vector $\mathcal{A}_{\rm atom}$:

$$
\mathcal{A}_{\rm atom} =
\left(
Z, \,
N, \,
q, \,
m, \,
I_{\rm nuc}, \,
\mu_{\rm nuc}, \,
\mathcal{E}_{\rm elec}, \,
R_{\rm cloud}, \,
\delta_{\rm shell}, \,
\alpha_{\rm pol}, \,
E_{\rm ion}, \,
E_{\rm aff}, \,
\tau_{\rm relax}, \,
\sigma_{\rm scatter}
\right)
$$

Where $\mathcal{E}_{\rm elec}$ represents the electron configuration and shell state.

---

## 4. Scale Context and Environmental Dependence

An atom **does not have one absolute radius**. It possesses several radius-readouts (covalent, ionic, Van der Waals) depending entirely on context: isolated, bonded, ionized, excited, compressed, or measured.

Furthermore, an atom changes its readout depending on its surroundings (e.g., molecule, crystal, strong fields, high pressure). Therefore:

> [!IMPORTANT]
> **An atom is not just an isolated object; it is an object *plus* its readout context.**

---

## 5. Implemented Closure-Context Vector

The Scale-2 engine now exposes a narrower diagnostic projection of
$\mathcal{A}_{\rm atom}$ named `AtomicClosureContext`. It captures the shell
scale needed by `SPEC_SCALE_CONTEXT_READOUT.md` without changing the molecular
dynamics force scale.

The implemented vector is:

$$
\mathcal{C}_{\rm atom} =
\left(
Z,\,
n_{\rm shell},\,
Z_{\rm eff},\,
R_{\rm cloud},\,
\delta_{\rm valence},\,
\xi_{\rm orbital},\,
\tau_{\rm electronic}
\right)
$$

with diagnostic ratios:

$$
\kappa = \frac{R_{\rm cloud}}{a},\qquad
\zeta = \frac{R_{\rm cloud}}{L},\qquad
\beta = \frac{\delta_{\rm valence}}{R_{\rm cloud}},\qquad
\Theta = \frac{\tau_{\rm electronic}}{\tau_{\rm ref}}.
$$

The cloud scale is the shell-context estimate:

$$
R_{\rm cloud} = R_{\rm BOHR}\frac{n_{\rm shell}^2}{Z_{\rm eff}}.
$$

This encodes the intended correction:

$$
Z \rightarrow \text{source loading},\qquad
n_{\rm shell} \rightarrow \text{scale tier},\qquad
Z_{\rm eff} \rightarrow \text{screened return force}.
$$

Thus a new shell can increase $R_{\rm cloud}$ even when the simulation
interaction radius still decreases with raw $Z$.

### 5.1 Engine contract

| Surface | Contract |
| :--- | :--- |
| `AtomicClosureContext` | Physics-facing shell-context diagnostic vector. |
| `compute_atomic_closure_context(Z, cfg)` | C++ helper returning the vector from standard shell bookkeeping plus Slater screening. |
| `AtomicProperties::closure_context` | Cached default context returned by `compute_atomic_properties(Z, N)`. |
| `AtomEngine::closure_context_for(id, cfg)` | Per-atom API for current `AtomEngine` instances. |
| `computeAtomicClosureContext(Z, opts)` | Browser/JS mirror in `engine/web/js/atomic-props.js`. |
| `computeAtomicProps(...).closure_context` | JS default context paired with existing simulation properties. |

### 5.2 Non-retuning rule

`Atom.radius` remains the simulation interaction/Lennard-Jones radius used by
Scale-2 forces and scale bridges. It is intentionally still the legacy
monotone form:

$$
R_{\rm sim} = R_{\rm BOHR}/Z^{1/3}.
$$

`AtomicClosureContext::r_cloud` is the physical shell-context readout. It must
not be substituted into `vdw_sigma`, bond capture radii, CUDA atom buffers, or
Scale-2 force kernels without a separate molecular-dynamics retuning and audit.

### 5.3 Epistemic status

- Electron configuration and Aufbau exceptions are standard chemistry
  bookkeeping used as reference structure.
- Slater shielding constants are `[IMPOSED]` empirical screening rules.
- The $R_{\rm BOHR} n^2/Z_{\rm eff}$ cloud radius is a parametric hydrogenic
  shell-scale estimate, not an FTD derivation of empirical atomic radii.
- No fine-structure, CODATA, spectroscopy, or target atomic-radius data enter
  the implementation.

Reference tests: `engine/tests/test_atom_engine.cpp` checks that Li's closure
cloud exceeds H's after the $n=2$ shell opens, that Period-2 closure scales
contract across Li -> C -> Ne, that Na resets larger than Ne, and that the
legacy simulation radius remains unchanged.

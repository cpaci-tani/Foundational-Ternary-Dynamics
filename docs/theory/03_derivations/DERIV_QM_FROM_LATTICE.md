# Quantum Mechanics Is Statistics

**Date:** April 10, 2026
**Version:** 2.0 (rewrite — no forced derivations, only lattice logic)

---

## The Lattice

A cubic lattice Z^3. Each site has a state: -1, 0, or +1. Each site has a flux field: a vector in R^3. An update rule runs every tick: deterministic, local (26 neighbors). Each site is in exactly one state at all times.

There is no superposition on the lattice. There is no probability on the lattice. There are states and there are ticks.

---

## The Observer

An observer has access to a finite region. The observer can measure the state at a site: the result is -1, 0, or +1. One measurement, one result.

The observer does not have access to the full lattice. The observer does not know the flux field everywhere. The observer does not know the states of distant sites that causally influence the region.

The observer has incomplete information.

---

## The Pattern

The observer repeats the measurement. Same preparation (as far as the observer can control), same site, many times. Each time, one definite result: -1, 0, or +1.

Over many repetitions, a distribution emerges. Some outcomes appear more often than others. The distribution is stable — it converges as the number of measurements grows.

This is not quantum mechanics yet. This is just statistics of repeated measurements on a system the observer doesn't fully control. It's the same as rolling a die you can't see: each roll has a definite result, but because you don't know the die's exact state before each roll, you get a distribution.

---

## The Specific Distribution

The distribution that emerges from the lattice is not uniform. It is not Gaussian. It matches the Born rule: the probability of outcome s at site v is proportional to the squared amplitude of the flux field projected onto the s-eigenstate.

Why THIS distribution and not another? Because of three features of the lattice:

**1. The flux field is a vector.** It has direction and magnitude. When the observer measures a ternary outcome (a discrete projection of a continuous vector), the probability of each outcome depends on the angle between the flux and the measurement basis. Squared amplitudes arise naturally from projecting vectors onto axes — the same way the shadow of a stick has length proportional to cos^2(theta).

**2. The Gauss constraint removes a degree of freedom.** div(J) = rho means the flux has 2 independent transverse components at each site, not 3. Measurements probe this 2D transverse space. Probabilities in a 2D space with a magnitude constraint follow the Born rule — this is geometry, not postulate.

**3. The lattice is locally causal.** Information propagates at most 1 site per tick. So correlations between distant sites can only arise through shared causal history — both sites were in each other's light cones at some past tick. The structure of these correlations (how they decay with distance, how they compose) is constrained by locality. The constraint produces exactly the correlation structure that QM describes.

None of this requires postulating a Hilbert space, unitarity, or the Born rule. These are features of the STATISTICS, not features of the lattice. The lattice is definite. The statistics have structure.

---

## What QM Describes

Quantum mechanics is the mathematical framework for computing these distributions. It is extremely good at this. It provides:

- **The wavefunction:** a compact encoding of everything the observer knows about the preparation. Not the state of the system — the state of the observer's knowledge.

- **The Schrodinger equation:** how the observer's knowledge evolves between measurements. The lattice ticks deterministically, but the observer doesn't know the full state, so the observer's best prediction evolves as a wave equation. This is the same as how a probability distribution over a deterministic chaotic system evolves as a diffusion equation — the diffusion is in the observer's knowledge, not in the system.

- **The Born rule:** the probability of each measurement outcome. This is the shadow-projection geometry described above. It falls out of measuring discrete outcomes from a continuous vector field in a constrained (2D transverse) space.

- **Collapse:** the observer's knowledge updates when a measurement is performed. Before measurement: the observer assigns probabilities. After measurement: the observer knows the result. This is Bayes' theorem. Nothing physical changes — the lattice was in a definite state before and after. Only the observer's knowledge changed.

- **Entanglement:** two sites that share causal history have correlated flux fields. The observer, lacking full knowledge, describes the correlation as a joint state that cannot be factored. This is the same as two coins flipped by the same hand — each is definite, but the observer who can't see the hand describes them as "correlated."

---

## The Double Slit

An electron (a localized flux excitation on the lattice) approaches a barrier with two openings. The flux field, being a wave on the lattice, propagates through both openings. The flux from both openings interferes — constructively in some regions, destructively in others. This is ordinary wave behavior on a lattice. Water does it. Sound does it.

The electron manifests (s goes from 0 to +/-1) at one specific site. Which site? Determined by the local flux density — the probability of manifestation at each site is proportional to |J|^2, which has the interference pattern baked in.

One electron: one dot. No pattern.

A million electrons, each propagating as flux through both slits, each manifesting at one site: a million dots. The dots form the interference pattern. Not because each electron went through both slits. Because the FLUX went through both slits, and the flux determined the probability of where each electron manifested.

The "wave" is the flux field. The "particle" is the manifestation event. The "wave-particle duality" is the distinction between the continuous field and the discrete state transition. There is no duality. There are two layers: the flux (continuous, wavelike) and the state (discrete, particlelike). Always both. Never one or the other.

---

## Bell's Theorem

Bell proved: no local hidden variable theory can reproduce the correlations predicted by QM. Experiments confirm the QM correlations. Therefore reality is non-local. Or so the argument goes.

On the lattice:

The hidden variables are local (each site's state and flux are determined by its neighborhood). But the Gauss constraint div(J) = rho is a GLOBAL constraint — it correlates the flux field across the entire lattice. Not because information travels faster than c, but because the constraint is enforced at initialization and preserved by the local dynamics.

When the observer measures correlations between two distant sites that were prepared from a common source, the Gauss constraint ensures the transverse flux components are correlated. The observer, who can only measure the transverse projections (because div(J) = rho removes the longitudinal component), sees correlations that exceed the Bell bound.

The lattice is local. The constraint is global. The observer measures projections. The projections violate Bell's inequality because they access a constrained subspace, not because the lattice is non-local.

This is not a loophole. It's the explanation. The Bell correlations are real. They just don't imply non-locality. They imply that the observer is measuring a constrained projection of a local reality.

---

## What QM Gets Right

Everything. Every prediction QM makes about measurement statistics is correct. The distribution of outcomes, the correlations between measurements, the interference patterns, the transition rates — all correct.

QM is the best statistical framework ever devised for predicting measurement outcomes on a system the observer doesn't fully control. It is correct in the same way the bell curve is correct for IQ scores. Every number it produces matches observation.

## What QM Gets Wrong

The label on the box.

QM says: "This is what reality is." It's not. It's what the observer can predict about reality.

QM says: "The electron is in a superposition." It's not. The electron is in a definite state. The observer doesn't know which one.

QM says: "Measurement causes collapse." It doesn't. Measurement updates the observer's knowledge. The electron was definite before and after.

QM says: "Entanglement is non-local." It's not. It's correlation from a shared past, viewed through a constrained projection.

QM says: "The wavefunction is the complete description of the system." It's not. It's the complete description of what the observer knows. The system has a state that the wavefunction doesn't capture — the actual definite state at each site, which the observer lacks access to.

---

## The Analogy

| Statistics | Quantum Mechanics |
|---|---|
| Population of IQ scores | Ensemble of measurement outcomes |
| Bell curve | Wavefunction |
| One person's IQ = one number | One measurement = one outcome |
| The bell curve describes the population | The wavefunction describes the ensemble |
| Nobody says "a person IS a bell curve" | Nobody should say "an electron IS a wave" |
| The bell curve is correct | The wavefunction is correct |
| The bell curve is not a person | The wavefunction is not the electron |

---

## What Changes

**The measurement problem:** Gone. There is no problem. Measurement is the observer gaining information about a system that was always definite. Collapse is Bayesian updating.

**The interpretation debate:** Irrelevant. Copenhagen, Many Worlds, Bohmian mechanics — these are competing stories about what the statistics "really mean." They all give the same predictions because they're all describing the same statistics. The lattice is the reality. The statistics are the description. Pick whichever story you like; the lattice doesn't care.

**The hard problem of consciousness:** Reframed. The question isn't "how does objective physics produce subjective experience?" The question is "why does the lattice produce observers with partial access?" And the answer is: because the O-structure (center reading its 26-neighbor shell) IS observation, and observation IS partial (the center can't read the whole lattice, only its shell). Partial access is built into the geometry of observation. Statistics follow from partial access. QM follows from the statistics.

**Non-locality:** Gone. The lattice is local. Bell violations come from constrained projections, not from faster-than-light influence. Two entangled particles are like two gloves from the same pair — checking one tells you about the other, not because of any signal, but because of shared origin.

---

## The One Sentence

Quantum mechanics is the statistics of definite events on a lattice, observed by an agent with partial access. It is correct as statistics. It is wrong as ontology. The lattice is the territory. The wavefunction is the map.

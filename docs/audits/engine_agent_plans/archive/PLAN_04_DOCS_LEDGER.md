# Plan 04 — Documentation and LEDGER Updates

## Objective

Add honest documentation for the three new graph modules and update the project status ledger.

## New docs

### 1. Branch holonomy theorem

Path:

`docs/theory/03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md`

Status:

THEOREM.

Core statement:

\[
H_x=-1
\Rightarrow
\lambda_{\min}=4\sin^2\left(\frac{\pi}{2N}\right).
\]

Required non-claim:

This is a finite graph branch-gap theorem. It is not by itself a particle mass derivation.

### 2. Z3 center closure

Path:

`docs/theory/03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md`

Status:

- center-neutral arithmetic: THEOREM
- confinement dynamics: OPEN
- open-flux penalty: CANDIDATE PRINCIPLE

Required non-claim:

The projector \(P_0^{(c)}\) proves center-neutral selection, not full QCD confinement.

### 3. Generation graph candidate

Path:

`docs/theory/05_particles/EXPLR_GENERATION_GRAPH_GAMMA_D.md`

Status:

CANDIDATE RECONSTRUCTION.

Core candidate:

\[
\Gamma_F(d)=K_3(q^{d+1},1,q^d;\phi=\pi+\pi/d)
\]

with:

\[
\Gamma_U=\Gamma_F(3),\qquad \Gamma_D=\Gamma_F(2).
\]

Required non-claim:

This is not a CKM theorem until \(d_U=3\), \(d_D=2\), and the phase rule are theorem-forced.

## LEDGER update template

Add rows similar to this. Use the repo's actual LEDGER format.

### Row 1

ID: next available  
Claim: Periodic branch anti-holonomy on an \(N^3\) cubic torus shifts allowed momentum to \(p=\pi/N\), giving \(\lambda_{\min}=4\sin^2(\pi/2N)\) for one twisted cycle.  
Status: THEOREM  
Evidence: `DERIV_BRANCH_HOLONOMY_GAP.md`; `test_branch_holonomy_gap.cpp`  
Dependencies: finite periodic cubic graph, signed incidence convention  
Non-claims: no direct particle mass prediction

### Row 2

Claim: Finite \(\mathbb Z_3\) center neutrality selects \(q\bar q\) and \(qqq\) closures via \(\sum c_i\equiv0\pmod3\).  
Status: THEOREM / CONDITIONAL THEOREM depending on ledger taxonomy  
Evidence: `DERIV_Z3_CENTER_GRAPH_CLOSURE.md`; `test_z3_color_center.cpp`  
Dependencies: finite \(\mathbb Z_3\) center model  
Non-claims: not full confinement, not \(\Lambda_{\rm QCD}\)

### Row 3

Claim: A compact one-parameter generation graph \(\Gamma_F(d)\) with \(d_U=3,d_D=2\) produces a CKM-like eigenbasis overlap.  
Status: CANDIDATE RECONSTRUCTION  
Evidence: `EXPLR_GENERATION_GRAPH_GAMMA_D.md`; `test_generation_graph.cpp`  
Dependencies: selected \(q^*\)-power edge weights, selected loop phase, selected sector depths  
Non-claims: not theorem-forced CKM, not mass derivation

## META_INDEX update

Add entries in the correct sections:

- `03_derivations` for branch holonomy
- `03_derivations` for Z3 center closure
- `05_particles` for generation graph

## Project atlas update

Only update `META_PROJECT_ATLAS.md` if the new modules create a new navigation category. Suggested row:

| If you want to work on finite graph overlays | `engine/include/ftd/branch_holonomy.h`, `color_center.h`, `generation_graph.h` | tests in `engine/tests/test_*graph*.cpp` |

## Acceptance criteria

- Docs compile as markdown.
- LEDGER statuses are conservative.
- No theorem overpromotion.
- All references point to actual files.

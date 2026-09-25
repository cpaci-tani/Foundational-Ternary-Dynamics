# Foundational Ternary Dynamics

**Foundational Ternary Dynamics (FTD)** is a philosophy-of-mathematics project
with an explicit mathematical core and deliberately bounded physics
connections. It asks one question, as honestly as it can be asked:

> Starting from finite-alphabet, local, deterministic records with a ternary
> manifestation readout, what can be *built*, and exactly where and why does
> the building stop?

FTD is **not** an attempt to replace the Standard Model or general relativity.
It is ordered **Ontology > Logic > Math > Physics**: the substrate fixes a small
set of commitments, mathematics is built forward from them, and physics enters
as a *constraint*, not the sole arbiter. The discipline that makes the project
worth reading is that it labels every claim by exactly how far it has actually
been carried: theorem, derivation, selection, conjecture, parametric insertion,
imposed input, open problem, or closed-negative route. It does not let a label
promote a claim beyond its evidence.

The repository combines a theory corpus, verification and proof scripts, a
C++/CUDA simulation engine, and a browser workspace. The
[repository map](REPOSITORY_MAP.md) shows module ownership; the
[dated resume](docs/WHERE_WE_LEFT_OFF.md) records the current working state.

## G* in exact mathematics

The constant used to organize one strand of FTD's mathematical work is

$$
G^* := \frac{\Gamma(1/4)}{\Gamma(3/4)}
     = \frac{\Gamma(1/4)^2}{\pi\sqrt{2}}
     = 2.958675119\ldots .
$$

The second form follows at once from Euler's [reflection
formula](https://dlmf.nist.gov/5.5#E3). Its exact formulas connect quartic
periods, modular values, a lattice Green function, and L-functions. These
are classical relations among mathematical objects, not separate physical
measurements.

### Quartic and elliptic periods

Let

$$
\varpi := 2\int_0^1 \frac{dt}{\sqrt{1-t^4}}
        = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}}.
$$

This is the lemniscatic period constant; it is **not** $G^*$. Their exact
relation is $G^*=2\varpi/\sqrt\pi$. The quartic integral is also a period
integral of the elliptic curve $E:y^2=x^3-x$: the substitution $x=t^{-2}$
transforms its invariant differential $dx/(2y)$ into
$-dt/\sqrt{1-t^4}$. The order-four automorphism
$(x,y)\mapsto(-x,iy)$ over $\mathbb C$ exhibits its square-lattice complex
multiplication.

Write $K(k)=\int_0^{\pi/2}(1-k^2\sin^2 u)^{-1/2}du$ for the complete
elliptic integral and $\operatorname{AGM}$ for Gauss's arithmetic-geometric
mean. At the self-complementary modulus $k=1/\sqrt2$, the same period gives

$$
G^* = \frac{2\sqrt2}{\sqrt\pi}K(1/\sqrt2)
    = \frac{2\sqrt\pi}{\operatorname{AGM}(1,\sqrt2)}.
$$

The second equality uses the [elliptic-integral/AGM
identity](https://dlmf.nist.gov/19.8#E5), with its modulus normalization
stated explicitly.

### A modular value and a lattice return sum

For $\vartheta_3(q)=\sum_{n\in\mathbb Z}q^{n^2}$, the square-lattice nome
$q=e^{-\pi}$ gives another exact value:

$$
G^* = \sqrt{2\pi}\,\vartheta_3(e^{-\pi})^2
    = \sqrt{2\pi}\sum_{(m,n)\in\mathbb Z^2}e^{-\pi(m^2+n^2)}.
$$

This is the [theta/elliptic-integral identity](https://dlmf.nist.gov/20.9#E2)
at the square-lattice parameter.

The three-dimensional body-centred-cubic (BCC) walk supplies a different
kind of representation. Its normalized return Green function is

$$
\begin{aligned}
W_{\mathrm{BCC}}
&= \frac{1}{\pi^3}\int_{[0,\pi]^3}
   \frac{du\,dv\,dw}{1-\cos u\cos v\cos w} \\
&= \sum_{m=0}^{\infty}\left(\frac{\binom{2m}{m}}{4^m}\right)^3
 = \frac{\Gamma(1/4)^4}{4\pi^3}
 = \frac{(G^*)^2}{2\pi}.
\end{aligned}
$$

$W_{\mathrm{BCC}}$ is the **sum** of return probabilities, not the
probability of one return. The product-cosine denominator identifies the BCC
walk; a simple-cubic or face-centred-cubic Watson integral is a different
object. See the [normalized lattice derivation](docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md)
and [Watson's evaluation](https://academic.oup.com/qjmath/article-abstract/os-10/1/266/1520070).

### Arithmetic special values

A Dirichlet L-derivative and a fixed elliptic-curve L-value give two
arithmetic expressions for $G^*$. For the Dirichlet beta function
$\beta(s)=L(s,\chi_{-4})$, the [Hurwitz-zeta
derivative](https://dlmf.nist.gov/25.11#E18) at zero gives

$$
\log G^* = \beta'(0)+\log 2.
$$

For the **specified** curve $E:y^2=x^3-x$, of conductor 32, its central
value is

$$
L(E,1)=\frac{\varpi}{4}=\frac{G^*\sqrt\pi}{8}.
$$

The first equality follows from the associated level-32 newform's
Mellin/Beta integral; it is not a statement about all CM curves or their
twists. See [Li, Long and Tu, Lemma
3.1](https://sigma-journal.com/2018/090/) and
the [curve record](https://www.lmfdb.org/EllipticCurve/Q/32/a/3). The
[identity manuscript](docs/papers/src/PAPER_GSTAR_IDENTITIES.tex) collects
further forms and their normalizations; equivalent rearrangements are not
independent proofs.

## The FTD question

The equalities above hold whether or not FTD describes nature. They do not
show that $G^*$ is selected by the finite dynamics, nor that it equals a
measured coupling. In particular, the BCC formula evaluates an idealized
lattice Green function, not a native observable of the selected v3 law.

The [v3 constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)
adopts finite records on an oriented three-dimensional cubical complex,
ordinal ticks, a ternary site readout of a larger record, radius-one Moore
causality, and a homogeneous deterministic transition
$X_{n+1}=\Phi(X_n)$. The [carrier](docs/theory/01_reference/SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md)
and [reference law](docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md)
are selected constructions. They have finite collision and expiry
certificates and a bounded transverse-vacuum linearization at stated
preparations; they are not uniquely forced by the postulates.

To connect the mathematics to physics, a target-blind readout of finite
$\Phi$ histories would need a specified preparation, domain, and error or
stability bound, followed by a separate operational identification. That
chain is open for $G^*$, as are general recoveries of stable matter,
charged electromagnetism, Born frequencies, a common relativistic cone, and
tensor gravity. Substituting a framework value into an established physics
formula is a [parametric insertion](docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md),
not a recovery of that formula.

The [C++ engine](engine/SPEC_ENGINE.md) implements a different, legacy
continuous-flux law with optional extensions. Its measurements are not runs
of the selected v3 $\Phi$. The browser workspace also includes separate
effective engines and visualizations; the [scale ownership
audit](docs/reference/REF_SCALE_OWNERSHIP_AND_RECOVERY.md) records their
current relationship to the finite model.

## Reading the project

The [ledger](docs/theory/07_assessment/core_ledgers/LEDGER.md) controls
claim status, followed by the active v3 constitution, branch constitutions,
and other prose. The [dated resume](docs/WHERE_WE_LEFT_OFF.md) gives the
current state; the [open-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md)
and [resolved-items tracker](docs/theory/07_assessment/core_ledgers/TRACKER_RESOLVED_ITEMS.md)
separate live work from provenance. The [curated theory index](docs/theory/META_INDEX.md),
[repository map](REPOSITORY_MAP.md), and [contributing guide](CONTRIBUTING.md)
are the shortest routes into the corpus and code.

## Run and check

These commands exercise documented code and finite checks. A passing test
does not change a claim's physical status.

```bash
python -m pip install -r requirements.txt -r requirements-dev.txt
python -m pytest scripts/tests/
python scripts/proofs/proof_v3_common_action_phi_v2.py
python scripts/proofs/proof_v3_target_firewall.py
python scripts/verification/verify_index_links.py --tracked
```

For the CPU-only C++ merge gate:

```bash
cmake -S engine -B engine/build -DFTD_ENABLE_CUDA=OFF
cmake --build engine/build --target ftd_merge_gate_build --config Release --parallel 24
ctest --test-dir engine/build -L merge_gate -j 24 -C Release --output-on-failure
```

The full Windows native build uses `engine\build_native.bat`. Its toolchain
and GPU requirements are recorded in [CLAUDE.md](CLAUDE.md).

Build the browser assets:

```bash
npm ci
npm run build
```

Run the source workspace with its local service:

```bash
python engine/web/serve.py 8080
```

Open `http://localhost:8080`. Browser test dependencies and commands are in
the [web test package](engine/web/tests/package.json).

## License and citation

The repository is distributed under [CC BY-NC-SA 4.0](LICENSE). Cite a
versioned document or certificate for a specific result; a repository-level
citation identifies the corpus, not the status of every historical claim.

```bibtex
@misc{ftd2026,
  title = {Foundational Ternary Dynamics},
  year = {2026},
  note = {Research corpus and simulation software},
  url = {https://github.com/cpaci-tani/Foundational-Ternary-Dynamics}
}
```

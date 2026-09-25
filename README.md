# Foundational Ternary Dynamics

**Foundational Ternary Dynamics (FTD)** is a philosophy-of-mathematics project
with an explicit mathematical core and deliberately bounded physics
connections. It asks one question, as honestly as it can be asked:

> Starting from finite-alphabet, local, deterministic records with a ternary
> manifestation readout, what can be *built*, and exactly where and why does
> the building stop?

Its order is **Ontology > Logic > Math > Physics**: specify the objects,
establish their mathematical relations, and investigate the physical
descriptions they support. A recurring object in this work is the constant
$G^*$. Its exact representations connect a quartic period to elliptic
curves, convergent averaging, square-lattice sums, three-dimensional random
walks, and arithmetic L-values.

## The constant G*

Define

$$
G^* := \frac{\Gamma(1/4)}{\Gamma(3/4)}
     = \frac{\Gamma(1/4)^2}{\sqrt{2}\,\pi}
     = 2.958675119188638892\ldots .
$$

The second equality is Euler's [reflection
formula](https://dlmf.nist.gov/5.5#E3) at $1/4$. Gamma recurrence gives an
elementary sequence converging to the same value:

$$
G^* = \lim_{N\to\infty}\frac{1}{\sqrt N}
      \prod_{n=0}^{N-1}\frac{4n+3}{4n+1}.
$$

The factors run through the two odd residue classes modulo four. The
normalized finite product equals
$G^*\Gamma(N+3/4)/[\sqrt N\,\Gamma(N+1/4)]$; the gamma-ratio asymptotic
makes its limit explicit. This [Wallis-type
representation](docs/papers/src/PAPER_GSTAR_IDENTITIES.tex) builds the value
from integer arithmetic and a square-root normalization.

## A quartic period and an elliptic curve

The substitution $u=t^4$ in the beta integral gives

$$
I:=\int_0^1\frac{dt}{\sqrt{1-t^4}}
  =\frac14 B\!\left(\frac14,\frac12\right)
  =\frac{\sqrt\pi}{4}G^*.
$$

Writing $\varpi=2I$ defines the lemniscatic period constant, with
$G^*=2\varpi/\sqrt\pi$. The distinction between $G^*$ and $\varpi$
fixes the normalization as the same integral moves between settings.

Consider the elliptic curve

$$
E:\quad y^2=x^3-x,\qquad \omega=\frac{dx}{2y}.
$$

On the positive real branch, $x=t^{-2}$ transforms $\omega$ into
$-dt/\sqrt{1-t^4}$. For this differential the least positive real period
is $\varpi$. Over $\mathbb C$, the map $(x,y)\mapsto(-x,iy)$ is an
order-four automorphism. The period lattice is a real rescaling of
$\mathbb Z+i\mathbb Z$, and the curve has complex multiplication by the
Gaussian integers $\mathbb Z[i]$. Thus the quartic integral belongs to the
geometry of a square complex torus. The
[period calculation](docs/papers/src/PAPER_GSTAR_IDENTITIES.tex) and
[curve record](https://www.lmfdb.org/EllipticCurve/Q/32/a/3) specify this
curve and differential.

## Elliptic integrals and the arithmetic-geometric mean

Use the modulus convention

$$
K(k)=\int_0^{\pi/2}\frac{du}{\sqrt{1-k^2\sin^2u}}.
$$

The lemniscatic case is $k=1/\sqrt2$, where the modulus equals its
complement $k'=\sqrt{1-k^2}$. Transforming the quartic integral and applying
[Gauss's AGM identity](https://dlmf.nist.gov/19.8#E5) gives

$$
G^* = \frac{2\sqrt2}{\sqrt\pi}K(1/\sqrt2)
    = \frac{2\sqrt\pi}{\operatorname{AGM}(1,\sqrt2)}.
$$

Starting with $a_0=1$ and $b_0=\sqrt2$, the iteration
$a_{n+1}=(a_n+b_n)/2$, $b_{n+1}=\sqrt{a_nb_n}$ brings both sequences to
their common arithmetic-geometric mean. Their difference contracts
quadratically near the limit. An elliptic period therefore also has a fast
construction using arithmetic and square roots.

## Theta functions and square-lattice counting

The equality $K(k')=K(k)$ gives period ratio $\tau=i$ and elliptic nome
$q=\exp(-\pi K(k')/K(k))=e^{-\pi}$. With
$\vartheta_3(q)=\sum_{n\in\mathbb Z}q^{n^2}$, the
[theta representation of the elliptic integral](https://dlmf.nist.gov/20.9#E2)
becomes

$$
\begin{aligned}
\frac{G^*}{\sqrt{2\pi}}
 &=\vartheta_3(e^{-\pi})^2 \\
 &=\sum_{(m,n)\in\mathbb Z^2}e^{-\pi(m^2+n^2)}
  =\sum_{r=0}^{\infty}r_2(r)e^{-\pi r}.
\end{aligned}
$$

Here $r_2(r)$ counts ordered integer pairs $(m,n)$ with $m^2+n^2=r$,
including signs, and $r_2(0)=1$. Grouping the Gaussian lattice sum by
squared distance expresses the normalized period as a value of the
generating function for sums of two squares. The square lattice of the
elliptic curve now appears as an explicit counting problem.

## Return paths on the BCC lattice

A second lattice connection lives in three dimensions. At each step let a
walker choose one of the eight displacements $(\pm1,\pm1,\pm1)$, uniformly
and independently of previous steps. Starting at the origin, the reachable
positions form a body-centred-cubic lattice. A return after $2m$ steps requires each
coordinate to have exactly $m$ positive increments, so

$$
\Pr(S_{2m}=0)
 =\left(\frac{\binom{2m}{m}}{4^m}\right)^3,
\qquad \Pr(S_{2m+1}=0)=0.
$$

Summing these probabilities gives the expected number of visits to the
origin, including the initial visit. The corresponding Fourier integral
and Watson evaluation are

$$
\begin{aligned}
W_{\mathrm{BCC}}
 &=\sum_{m=0}^{\infty}
    \left(\frac{\binom{2m}{m}}{4^m}\right)^3 \\
 &=\frac1{\pi^3}\int_{[0,\pi]^3}
   \frac{du\,dv\,dw}{1-\cos u\cos v\cos w} \\
 &=\frac{4K(1/\sqrt2)^2}{\pi^2}
  =\frac{(G^*)^2}{2\pi}
  =1.393203929685676859\ldots .
\end{aligned}
$$

The probability of ever returning after departure is consequently

$$
\Pr(\text{return})=1-\frac1{W_{\mathrm{BCC}}}
                  =1-\frac{2\pi}{(G^*)^2}
                  =0.282229988953870002\ldots .
$$

[Guttmann's account of lattice Green functions](https://arxiv.org/abs/1004.1435),
Introduction and Section 1.2, gives the walk normalization, return relation, and
elliptic-integral evaluation. This is a precise connection between an
elliptic period and a discrete stochastic process on the specified BCC
graph.

## Arithmetic special values

The residue classes in the finite product also define the Dirichlet
character $\chi_{-4}$: it is $0$ on even integers, $1$ on integers congruent
to $1$ modulo four, and $-1$ on those congruent to $3$. Its L-function is
the Dirichlet beta function,

$$
\beta(s)=L(s,\chi_{-4})
        =\sum_{n=0}^{\infty}\frac{(-1)^n}{(2n+1)^s},
\qquad \Re(s)>1.
$$

After analytic continuation to $s=0$, the identity
$\beta(s)=4^{-s}[\zeta(s,1/4)-\zeta(s,3/4)]$ and the
[Hurwitz-zeta derivative formula](https://dlmf.nist.gov/25.11#E18) yield

$$
\log G^*=\beta'(0)+\log2.
$$

The curve $E:y^2=x^3-x$ provides another arithmetic construction. For each
odd prime $p$, let $a_p=p+1-\#E(\mathbb F_p)$ measure its point-count
deviation. These are the prime coefficients of the weight-two,
level-32 newform

$$
f(\tau)=\eta(4\tau)^2\eta(8\tau)^2
       =\sum_{n\ge1}a_n e^{2\pi i n\tau},
$$

where $\eta$ is the Dedekind eta function. Its coefficients define the
Hasse-Weil L-function $L(E,s)=\sum_{n\ge1}a_n n^{-s}$ for sufficiently
large $\Re(s)$, followed by analytic continuation. The Mellin integral
evaluated by [Li, Long and Tu, Lemma 3.1](https://sigma-journal.com/2018/090/)
gives

$$
L(E,1)=\frac18 B\!\left(\frac12,\frac14\right)
      =\frac{\varpi}{4}
      =\frac{G^*\sqrt\pi}{8}
      =0.655514388573029952\ldots .
$$

Counting points modulo primes has led back to the real quartic period.
This equality concerns the specified conductor-32 curve; other curves and
twists have their own L-functions and period normalizations.

These are classical identities. Their breadth comes from the constructions
they relate: an integral on a curve, an averaging iteration, a Gaussian
lattice sum, a random-walk Green function, and arithmetic special values.
The [identity manuscript](docs/papers/src/PAPER_GSTAR_IDENTITIES.tex)
collects the derivations and further representations, with its
[review corrections](docs/papers/src/GSTAR_IDENTITIES_CORRECTIONS.md) and
[verification catalogue](scripts/verification/gstar_identity_catalogue.py).

## The finite construction in FTD

FTD asks how such mathematical objects can arise as observables or
controlled limits of finite local records. The
[v3 constitution](docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)
adopts an oriented three-dimensional cubical complex, ordinal ticks, a
ternary readout of larger finite records, radius-one Moore causality, and a
homogeneous deterministic transition $X_{n+1}=\Phi(X_n)$. Its
[carrier](docs/theory/01_reference/SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md)
and [reference law](docs/theory/01_reference/SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md)
are selected constructions with finite collision and expiry certificates
and a bounded transverse-vacuum linearization at specified preparations.

A dynamical realization of $G^*$ would require a defined readout from
finite $\Phi$ histories, a preparation and scaling domain, and a
convergence or error bound. That link remains open. General recovery of
stable matter, charged electromagnetism, Born frequencies, a common
relativistic cone, and tensor gravity also remains open.

The [C++ engine](engine/SPEC_ENGINE.md) implements a legacy continuous-flux
law that differs from the selected v3 $\Phi$. Its results describe that
implementation and its chosen configuration. The browser workspace includes
additional effective engines; their relationship to the finite model is
recorded in the [scale ownership
audit](docs/reference/REF_SCALE_OWNERSHIP_AND_RECOVERY.md).

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

The identity verifier evaluates the catalogue's stated formulas and support
checks at high precision. Numerical agreement checks the implementation;
the mathematical arguments and their assumptions are in the linked sources.

```bash
python -m pip install -r requirements.txt -r requirements-dev.txt
python -m pytest scripts/tests/
python scripts/verification/verify_gstar_identity_catalogue.py
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

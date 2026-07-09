# REF — Exported results: the number theory of G\* = Γ(1/4)/Γ(3/4) (mathematician-facing)

**Tag:** [REFERENCE] — a self-contained, external-circulation statement of the *closed*, theorem-grade results organized around one distinguished constant. FTD-free by design: no framework vocabulary appears in the results; project context is confined to the closing appendix. Introduces no claim and promotes nothing. Companion to [`REF_EXPORTED_PROBLEMS_E1_E2.md`](REF_EXPORTED_PROBLEMS_E1_E2.md) (which exports the *open* problems); this document exports the *closed* ones.
**Audience:** working number theorists / transcendence theorists / arithmetic-geometers.
**Attribution discipline:** classical results are credited to their sources; the small residue that is project-specific is a *packaging/selection*, flagged as such, and never presented as a theorem.

---

## 0 · What this is, and how to read it

A single real constant,
$$G^{*} \;:=\; \frac{\Gamma(1/4)}{\Gamma(3/4)} \;=\; 2.9586751191886388923108213577\ldots,$$
sits at the intersection of several classical threads — the lemniscatic CM elliptic curve, the Chowla–Selberg formula, Watson's cubic-lattice integrals, symmetric-square and BSD $L$-values, and the Grothendieck period conjecture. This note collects, in mathematician-standard language, exactly which statements about $G^{*}$ are **theorems** (and whose), which are **numerical observations**, and which are **open** — and isolates the single non-elementary arithmetic input on which all transcendence content rests.

**Status words** (used in place of any project tag apparatus):
- **Theorem (classical).** Proven; elementary or standard.
- **Theorem (external).** Proven in the cited literature.
- **Proposition / Corollary.** Elementary; proof given.
- **Numerical observation.** Verified to high precision; *not* a theorem.
- **Open.** Genuinely open.
- **Selection (not forced).** A chosen assembly of otherwise-classical data; explicitly *not* a theorem.

### 0.1 · Four honesty guards (please keep these in view)

The results below are individually correctly labelled, but read *together* they can generate more momentum than they earn. Four guards:

1. **The degree-2 polynomial of §5 is not forced.** Its two coefficients are $L$-values (theorems); assembling them into that *particular* monic quadratic (a symmetric trace/norm target and the prefactor $16$) is a **selection** — for a $2\times2$ operator, trace and determinant are algebraically independent invariants, so no self-duality supplies a gluing constraint.
2. **The integer $16$ has two distinct roles.** As $|E(\mathbb{Q})_{\mathrm{tors}}|^{2}$ it is *forced* in the BSD bookkeeping (§1). As the *prefactor* of the §5 quadratic it is a *selected* member of a coincident set $|\mathrm{Aut}(E)|^{2}=|E(\mathbb{Q})_{\mathrm{tors}}|^{2}=16$. "Forced in BSD" must not be promoted into "forced prefactor."
3. **"Distinguished" ≠ "unique / maximal / forced."** $\mathbb{Q}(G^{*})$ is *a* $\pi$-free subfield, not the maximal one (§2); $d=-4$ is distinguished by a genuine unit-group theorem but the finer selection among class-number-one fields is criterion-dependent (§3); the period conjecture below is proven for one motive, not in general (§4).
4. **Single point of failure.** *Every* transcendence assertion here — $G^{*}$ transcendental, $\delta$ transcendental, the $\pi$-freeness of $\mathbb{Q}(G^{*})$, the period-conjecture lower bound — rests on the *one* input **Chudnovsky (1976)** (algebraic independence of $\Gamma(1/4)$ and $\pi$). Nothing here is claimed to be *derived* from below.

### 0.2 · Conventions

Throughout, $E$ is the elliptic curve
$$E:\; y^{2}=x^{3}-x \qquad (\text{conductor } 32,\ j=1728,\ \text{CM by } \mathbb{Z}[i],\ \text{LMFDB } \texttt{32.a3}).$$
Its automorphism group is $\mathrm{Aut}(E)\cong \mathbb{Z}/4$ and its rational torsion is $E(\mathbb{Q})_{\mathrm{tors}}\cong(\mathbb{Z}/2)^{2}$, so $|\mathrm{Aut}(E)|^{2}=|E(\mathbb{Q})_{\mathrm{tors}}|^{2}=16$.

We fix $\varpi$ to be the **real half-period** of $E$:
$$\varpi \;=\; \int_{1}^{\infty}\frac{dx}{\sqrt{x^{3}-x}} \;=\; \sqrt{2}\,K\!\left(\tfrac{1}{\sqrt2}\right) \;=\; 2\int_{0}^{1}\frac{dt}{\sqrt{1-t^{4}}} \;=\; \frac{\Gamma(1/4)^{2}}{2\sqrt{2\pi}} \;=\; 2.6220575542921198\ldots$$
(the Bernoulli/Gauss lemniscate constant), so the **full real period** is $2\varpi$. In the BSD formula we take the period $\varpi$ together with Tamagawa product $\prod_p c_p=4$; the equivalent normalization (full period $2\varpi$, $c_2=2$) yields the same $L(E,1)=\varpi/4$. **Note carefully: $G^{*}\approx 2.9587 \ne \varpi\approx 2.6221$** — $G^{*}$ is a $\Gamma$-quotient in the lemniscatic CM-period class, *not* the lemniscate constant itself.

---

## 1 · The constant $G^{*}$: closed forms, the Watson integral, the Chowla–Selberg instance

**Theorem 1 (elementary closed-form tower).** With $\varpi$ as in §0.2, and $G=1/M(\sqrt2,1)$ Gauss's arithmetic–geometric-mean constant,
$$G^{*}=\frac{\Gamma(1/4)}{\Gamma(3/4)}=\frac{\Gamma(1/4)^{2}}{\pi\sqrt2}=\frac{2\varpi}{\sqrt\pi}=2G\sqrt\pi.$$
*Proof.* Euler reflection $\Gamma(z)\Gamma(1-z)=\pi/\sin(\pi z)$ at $z=1/4$ gives $\Gamma(1/4)\Gamma(3/4)=\pi/\sin(\pi/4)=\pi\sqrt2$, hence $\Gamma(3/4)=\pi\sqrt2/\Gamma(1/4)$ and $G^{*}=\Gamma(1/4)^{2}/(\pi\sqrt2)$. With $\varpi=\Gamma(1/4)^{2}/(2\sqrt{2\pi})$ one gets $2\varpi/\sqrt\pi=\Gamma(1/4)^{2}/(\pi\sqrt2)$. Verified to $40$ digits. $\square$

**Theorem 2 (Watson's body-centred-cubic integral; Watson 1939).** Let
$$W_{3}:=\frac{1}{\pi^{3}}\int_{0}^{\pi}\!\!\int_{0}^{\pi}\!\!\int_{0}^{\pi}\frac{dx\,dy\,dz}{1-\cos x\cos y\cos z}$$
be the normalized diagonal Green's function of the BCC lattice. Then
$$W_{3}=\frac{\Gamma(1/4)^{4}}{4\pi^{3}}=\frac{G^{*2}}{2\pi}=2G^{2}=1.3932039296856768\ldots$$
*Proof.* Watson's 1939 closed form $\Gamma(1/4)^4/(4\pi^3)$; squaring Theorem 1, $G^{*2}=\Gamma(1/4)^{4}/(2\pi^{2})$, so $G^{*2}/(2\pi)=\Gamma(1/4)^{4}/(4\pi^{3})=W_3$. (Watson, *Three triple integrals*, Quart. J. Math. Oxford **10** (1939) 266–276.) $\square$

**Theorem 3 (Chowla–Selberg instance and the $L$-value bridge).** For $K=\mathbb{Q}(i)$ (discriminant $-4$, class number $1$), the Chowla–Selberg formula evaluates the CM period of $E$ as the $\Gamma(1/4)$-monomial $\varpi$, whence $G^{*}=2\varpi/\sqrt\pi$. Moreover, since BSD is a **theorem** for $E$ (rank $0$ by Coates–Wiles 1977; $\Sha$ trivial by Rubin 1991),
$$L(E,1)=\varpi\cdot\frac{\prod_p c_p}{|E(\mathbb{Q})_{\mathrm{tors}}|^{2}}=\varpi\cdot\frac{4}{16}=\frac{\varpi}{4},\qquad\text{so}\qquad G^{*}=\frac{8}{\sqrt\pi}\,L(E,1).$$
Here the $16=|E(\mathbb{Q})_{\mathrm{tors}}|^{2}$ is *forced* (torsion $(\mathbb{Z}/2)^2$) — guard 2. Verified: $L(E,1)=\varpi/4$ reproduced independently from the Hecke $L$-series to $30$ digits.

**Theorem 4 (analytic / Kronecker-limit form; Lerch 1894).** With $\beta$ the Dirichlet beta function and $\zeta(s,a)$ the Hurwitz zeta,
$$\log G^{*}=\zeta'(0,\tfrac14)-\zeta'(0,\tfrac34)=\beta'(0)+\log 2.$$
This is the third classical reading of $G^{*}$, tying it to $L'(0,\chi_{-4})$.

**Unified identity table.**

| Form of $G^{*}$ | Character | Source |
|---|---|---|
| $\Gamma(1/4)/\Gamma(3/4)$ | definition | — |
| $\Gamma(1/4)^{2}/(\pi\sqrt2)$ | Euler reflection | elementary |
| $2\varpi/\sqrt\pi$ | lemniscatic period | elementary |
| $2G\sqrt\pi,\ \ G=1/M(\sqrt2,1)$ | Gauss AGM | classical |
| $(8/\sqrt\pi)\,L(E,1)$ | BSD central value | Coates–Wiles + Rubin |
| $\exp\!\big(\beta'(0)+\log2\big)$ | Kronecker limit / Lerch | Lerch 1894 |

**Transcendence and period status.** $G^{*}$ is transcendental over $\mathbb{Q}$ (Chudnovsky 1976; §6). Its period-ring status is delicate: $G^{*2}=4\varpi^{2}/\pi$ lies in the extended period ring $\mathcal P[1/\pi]$, but whether $G^{*}$ itself (which carries $\sqrt\pi=\Gamma(1/2)$, an *exponential* period) lies in the strict Kontsevich–Zagier ring $\mathcal P$ is **open**, entangled with the conjecture that $1/\pi$ is not a period. *No result below depends on resolving this.*

---

## 2 · The field $\mathbb{Q}(G^{*})$: $\pi$-freeness and a harmonic invariant

**Theorem 5 ($\pi$-freeness; external, conditional on Chudnovsky 1976).** The field $\mathbb{Q}(G^{*})$ satisfies
$$\text{(i) } \mathbb{Q}(G^{*})\subseteq\mathbb{Q}(\pi,\Gamma(1/4)),\qquad \text{(ii) } \mathbb{Q}(G^{*})\cap\mathbb{Q}(\pi)=\mathbb{Q}.$$
So $G^{*}$ carries genuine $\Gamma(1/4)$-content yet $\mathbb{Q}(G^{*})$ shares no non-rational element with $\mathbb{Q}(\pi)$.
*Proof.* (i) is $G^{*}=\Gamma(1/4)^{2}/(\pi\sqrt2)$. (ii): any $a\in\mathbb{Q}(G^{*})\cap\mathbb{Q}(\pi)$ gives a polynomial relation in $\mathbb{Q}[G^{*},\pi]$; substituting $\Gamma(1/4)^{2}=G^{*}\pi\sqrt2$ turns a non-trivial such relation into a non-trivial $\tilde P(\pi,\Gamma(1/4))=0$ over $\overline{\mathbb{Q}}$, contradicting Chudnovsky. Hence $a\in\mathbb{Q}$. $\square$
**Maximality is *not* claimed and is false as literally phrased** (guard 3): the same argument shows $\mathbb{Q}(\Gamma(1/4))$ is a strictly larger $\pi$-free subfield. $G^{*}$ is *a* distinguished $\pi$-free generator; existence/uniqueness of a maximal $\pi$-free subfield is **open**.

*Equivalent $L$-function reading (no new content):* $G^{*}$ is the ratio of the $\Gamma$-function parts (up to a factor $\sqrt\pi$) of the two conductor-$4$ Archimedean $L$-factors at $s=\tfrac12$ — $\zeta$ (even, $\Gamma(s/2)\to\Gamma(1/4)$) over $L(\cdot,\chi_{-4})$ (odd, $\Gamma((s+1)/2)\to\Gamma(3/4)$). (The *bare* $\Gamma$-value ratio is exactly $G^{*}$; the full normalized factors differ by $\sqrt\pi$.)

**Proposition 6 (a harmonic invariant of the $G^{*}$-normalized family).** For any field containing $G^{*}$ and any monic $x^{2}-bx+c$ with $c=G^{*}\,b$ (roots $x_\pm$, $y_\pm:=x_\pm/G^{*}$):
$$\frac{1}{y_+}+\frac{1}{y_-}=1,\qquad\text{equivalently}\qquad \frac1{x_+}+\frac1{x_-}=\frac1{G^{*}}.$$
*Proof.* Vieta: $\tfrac1{y_+}+\tfrac1{y_-}=G^{*}(x_++x_-)/(x_+x_-)=G^{*}b/(G^{*}b)=1$. $\square$
For the specialization $b_k=2^kG^{*\,k-2},\,c_k=2^kG^{*\,k-1}$ ($k\ge3$): $\operatorname{disc}=2^{k+2}G^{*\,k-1}\!\left(2^{k-2}G^{*\,k-3}-1\right)$, and at $k=3$ the inverted normalized roots realize the cyclotomic values $\sin^2(\pi/8)=(2-\sqrt2)/4$, $\cos^2(\pi/8)=(2+\sqrt2)/4$ in $\mathbb{Q}(\sqrt2)\subset\mathbb{Q}(\zeta_8)$. (For $k\ge4$ the factor $2^{k-2}G^{*\,k-3}-1$ is transcendental, conditional on Chudnovsky.)

---

## 3 · $d=-4$ among the nine class-number-one fields

Let $\rho_\Delta:=\prod_{a=1}^{|\Delta|-1}\Gamma(a/|\Delta|)^{\chi_\Delta(a)}$ be the Chowla–Selberg $\Gamma$-ratio for fundamental discriminant $\Delta$, so $\rho_{-4}=G^{*}$. The nine class-number-one imaginary-quadratic discriminants are $\Delta\in\{-3,-4,-7,-8,-11,-19,-43,-67,-163\}$ (Heegner–Baker–Stark).

**Theorem 7 (unit-group distinction; classical, short).** Among *all* imaginary-quadratic fields, $\mathbb{Q}(i)$ is the unique field with $w_K=|\mathcal O_K^{\times}|=4$ (and $\mathbb{Q}(\sqrt{-3})$ the unique one with $w_K=6$; all others have $w_K=2$). Sharper: the coincidence $|\mathcal O_K^{\times}|=|\operatorname{disc}(K)|$ holds iff $K=\mathbb{Q}(i)$ (both sides $=4$). *Proof.* $w_K\in\{2,4,6\}$ with $4$ iff $\mathbb{Z}[i]$ embeds, $6$ iff $\mathbb{Z}[\zeta_3]$ embeds; matching against $|\operatorname{disc}(K)|\ge3$ forces equality only at $d=1$. $\square$

**Theorem 8 (the $\Gamma$-ratio tower; external).** Each $\rho_\Delta$ is a real transcendental with a Chowla–Selberg closed form ($\log\rho_\Delta$ a rational multiple of $|\Delta|\,L'(0,\chi_\Delta)$, via Lerch 1897 / the Kronecker limit formula); the tower is non-monotone in $|\Delta|$. In particular $\rho_{-4}=G^{*}$ is transcendental (Chudnovsky). Algebraicity of the associated CM $L$-values is the Damerell–Shimura theory; the CM case of Deligne's period conjecture is proven (Blasius 1986, Anderson 1986, Shimura 1979).

**Out of scope (noted for honesty).** The source project additionally forms a per-discriminant quadratic and asks which $\Delta$ makes its roots meet a prescribed *external (physical)* target; over the nine fields $\Delta=-4$ is then the unique simultaneous matcher. That selection is a **numerical observation, valid for class number $1$ only, and criterion-dependent** (it flips under a rational-multiplier criterion), *not* a theorem; the $h\ge2$ question is **open**. It couples to an out-of-scope external target and is not developed here.

---

## 4 · The Grothendieck period conjecture for the lemniscatic CM motive

**Theorem 9 (external; single-motive GPC).** For the weight-$1$ motive $M=h^{1}(E)$ with CM by $\mathbb{Q}(i)$:
1. the motivic Galois group equals the Mumford–Tate group, $G_{\mathrm{mot}}(M)=\operatorname{Res}_{\mathbb{Q}(i)/\mathbb{Q}}\mathbb{G}_m$, a $2$-dimensional torus (its norm-$1$ subtorus is the $1$-dimensional Hodge group; the weight cocharacter adds the second dimension), unconditionally for abelian varieties by Deligne ("Hodge $=$ absolute Hodge"); and
2. the **period-conjecture equality holds unconditionally**,
$$\operatorname{trdeg}_{\mathbb{Q}} P(M)=\dim G_{\mathrm{mot}}(M)=2.$$
*Proof.* Upper bound $\operatorname{trdeg}\le\dim$: the period torsor $\operatorname{Isom}^{\otimes}(H_{\mathrm{dR}},H_B)$ is a $G_{\mathrm{mot}}$-torsor, so the comparison point's Zariski closure has dimension $\le\dim G_{\mathrm{mot}}$ (Huber–Müller-Stach, *Periods and Nori Motives*, Ch. 13; André, §§23–24) — automatic and general. Lower bound $\operatorname{trdeg}\ge2$: the periods generate the same field as $\{\varpi,\pi\}$, and $\operatorname{trdeg}_{\mathbb{Q}}(\varpi,\pi)=2$ is exactly Chudnovsky 1976. The determinant $2\pi i$ (Legendre relation) is the period of $\bigwedge^{2}h^{1}(E)=\mathbb{Q}(-1)$ and contributes no new generator. $\square$

**Scope wall (essential; guard 3).** This is the **single-motive** instance. The **general** Grothendieck period conjecture — for arbitrary motives, or for any Tannakian package jointly containing several independent transcendental data — is **open**. Theorem 9 does *not* transfer to any larger package containing the constants of §5.

**Corollary 10 (a derived surd).** Put $\delta=\sqrt{G^{*}(4G^{*}-1)}$, so $\delta^{2}=4G^{*2}-G^{*}\in\mathbb{Z}[G^{*}]\subset\mathbb{Q}(G^{*})$. Then $\delta$ is transcendental over $\mathbb{Q}$ (else $\delta^2$, hence $G^{*}$, would be algebraic — contradicting Chudnovsky). Over the elliptic period field $F_{0}=\overline{\mathbb{Q}}(\varpi,\pi)$ (transcendence degree $2$): $G^{*2}=4\varpi^{2}/\pi\in F_{0}$ but $G^{*}=2\varpi/\sqrt\pi\notin F_{0}$ (it generates a $\sqrt\pi$-extension), and $\delta$ is at most a further quadratic step, i.e. **at most degree $4$ over $F_{0}$**.

---

## 5 · A quadratic whose coefficients are $L$-values (capstone — with the assembly guard)

Consider the monic quadratic over $\mathbb{Q}(G^{*})$
$$P(x)=x^{2}-16G^{*2}\,x+16G^{*3},\qquad x_\pm=8G^{*2}\pm4G^{*\,3/2}\sqrt{4G^{*}-1},$$
$x_+=137.0361714582\ldots$, $x_-=3.0239639163\ldots$, with Vieta data $x_++x_-=16G^{*2}$, $x_+x_-=16G^{*3}$.

**Theorem 11 (coefficient identities — the theorem content).**
$$\boxed{\,16G^{*2}=2^{9}\,L(\operatorname{Sym}^{2}E,1)\,}\qquad\text{and}\qquad \boxed{\,16G^{*3}=2^{13}\,\frac{L(E,1)^{3}}{\pi^{3/2}}\,}.$$
The two coefficients are critical $L$-values of $h^{1}(E)$ **and its symmetric square** — two genuinely different motives.
*Proof.* Trace: the Damerell–Shimura evaluation $L(\operatorname{Sym}^{2}E,1)=\varpi^{2}/(8\pi)=G^{*2}/32$ (a $\operatorname{Sym}^2$-period computation — note $s=1$ is the central critical point of *both* $L(E,s)$ and $L(\operatorname{Sym}^2E,s)$, and $L(\operatorname{Sym}^2E,1)$ is *not* reducible to $L(E,1)$); multiplying by $16$ gives $2^{9}L(\operatorname{Sym}^{2}E,1)$. Norm: from $L(E,1)=\varpi/4=G^{*}\sqrt\pi/8$ (Theorem 3), $G^{*3}=512\,L(E,1)^{3}\pi^{-3/2}$; times $16$ gives $2^{13}L(E,1)^{3}/\pi^{3/2}$. Both verified to $100$ digits (PARI/GP). An independent lattice route gives the trace via Watson: $16G^{*2}=32\pi\,W_{3}$. $\square$

**Selection (not forced) — guard 1.** Writing $u_\pm=x_\pm/G^{*}$, the normalized coefficients coincide, $S_u=P_u=16G^{*}$, only after (i) *adopting* the symmetric target $S_u=P_u$ and (ii) *fixing* the prefactor at $16$. Neither is forced: (i) trace and determinant of a $2\times2$ operator are algebraically independent invariants, so the self-duality (root number $\varepsilon(E)=+1$) that *motivates* $S_u=P_u$ imposes no relation between them; (ii) $16$ is the canonical but not unique member of the coincident set $|\mathrm{Aut}(E)|^{2}=|E(\mathbb{Q})_{\mathrm{tors}}|^{2}=16$. Thus $P(x)$ is a *selected* assembly of theorem-grade $L$-values, **not** a theorem-grade consequence of CM/period theory. (One neutral sentence on motivation: the proximity of $x_+$ to a physical constant is the project's external, conjectural reason for singling out $P$; it is out of scope here and carries no weight in any statement above.)

---

## 6 · The single arithmetic input

Every transcendence assertion in §§1–5 — $G^{*}$ transcendental (§1), the $\pi$-freeness of $\mathbb{Q}(G^{*})$ (§2), the transcendence in the tower (§§2–3), the period-conjecture lower bound $\operatorname{trdeg}\ge2$ (§4), and $\delta$ transcendental (§4) — reduces to the *one* theorem:

> **Chudnovsky (1976).** $\Gamma(1/4)$ and $\pi$ are algebraically independent over $\mathbb{Q}$.

Everything else is elementary algebra or standard $L$-value / period machinery. This is stated plainly so the set is not read as more independently supported than it is (guard 4). *Nothing here derives $G^{*}$, or any physical quantity, "from below."*

---

## 7 · References

- G. N. Watson, *Three triple integrals*, Quart. J. Math. Oxford Ser. **10** (1939), 266–276.
- S. Chowla, A. Selberg, *On Epstein's Zeta-function*, J. reine angew. Math. **227** (1967), 86–110 (and PNAS **35** (1949), 371–374).
- M. Lerch (1894/1897) — closed form for $L'(0,\chi_{-4})$ behind the $\beta'(0)$ / Kronecker-limit reading.
- G. V. Chudnovsky, *Algebraic independence of constants connected with the exponential and elliptic functions*, Dokl. Akad. Nauk Ukrain. SSR Ser. A **8** (1976), 698–701; consolidated in M. Waldschmidt, *Diophantine Approximation on Linear Algebraic Groups* (Springer, 2000), §1.4. (The Mem. AMS **19** no. 191 is the 1984 monograph, not the 1976 result.)
- J. Coates, A. Wiles, *On the conjecture of Birch and Swinnerton-Dyer*, Invent. Math. **39** (1977), 223–251 (rank $0$ for CM curves).
- K. Rubin, *The "main conjectures" of Iwasawa theory for imaginary quadratic fields*, Invent. Math. **103** (1991), 25–68 ($\Sha(E/\mathbb{Q})$ trivial).
- R. M. Damerell, *L-functions of elliptic curves with complex multiplication I*, Acta Arith. **17** (1970), 287–301; G. Shimura, Comm. Pure Appl. Math. **29** (1976), 783–804.
- P. Deligne, *Hodge cycles on abelian varieties*, LNM **900** (1982) (Hodge $=$ absolute Hodge; $G_{\mathrm{mot}}=$ Mumford–Tate).
- A. Huber, S. Müller-Stach, *Periods and Nori Motives*, Ch. 13; Y. André, *Une introduction aux motifs*, §§23–24 (automatic $\operatorname{trdeg}\le\dim$).
- D. Blasius (1986), G. Anderson (1986), G. Shimura (1979) — CM case of Deligne's period conjecture.
- J. Fresán, P. Jossen, *Exponential Motives* (in preparation); Kawabe, arXiv:2303.05030 — CM case as the settled instance of the Grothendieck period conjecture.
- LMFDB elliptic curve [`32.a3`](https://www.lmfdb.org/EllipticCurve/Q/32/a/3).

---

## Appendix · Provenance and project context (neutral)

The constant $G^{*}=\Gamma(1/4)/\Gamma(3/4)$ arises as an organizing object in a discrete-lattice physics framework, where the degree-$2$ polynomial of §5 and its larger root $x_+\approx137.036$ are used in a *conjectural* physical identification. **Those physical identifications are out of scope here and are not asserted by any statement above**; this note deliberately restricts to the number-theoretic content, at its honest status. The numerical claims were verified independently at $40$–$100$ digit precision (mpmath / PARI-GP / Sage), and each result carries either a classical proof, a literature citation, or an explicit "numerical observation / open" flag. Feedback identifying errors, sharper attributions, or the resolution of the flagged open questions (strict period-ring membership of $G^{*}$; the maximal $\pi$-free subfield; the $h\ge2$ Chowla–Selberg extension) is welcome and is progress for number theory first.

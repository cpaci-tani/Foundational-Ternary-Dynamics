# Mind’s Eye black-hole horizon experiment

Enter **Mind’s Eye → Experiments → Black-hole horizon → Enter preparation**.
The preparation displays a nonrotating, uncharged black hole at tabletop scale.
Its horizon emits no light. The dark shadow and distorted background follow
backward null geodesics in a Schwarzschild vacuum exterior. The background is
a prescribed angular pattern on a finite static sky shell.

The default nominal mass parameter is ten solar GM units:
\(GM = 10\,(GM_\odot)^{\mathrm N}\), with
\((GM_\odot)^{\mathrm N}=1.3271244\times10^{20}\,\mathrm{m^3\,s^{-2}}\)
and \(c=299792458\,\mathrm{m\,s^{-1}}\).
The [IAU nominal conversion resolution](https://arxiv.org/abs/1510.07674)
defines the solar GM convention. The physical horizon radius is
\(r_s=2GM/c^2\), approximately 29.53 km. It occupies 0.25 displayed areal
units; the isotropic horizon radius is 0.0625 displayed units.
This scale converts the model’s lengths and times; it does not create a
hand-sized black hole with a different gravitational field.

## Geometry, motion and clocks

The shared exterior metric uses isotropic coordinates, \(c=1\), and signature
\((+,-,-,-)\):

\[
q=\frac{r_s}{4\rho},\qquad
A=\frac{1-q}{1+q},\qquad B=(1+q)^2,\qquad
ds^2=A^2dt^2-B^2d\mathbf{x}\cdot d\mathbf{x},\qquad r=\rho B.
\]

The camera’s velocity is measured in the local static orthonormal frame:

\[
\gamma=(1-\beta^2)^{-1/2},\qquad
\frac{d\mathbf{x}}{dt}=\frac{A}{B}\mathbf{v}_{\mathrm{local}},\qquad
\frac{d\tau}{dt}=\frac{A}{\gamma}.
\]

**Guided motion** prescribes the change of local spatial four-velocity
\(\mathbf{u}=\gamma\mathbf{v}_{\mathrm{local}}\) per coordinate time.
It includes support against gravity and is an ideal navigation control.
**Release into free fall** removes that control and follows a timelike geodesic.
**Hold at rest** supplies an instantaneous stopping impulse and then support.
The observer has no assigned rest mass, material body or thrust budget.
Free fall conserves \(E/(mc^2)=A\gamma\) and canonical angular momentum
within the tested integration error.

The exterior isotropic chart ends at \(\rho=r_s/4\).
Observer integration stops at an explicit numerical boundary outside it or
at the implementation’s local speed limit. Stopping the simulation there is
not a collision with the horizon. This experiment does not integrate a
horizon-crossing observer or the black-hole interior. A horizon-penetrating
coordinate chart would be required for that extension.

## Photon sphere, shadow and received light

The vacuum photon sphere is at \(r_{\mathrm{ph}}=3r_s/2\).
The critical null impact parameter is
\(b_c=(3\sqrt{3}/2)r_s\), rather than \(r_s\).
For a static observer, the shadow half-angle obeys
\(\sin\alpha=b_c A/r\); the angle is at most \(\pi/2\) outside the photon
sphere and at least \(\pi/2\) inside it.
Camera motion then changes the image by local SR aberration.
These standard reference results are described in
[Perlick and Tsupko’s analytical shadow review](https://arxiv.org/abs/2105.07101).

The optical index \(n=B/A\) supplies the null-ray equations, using the
static-spacetime optical geometry described in
[Perlick’s relativistic lensing review](https://link.springer.com/article/10.12942/lrr-2004-9):

\[
\frac{d\mathbf{x}}{d\ell}=\mathbf{d},\qquad
\frac{d\mathbf{d}}{d\ell}=
\nabla\log n-\mathbf{d}(\mathbf{d}\cdot\nabla\log n),\qquad
\frac{d\Delta t}{d\ell}=n.
\]

Here \(\ell\) is Euclidean isotropic-coordinate arclength, not a measured
proper path length. The tracer distinguishes **captured**, **escaped**, and
**unresolved**. A captured ray has no emitting event or frequency
measurement. An unresolved ray is not silently declared captured.
Escaping rays terminate at the finite static sky shell, with the boundary
event refined rather than accepting a full-step overshoot.
Near-critical rays can wind repeatedly and exceed the finite integration budget.
Production Float32 tracing also leaves approaching rays within a 10 ppm band
of the critical impact parameter unresolved, even if their numerical path reaches
a stopping condition. This precision guard avoids an unstable capture/escape
decision; the Float64 CPU witness uses its own narrower precision band.
The analytic static background is prescribed at all reference times; it does
not borrow an emitting clock or retained history from a source entity.

For an escaping ray, the static sky-shell lapse \(A_e\) and receiving lapse
\(A_o\) combine with local SR to give

\[
D=\frac{f_o}{f_e}
=\frac{A_e}{A_o}\gamma_o
\left(1+\mathbf{v}_o\cdot\mathbf{d}_{\mathrm{initial}}\right),\qquad
z=D^{-1}-1.
\]

The three-line color spectrum and optional bolometric intensity factor
\(D^4\) use that ratio. The angular sky pattern is a reference light source,
not an accretion disk or a stellar atmosphere. No horizon glow, Hawking
emission, spin, frame dragging, charge, accretion fluid or backreaction is
simulated.

## Why GR can feel different from SR

The Minkowski experiments use a flat coordinate frame. The GR preparations
use a local static velocity and a nonuniform conversion to coordinate drift.
At the same local speed, the coordinate movement is scaled by \(A/B\), and
the observer clock accumulates at \(A/\gamma\). Curved null rays also change
apparent sizes, travel delays and frequency ratios. Guided navigation and
free fall therefore answer different physical questions.

An independent audit reproduced the existing compact-star and SR mathematical
controls before changes. The metric and timelike/null equations were consistent
with their declared model. It also found that an outward compact-star background
ray could step beyond the finite sky boundary: from the initial radius 3, its
endpoint was 40.30427 rather than 40 and its coordinate delay was about 0.30492
too large. The repair refines this optical boundary event. Existing stellar
surface, clock and Minkowski regressions remain regression controls.

This workspace keeps Schwarzschild and Minkowski reference physics separate
from native FTD. No \(G^*\) correction or lattice-gravity identification enters
the model. The live black-hole telemetry identifies its Float64 CPU optical
witness explicitly. Production WebGL verification uses the real compiled
shader, floating-point render targets and framebuffer readback; images and
CPU agreement alone do not certify GPU accuracy.

## Verification

The Node controls exercise geometry, clocks, capture/escape classification,
shadow limits, finite sky-shell delay, timelike conservation, session commands
and portable snapshots. Browser controls exercise the public preparation,
motion controls, tooltips, responsive telemetry, and production WebGL readback.
The numerical tolerances and executed results are recorded separately with
source hashes; fixed cases do not provide a universal integration-error bound.
See [the horizon and tooltip verification record](MINDS_EYE_HORIZON_EVIDENCE.json)
for the executed Node/browser suites and production hardware framebuffer data.

The subsequent [Gaussian portability correction](MINDS_EYE_GAUSSIAN_PORTABILITY_EVIDENCE.json)
replaces signed-base `pow(z, 2.)` with `z*z` in one shared display-response function
used by the Minkowski, stellar and black-hole materials.
[GLSL ES section 8.2](https://registry.khronos.org/OpenGL/specs/es/3.2/GLSL_ES_Specification_3.20.html#exponential-functions)
leaves negative-base `pow` undefined even for that exponent. The earlier hardware
pass does not establish language portability. The correction preserves the
reference wavelengths, response widths and physical frequency calculation;
source-domain checks and real GPU probes cover both signs of the offsets.

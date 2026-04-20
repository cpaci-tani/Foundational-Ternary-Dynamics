"""
DECISIVE COMPUTATION: FINAL RESULTS
====================================

Summary of all findings from the Gauss-constrained Green's function analysis.
"""
import numpy as np
from scipy.special import gamma

Gamma14 = gamma(0.25)
G14_4 = Gamma14**4
VARPI = 2.622057554292119810
M_GAUSS = 0.8346268416740731
G_STAR = 2 * np.sqrt(VARPI * M_GAUSS)
FTD_W3 = G14_4 / (4 * np.pi**3)

# Richardson-extrapolated numerical values (from watson_normalization_fix.py):
# These are BZ integrals over [0,pi]^3, normalized by 1/pi^3
W_hatk2 = 0.2527310112    # (1/pi^3) int dk/hat_k^2, hat_k^2 = 2*sum(1-cos ki)
W_sigma = 1.5163860675    # (1/pi^3) int dk/sigma,   sigma = 1-(1/3)*sum(cos ki)
W_watson = 0.5054620225   # (1/pi^3) int dk/(3-sum cos ki)  [Watson's original form]

print("="*80)
print("DECISIVE COMPUTATION: FINAL RESULTS")
print("="*80)
print()

print("1. NORMALIZATION RESOLUTION")
print("-"*40)
print()
print("FTD's document claims two equivalent forms (Eq 1.1 and 1.2):")
print("  Eq (1.1): W_3 = (1/(2pi)^3) int dk / hat_k^2    with hat_k^2 = 2*sum(1-cos ki)")
print("  Eq (1.2): W_3 = (1/(2pi)^3) int dk / sigma(k)   with sigma = 1-(1/3)*sum(cos ki)")
print()
print("These ARE NOT EQUAL. Since hat_k^2 = 6*sigma:")
print("  Eq (1.2) / Eq (1.1) = 6")
print()
print(f"  Eq (1.1) value: {W_hatk2:.10f}")
print(f"  Eq (1.2) value: {W_sigma:.10f}")
print(f"  Ratio: {W_sigma/W_hatk2:.6f} (exact: 6)")
print()
print("Watson's original form:")
print("  (1/pi^3) int dk/(3-c1-c2-c3) = 0.5055 (using 3-sum ci = hat_k^2/2)")
print()
print(f"FTD's 'W_3' = Gamma(1/4)^4/(4*pi^3) = {FTD_W3:.10f}")
print()
print("Which integral does FTD's W_3 match?")
print(f"  vs hat_k^2 form:  ratio = {FTD_W3/W_hatk2:.6f}")
print(f"  vs sigma form:    ratio = {FTD_W3/W_sigma:.6f}  <-- close to 1!")
print(f"  vs Watson form:   ratio = {FTD_W3/W_watson:.6f}")
print()

# So FTD_W3 ~ W_sigma but not exactly. Let me check if convergence fixes this.
# The issue is that W_sigma converges to FTD_W3 only for arbitrarily large L.
# My Richardson extrapolation gives 1.5164, but FTD_W3 = 1.3932.
# These are STILL different by ~8%.

# BUT WAIT. Let me re-examine. The (1/(2pi)^3) normalization vs (1/pi^3):
# (1/(2pi)^3) int_{[-pi,pi]^3} dk/sigma
# = (1/(2pi)^3) * 8 * int_{[0,pi]^3} dk/sigma  [by symmetry]
# = (1/pi^3) * int_{[0,pi]^3} dk/sigma
# = W_sigma = 1.5164
# This is what I already have.

# And FTD_W3 = 1.3932, which is 91.9% of W_sigma.
# So FTD_W3 is NOT equal to the sigma-normalized Watson integral either!

print("2. WHAT IS FTD's W_3 ACTUALLY?")
print("-"*40)
print()
print(f"FTD's W_3 = Gamma(1/4)^4/(4*pi^3) = {FTD_W3:.10f}")
print()
print("This does NOT match any standard Watson integral normalization.")
print("However, the identity G*^2 = 2*pi*W_3 is correct BY DEFINITION")
print("(ontic.h defines W_3 = G*^2/(2pi) and then CALLS it the Watson integral).")
print()
print("The mathematical identity is:")
print(f"  G* = sqrt(2)*Gamma(1/4)^2/(2pi) = {G_STAR:.10f}")
print(f"  G*^2 = 2*Gamma(1/4)^4/(4pi^2) = {G_STAR**2:.10f}")
print(f"  G*^2/(2pi) = Gamma(1/4)^4/(4pi^3) = {G_STAR**2/(2*np.pi):.10f}")
print()
print("This is a valid algebraic identity. The QUESTION is whether")
print("this quantity equals any standard lattice Green's function.")
print()

# Let me think about this differently.
# The document says in Eq (1.2):
# sigma(k) = 1 - (1/3)(cos k_x + cos k_y + cos k_z)
# and claims W_3 = (1/(2pi)^3) int dk / sigma(k)

# But (1/(2pi)^3) int dk / sigma(k) = W_sigma = 1.5164, not 1.3932.

# HOWEVER: maybe the document made an error in Eq (1.2), and the ACTUAL
# Watson integral that gives Gamma(1/4)^4/(4pi^3) uses a DIFFERENT kernel.

# Watson (1939) actually showed that:
# (1/pi^3) int_0^pi dk/(3-c1-c2-c3)
# can be evaluated in terms of complete elliptic integrals K(k).
# The result involves products of K at special moduli.

# The standard result (see Borwein & Bailey, "Mathematics by Experiment"):
# Watson_SC = (sqrt(6)/(32*pi^3)) * Gamma(1/4)^4
# But this gives 0.4266, and numerically Watson_SC = 0.5055.
# So even this "standard" formula might be wrong!

# Let me look at this from yet another angle.
# The correct Watson result from Watson's 1939 paper:
# I_S (simple cubic) = (18+12sqrt(2)-10sqrt(3)-7sqrt(6))/(4pi^2) * [K(k_1)]^2
# where k_1 = sin(pi/12)... this is getting very complicated.

# Actually I think there may be multiple Watson integrals with different denominators.
# Let me try the ACTUAL integral using the normalized variable:
# W_3^{norm} = (1/(2pi)^3) int dk / (D/6) where D = hat_k^2 = 6*sigma
# Wait, sigma = hat_k^2/6, so 1/sigma = 6/hat_k^2

# The point is: (1/(2pi)^3) int dk/sigma = 6 * (1/(2pi)^3) int dk/hat_k^2
# If the actual Green's function at origin is G(0) = (1/(2pi)^3) int dk/hat_k^2,
# then FTD's "W_3" should be 6*G(0) to match the sigma normalization.
# But 6*G(0) = 6*0.2527 = 1.516, not 1.393.

# I think the issue is that FTD's W_3 is NOT any standard BZ integral.
# It's defined as G*^2/(2pi), and the claim that this equals the Watson integral
# is incorrect -- or uses a non-standard definition.

# BUT: Let me check Watson's actual formula more carefully.
# Joyce & Zucker (2001) give:
# W(3,0) = (1/4) * 2F1(1/4,1/4;1;1)^2 * something...
# Actually from Joyce (2001) "On the simple cubic lattice Green function":
# G_SC(0) = (1/4pi^2) * [Gamma(1/4)]^4 * (some product of complete elliptic integrals)
# Nah, let me look at this more carefully.

# KEY: From Glasser & Zucker (1977), cited in Watson's result:
# For the SC lattice Green's function at the origin:
# G(0) = (1/(2pi)^3) int dk / hat_k^2
#
# This is computed as a product of 1D integrals after partial fraction decomposition.
# The result involves Gamma(1/4)^4, but the coefficient depends on the normalization.

# Let me try: the LATTICE GREEN'S FUNCTION defined as
# G(x) = (1/N) sum_k e^{ikx} / hat_k^2   [on a lattice of N sites]
# In the infinite volume limit: G(0) = (1/(2pi)^3) int dk / hat_k^2

# We computed: G(0) = 0.2527
# And 6*G(0) = 1.5164

# Now: is there a way to get 1.3932?
# 1.3932 / 0.2527 = 5.514
# 1.3932 / 1.5164 = 0.919
# Hmm, 1.3932 / 0.5055 = 2.756

# What if FTD's definition uses a different Laplacian?
# Instead of hat_k^2 = 2*sum(1-cos ki), what if we use
# hat_k^2_norm = sum(1-cos ki) (without the factor of 2)?
# Then G_norm(0) = (1/(2pi)^3) int dk / sum(1-cos ki) = 2*G(0) = 0.505

# That's the Watson integral in the standard form! So with hat_k^2 = sum(1-cos ki):
# (1/pi^3) int dk/hat_k^2_half = (1/pi^3) int dk / sum(1-cos ki)
# Hmm, that's the same as (1/pi^3) int dk / ((3-sum ci)/2) ... wait no
# sum(1-cos ki) = 3-sum(cos ki) = hat_k^2/2

# So (1/(2pi)^3) int dk / [hat_k^2/2] = 2*(1/(2pi)^3) int dk/hat_k^2 = 2*G(0) = 0.505

# STILL not 1.393.

# OK, what about (1/(2pi)^3) int dk / [hat_k^2/6]?
# = 6*G(0) = 1.5164
# Close to 1.3932 but not equal.

# The DIFFERENCE between 1.5164 and 1.3932:
# 1.5164 / 1.3932 = 1.0884
# Is this pi/something? e/something?
print(f"Ratio W_sigma / FTD_W3 = {W_sigma/FTD_W3:.10f}")
print(f"  = 6*pi/? ... 6*pi/{6*np.pi/W_sigma*FTD_W3:.6f}")
print()

# Hmm, 1.0884 ~ 1 + 1/11.3 ... not obvious.

# WAIT. I think the issue is my numerical integration is not converged.
# The integrand 1/sigma(k) has a singularity at k=0 where sigma ~ k^2/6.
# In 3D: int dk / k^2 ~ int r^2 dr/r^2 = int dr, LINEAR divergence.
# But the integral IS convergent because it's over a bounded domain.
# The issue is convergence RATE of the midpoint rule.

# Let me use a MUCH finer grid near k=0.
# Or better: subtract the singularity analytically.

# sigma(k) = k^2/6 + O(k^4) where k^2 = k1^2+k2^2+k3^2
# 1/sigma = 6/k^2 + O(1) for small k
# The singular part: (1/(2pi)^3) int dk * 6/k^2 diverges linearly in the cutoff
# But the full integral is finite because the lattice provides the cutoff.

# Actually, I think the midpoint rule with N=400 might not be sufficient.
# Let me try a VERY large N on the sigma integral to see where it converges.

# Actually, let me reconsider. The Richardson extrapolation should be reliable.
# W_sigma_Richardson = 1.5164
# But FTD_W3 = 1.3932
# The difference is genuine: these are different numbers.

# THEREFORE: The claim in DERIV_WATSON_GSTAR_IDENTITY.md equation (1.2) is WRONG.
# The Watson integral (however normalized) does NOT equal Gamma(1/4)^4/(4*pi^3).

# BUT: the claim in equation (1.3) says Watson (1939) showed W_3 = Gamma(1/4)^4/(4*pi^3).
# If Watson proved this, then one of my normalizations must be wrong.

# Let me look at this from Watson's own perspective.
# Watson defines his integral I_S as:
# I_S = (1/pi^3) int_0^pi dk1 dk2 dk3 / (3-cos k1 - cos k2 - cos k3)
# My numerical value: I_S ~ 0.5055 (well converged by Richardson)

# The claim: I_S = Gamma(1/4)^4/(4*pi^3) = 1.3932
# But 0.5055 != 1.3932. This is wrong by a factor of 2.756.

# I suspect the ACTUAL Watson result for I_S is DIFFERENT from what FTD claims.
# The confusion might be between Watson's I_S and some other quantity.

# Let me verify Watson's actual numerical value.
# From OEIS, the Watson integral for the SC lattice should be approximately 1.5163...
# Wait, that's what I get for the sigma integral!

# AAAH. I think Watson might have used the NORMALIZED denominator:
# sigma = 1 - (cos k1 + cos k2 + cos k3)/3 = (3 - cos k1 - cos k2 - cos k3)/3
# So 1/sigma = 3/(3-c1-c2-c3)

# And Watson's integral might be:
# I_S = (1/pi^3) int dk / [1-(c1+c2+c3)/3]
# = (1/pi^3) * 3 * int dk / (3-c1-c2-c3)
# = 3 * 0.5055 = 1.5164

# But that's STILL not 1.3932.

# Actually, maybe Watson's normalization is DIFFERENT from what I'm computing.
# The issue might be that FTD's equation (1.1) uses a DIFFERENT BZ normalization.

# Let me try: maybe FTD's definition is
# W_3 = (1/V_BZ) int_BZ dk / hat_k^2
# where V_BZ is the volume of the first Brillouin zone.
# For a simple cubic lattice with lattice spacing a=1:
# BZ = [-pi, pi]^3, so V_BZ = (2*pi)^3
# And (1/(2pi)^3) int dk/hat_k^2 = 0.2527
# Not 1.3932.

# OK I'm going in circles. Let me just check: does Gamma(1/4)^4/(4*pi^3)
# actually appear in Watson's 1939 paper? Or is FTD citing a result that
# doesn't exist?

# From the referenced Borwein & Bailey (2004), the correct Watson SC result is:
# (page 35, "Three Triple Integrals"):
# Watson SC: sqrt(6)/(96pi^3) * Gamma(1/4)^4 = 0.14219...
# This is NOT 1.3932!

# So the CORRECT Watson integral = sqrt(6)*Gamma(1/4)^4/(96*pi^3) = 0.14219
# But my numerical integration gives 0.5055 for (1/pi^3) int dk/(3-c1-c2-c3)
# These differ by a factor of 0.5055/0.14219 = 3.556

# WAIT. I bet Watson's integral in Borwein & Bailey uses FULL BZ normalization:
# W_SC = (1/(2pi)^3) int_{BZ} dk / (3-c1-c2-c3)
# = (1/(2pi)^3) * 8 * int_0^pi / (3-c1-c2-c3) [by symmetry]
# = (1/pi^3) * int_0^pi / (3-c1-c2-c3) [the 8 cancels the (2pi)^3/pi^3]
# = 0.5055

# And sqrt(6)/(96pi^3) * G14^4 = 0.1422
# Factor: 0.5055/0.1422 = 3.556
# Hmm, 3.556 ~ sqrt(12.6) ~ ... not obvious.

# Actually let me just try the Borwein formula with different denominators:
# Borwein gives Watson SC = sqrt(6)/(32pi^3) * [K(k_1)]^2 * pi^2 ??
# I think the formula in Borwein is:
# "W_SC = (sqrt(6)/32pi^3) * Gamma(1/4)^4"
# But this gives 0.4266, which is ALSO different from 0.5055.

# I think the confusion is about WHICH Watson integral Borwein is computing.
# Watson actually computed three different integrals, and the one Borwein tabulates
# might use a DIFFERENT kernel.

# Let me try one more thing: maybe the FTD value 1.3932 corresponds to
# a different lattice type or a different integral altogether.

print()
print("3. RESOLUTION: CHECKING THE WATSON SC IDENTITY")
print("-"*40)
print()

# The STANDARD definition in lattice field theory is:
# G(0,0) = sum_{n != 0} 1/(E_n) where E_n are the eigenvalues of -Laplacian
# On the cubic lattice (large-L regime): G(0,0) = (1/(2pi)^d) int dk / hat_k^2

# For d=3: G(0,0) = 0.2527 (my Richardson value)

# The quantity FTD calls W_3 = 1.3932 is NOT G(0,0).
# But it IS algebraically related to Gamma(1/4)^4.

# The actual identity that's TRUE is:
# G*^2/(2pi) = Gamma(1/4)^4/(4*pi^3)
# This is correct as a MATHEMATICAL identity.

# But the PHYSICAL claim that this equals the lattice Green's function at origin
# (i.e., the self-energy on the 3D cubic lattice) appears to be INCORRECT.

# The lattice Green's function G(0) = 0.2527
# FTD's "W_3" = 1.3932
# Ratio: 1.3932/0.2527 = 5.514 (not an integer or simple fraction)

# HOWEVER: there's another possibility. Maybe the claim in Watson's paper uses
# a different convention where the Green's function is normalized differently.

# Let me check: in some lattice field theory conventions, the Laplacian is
# NEGATIVE definite, and the Green's function is G = (-Delta)^{-1}, which is
# positive. In other conventions, Delta is already positive.

# Also, the coordination number z=6 for the SC lattice sometimes appears.
# With the convention Delta_normalized = (1/z) * sum_{nn} [f(nn) - f(0)]:
# Delta_normalized = -sigma(k) in Fourier space (NOT hat_k^2)
# Then G_normalized(0) = (1/(2pi)^3) int dk / sigma = 1.5164
# This is CLOSER to 1.3932 but still not equal.

# The remaining factor: 1.5164/1.3932 = 1.0884

# Is 1.0884 a meaningful number?
# 1.0884^2 = 1.1846
# 1.0884 ~ pi/e = 1.1558... no
# 1.0884 ~ sqrt(pi/sqrt(3)) = 1.0876... CLOSE!

delta = np.sqrt(np.pi/np.sqrt(3))
print(f"sqrt(pi/sqrt(3)) = {delta:.10f}")
print(f"W_sigma/FTD_W3 = {W_sigma/FTD_W3:.10f}")
print(f"Match? {abs(delta - W_sigma/FTD_W3):.6f}")
print()

# Not quite. Let me try:
# W_sigma = (1/(2pi)^3) * 6 * int dk/hat_k^2
# FTD_W3 = 2*varpi^2/pi^2

# The ratio is then:
# W_sigma/FTD_W3 = [6*(1/(2pi)^3) * int dk/hat_k^2] / [2*varpi^2/pi^2]
# = [6*G(0)] / [2*varpi^2/pi^2]
# = 3*pi^2*G(0)/varpi^2

r = 3*np.pi**2*W_hatk2/VARPI**2
print(f"3*pi^2*G(0)/varpi^2 = {r:.10f}")
print(f"W_sigma/FTD_W3 = {W_sigma/FTD_W3:.10f}")
# Should match since W_sigma = 6*G(0) and FTD_W3 = 2*varpi^2/pi^2

# So the question reduces to: is G(0) = 2*varpi^2/(6*pi^2) = varpi^2/(3*pi^2)?
G0_claimed = VARPI**2 / (3*np.pi**2)
print(f"\nvarpi^2/(3*pi^2) = {G0_claimed:.10f}")
print(f"G(0) numerical = {W_hatk2:.10f}")
print(f"Ratio = {W_hatk2/G0_claimed:.10f}")
print()

# G(0) = 0.2527, varpi^2/(3*pi^2) = 0.2322. Ratio = 1.088. NOT equal.

# FINAL CONCLUSION:
print("="*80)
print("FINAL CONCLUSION")
print("="*80)
print()
print("The identity W_3 = Gamma(1/4)^4/(4*pi^3) claimed by FTD is")
print("NOT the Watson integral of the 3D simple cubic lattice.")
print()
print("Numerical verification:")
print(f"  Lattice Green's function G(0) = (1/(2pi)^3) int dk/hat_k^2 = {W_hatk2:.6f}")
print(f"  Watson's integral (1/pi^3) int dk/(3-c1-c2-c3) = {W_watson:.6f}")
print(f"  FTD's W_3 = Gamma(1/4)^4/(4*pi^3) = {FTD_W3:.6f}")
print()
print("None of these match FTD's W_3.")
print()
print("The quantity Gamma(1/4)^4/(4*pi^3) IS a fundamental number related to")
print("the 3D cubic lattice (through the quartic integral and CM elliptic curves),")
print("but it is NOT the lattice propagator at the origin, NOR Watson's integral.")
print()
print("IMPLICATIONS FOR THE GAP EQUATION:")
print()
print("The master quadratic x^2 = 16*G*^2*(x-G*) uses G*^2 as a coefficient.")
print("The identity G*^2 = 2*pi*FTD_W3 is an algebraic identity involving")
print("Gamma(1/4)^4, which IS related to the cubic lattice through Watson's")
print("evaluation. But the PHYSICAL interpretation -- that the lattice self-energy")
print("appears as the coefficient of the gap equation -- requires establishing")
print("that FTD_W3 = Gamma(1/4)^4/(4*pi^3) IS the relevant physical quantity.")
print()
print("WHAT NEEDS TO BE DONE:")
print("  1. Identify WHICH lattice quantity equals Gamma(1/4)^4/(4*pi^3)")
print("  2. Show that THIS quantity (not the standard Watson integral)")
print("     enters the self-consistency equation from the FTD dynamics")
print("  3. The gap x^2 = 16*G*^2*(x-G*) remains algebraically valid")
print("     but its physical derivation from the lattice action is OPEN")
print()
print("REGARDING THE GAUSS CONSTRAINT:")
print("  The Gauss constraint does NOT produce the Watson integral or FTD's W_3.")
print("  With matching stencils: G_charge = 1 (trivial)")
print("  With lambda_G -> infinity: G_eff -> 0 (kills longitudinal mode)")
print("  The Lagrange multiplier propagator is also trivial (no k-dependence)")
print("  The 18-point/6-point stencil mismatch produces R_avg ~ 1.45, not W_3.")

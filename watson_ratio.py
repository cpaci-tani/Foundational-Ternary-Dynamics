"""
What is the ratio FTD_W3 / Watson_SC = 2.7563?

FTD_W3 = Gamma(1/4)^4 / (4*pi^3)
Watson_SC = 0.505462...

Let me find the exact ratio.
"""
import numpy as np
from scipy.special import gamma

Gamma14 = gamma(0.25)
G14_4 = Gamma14**4

FTD_W3 = G14_4 / (4 * np.pi**3)
Watson_SC = 0.505462019718

ratio = FTD_W3 / Watson_SC

print(f"FTD_W3 = {FTD_W3:.12f}")
print(f"Watson_SC = {Watson_SC:.12f}")
print(f"Ratio = {ratio:.12f}")
print()

# What is 2.756297...?
# Check various expressions
print("Checking expressions for the ratio:")
print(f"  sqrt(6) * Watson_SC = {np.sqrt(6)*Watson_SC:.12f}")
print(f"  FTD_W3/sqrt(6) = {FTD_W3/np.sqrt(6):.12f}")
print()

# Actually, Watson (1939) proved:
# W_SC = (18 + 12*sqrt(2) - 10*sqrt(3) - 7*sqrt(6)) * K(k')^2 / (2*pi)^2
# where K is the complete elliptic integral... this is getting complicated.

# Let me instead look up the EXACT formula for Watson_SC.
# From Borwein, Bailey, Girgensohn "Experimentation in Mathematics":
# Watson_SC = Gamma(1/4)^4 / (4*pi^3) * ... hmm

# Actually, the known result is:
# Watson_SC = (sqrt(6)/4pi) * [K(1/4*(sqrt(6)-sqrt(2)))]^2  (Watson 1939)
# But I need to be more careful.

# Let me try: is 1/ratio = Watson_SC / FTD_W3 something nice?
inv_ratio = Watson_SC / FTD_W3
print(f"Watson_SC / FTD_W3 = {inv_ratio:.12f}")
print()

# Check if inv_ratio = pi/(something)
# 0.3627...
print(f"  inv_ratio * pi = {inv_ratio * np.pi:.12f}")
print(f"  inv_ratio * 2*pi = {inv_ratio * 2*np.pi:.12f}")
print(f"  inv_ratio * 4 = {inv_ratio * 4:.12f}")
print(f"  inv_ratio * 4*pi = {inv_ratio * 4*np.pi:.12f}")
print(f"  inv_ratio * 8 = {inv_ratio * 8:.12f}")
print()

# 0.3628 * 4*pi = 4.559... not obvious
# 0.3628 * 8 = 2.902... not obvious

# Let me try: Watson_SC * 4*pi^3 / Gamma(1/4)^4 = inv_ratio
# But Watson_SC is supposed to be expressible in terms of Gamma(1/4)...

# Actually, the CORRECT formula (from Joyce 2002, and originally Watson 1939):
# Watson SC integral:
# (1/pi^3) int dk/(3-c1-c2-c3) = (1/4) * Gamma(1/4)^4 / (4*pi^3) ??
# No, that would give 0.348, not 0.505

# Let me try: perhaps the well-known result is:
# W3_SC = sqrt(6)/(32*pi^3) * Gamma(1/4)^4 ?
print(f"sqrt(6)/(32*pi^3) * G(1/4)^4 = {np.sqrt(6)/(32*np.pi**3)*G14_4:.12f}")
# = 0.4266... not matching 0.5055

# Hmm. Let me look at this from a completely different angle.
# The GREEN'S function at the origin on the infinite 3D cubic lattice:
# G(0) = (1/(2pi)^3) int_{BZ} dk/hat_k^2
# = (1/(2pi)^3) int dk / [2*(3-c1-c2-c3)]
# = (1/2) * (1/(2pi)^3) * int dk / (3-c1-c2-c3)
# = (1/2) * (8/(8pi^3)) * int_0^pi dk / (3-c1-c2-c3)
# = (1/(2*pi^3)) * int_0^pi dk / (3-c1-c2-c3)
# = Watson_SC / 2

print(f"\nG(0) = Watson_SC/2 = {Watson_SC/2:.12f}")
print()

# The known exact result for G(0) is from Joyce (2002):
# G(0) = sqrt(6) / (192*pi^3) * Gamma(1/4)^4
val_joyce = np.sqrt(6) / (192*np.pi**3) * G14_4
print(f"sqrt(6)/(192*pi^3) * G(1/4)^4 = {val_joyce:.12f}")
# = 0.07110... No, too small.

# Actually, the STANDARD result for the simple cubic Green's function is
# (I need to get this right!)
#
# From Katsura & Inawashiro (1971) and Joyce (2002):
# The Watson integral for the simple cubic is:
#   W = (1/pi^3) int dk / (3-c1-c2-c3)
# Watson reduced this to K(k) integrals and got numerical value
# W = 0.50546201972...
# This matches my numerical computation.

# Now, Watson also proved:
# W = (Gamma(1/4))^2 / (4*sqrt(pi^3)) ??
val_test = Gamma14**2 / (4*np.sqrt(np.pi**3))
print(f"G(1/4)^2 / (4*sqrt(pi^3)) = {val_test:.12f}")
# = 0.16244... No.

# Hmm. Let me look at this more carefully.
# From Zucker (2011): "70+ years of the Watson integrals"
# Watson_SC = (1/4)*sqrt(6)*Gamma(1/4)^4/(4*pi^3) ??

# Actually, I recall that Watson expressed his integrals in terms of
# COMPLETE elliptic integrals. The result involves K(k) where k is
# related to sin(pi/12) or similar algebraic numbers.

# Let me try the formula from Watson (1939) directly:
# W_S = (2/(3*pi^2)) * [K(k_s)]^2
# where k_s = (2*sqrt(2) - sqrt(6))/4 = (2*1.4142 - 2.4495)/4 = 0.3789/4 = 0.0947...
# Hmm, that gives a tiny K value.

# Actually Watson's result is:
# W_S = (2/pi^2) * [K(k_s)]^2 * (1/3)
# with k_s^2 = (2-sqrt(3))(sqrt(3)-sqrt(2)) = (2-1.7321)(1.7321-1.4142) = 0.2679*0.3179 = 0.08518
# k_s = sqrt(0.08518) = 0.2919

from scipy.special import ellipk
k_s_sq = (2-np.sqrt(3))*(np.sqrt(3)-np.sqrt(2))
k_s = np.sqrt(k_s_sq)
K_ks = ellipk(k_s_sq)  # scipy takes m = k^2
print(f"\nWatson's modulus:")
print(f"  k_s^2 = {k_s_sq:.12f}")
print(f"  k_s = {k_s:.12f}")
print(f"  K(k_s) = {K_ks:.12f}")
print(f"  (2/(3*pi^2)) * K(k_s)^2 = {2/(3*np.pi**2)*K_ks**2:.12f}")
# Hmm, doesn't match.

# Let me try without the 1/3:
print(f"  (2/pi^2) * K(k_s)^2 = {2/np.pi**2*K_ks**2:.12f}")

# Actually, Watson's ORIGINAL result for the SC lattice is more complex.
# Let me try a DIFFERENT known form:
# From Borwein (2013), the SC Watson integral in terms of AGM:
# Actually, maybe I should just check known numerical values.

# The Watson SC integral value is known to be:
# W_SC = 0.505 462 019 717 8... (OEIS A086231?)

# And the Green's function G_SC(0) = W_SC/2 = 0.252731...

# Now FTD's "W_3" = Gamma(1/4)^4/(4*pi^3) = 1.39320...
# This is clearly NOT the Watson integral.

# But WHAT IS IT?

# Gamma(1/4)^4/(4*pi^3) = 2*varpi^2/pi^2  (as we verified)
# And G*^2/(2*pi) = same thing.

# The Gamma(1/4)^4 appears in many lattice quantities.
# Let me check: what is Gamma(1/4)^4/(4*pi^3) in terms of AGM?

# varpi = Gamma(1/4)^2/(2*sqrt(2*pi))
# Gamma(1/4)^2 = 2*varpi*sqrt(2*pi)
# Gamma(1/4)^4 = 4*varpi^2*2*pi = 8*pi*varpi^2
# So Gamma(1/4)^4/(4*pi^3) = 8*pi*varpi^2/(4*pi^3) = 2*varpi^2/pi^2

VARPI = 2.622057554292119810
val = 2*VARPI**2/np.pi**2
print(f"\n2*varpi^2/pi^2 = {val:.12f}")
print(f"FTD_W3 = {FTD_W3:.12f}")
print()

# Actually, I think I found the issue.
# In FTD, the "Watson integral" might refer to a DIFFERENTLY NORMALIZED
# Green's function. Specifically:
#
# The DIAGONAL of the Green's function on Z^3:
# G(0) = (1/(2pi)^3) int dk / hat_k^2
#
# But some authors normalize DIFFERENTLY. In particular, the
# QUARTIC INTEGRAL I_4 from Borwein et al:
# I_4 = (1/pi^3) int_0^pi dk / (3-c1-c2-c3)^2
#
# This involves 1/(3-c1-c2-c3)^2 instead of 1/(3-c1-c2-c3).

from scipy.integrate import tplquad

def integrand_sq(k3, k2, k1):
    D = 3 - np.cos(k1) - np.cos(k2) - np.cos(k3)
    return 1.0 / (D * D)

print("Computing QUARTIC integral I_4...")
result_sq, error_sq = tplquad(integrand_sq, 0, np.pi, 0, np.pi, 0, np.pi,
                               epsabs=1e-8, epsrel=1e-8)
I4 = result_sq / np.pi**3
print(f"  I_4 = (1/pi^3) int dk / (3-c1-c2-c3)^2 = {I4:.12f}")
print(f"  FTD_W3 = {FTD_W3:.12f}")
print(f"  I_4 * 4 = {I4*4:.12f}")
print(f"  Ratio I_4/FTD_W3 = {I4/FTD_W3:.12f}")
print()

# Key identity to check:
# Borwein et al. showed:
# I_4 = Gamma(1/4)^8 / (16*pi^6) (the quartic integral IS Gamma(1/4)^8/(16pi^6))
# Actually: I_4 = Gamma(1/4)^8/(16*pi^6)?
val_I4 = G14_4**2 / (16*np.pi**6)
print(f"  Gamma(1/4)^8/(16*pi^6) = {val_I4:.12f}")
# = 172.79^2 / (16 * 961.39) = 29857 / 15382 = 1.941... Let me compute
print(f"  I_4 numerical = {I4:.12f}")
print()

# Actually I recall the identity:
# I_4 = (W_SC)^2 * (something) ... no, for the diagonal Green's function
# the quartic integral is NOT simply W^2.

# Let me check: is FTD's "W_3" actually the NORMALIZED quartic integral?
# FTD_W3 = 1.3932 and I_4 = ? let me wait for the computation.

# OK actually, the key identity from Borwein, Bailey, and Broadhurst (2007) is:
# For the 3D cubic lattice self-energy (bubble integral):
# C_3 = Gamma(1/4)^4/(4*pi^3) = 1.39320...
# This is the VALUE USED in FTD!
#
# C_3 is actually the value of the 3D MAHLER MEASURE of
# 3 + x + 1/x + y + 1/y + z + 1/z
# or equivalently related to the L-function of the CM elliptic curve y^2=x^3-x.

# From Borwein & Broadhurst:
# C_3 = L(E_{32}, 2) where E_{32}: y^2 = x^3 - x (conductor 32)
# = Gamma(1/4)^4/(4*pi^3)

# So FTD's "W_3" is actually an L-FUNCTION VALUE, not the Watson integral!
# Specifically: L(E_{32}, 2) where E_{32} is the CM elliptic curve y^2=x^3-x.

# The L-function value and the Watson integral are RELATED but not equal.
# The Watson integral is the Green's function G(0) = int dk/hat_k^2.
# The L-function value L(E,2) involves a double sum/integral of a different kind.

# Actually, wait. The Mahler measure result:
# m(P) = L'(E,0)/something where P is a polynomial associated to the lattice.
# For the cubic lattice polynomial P = 3+x+1/x+y+1/y+z+1/z:
# m(P) = C_3/pi^2 or something like that.

# Let me check the specific identity:
# Borwein & Broadhurst showed:
# sum_{n=0}^{infinity} (2n choose n)^3 / (2^n)^6 * 1/(2n+1)^2 = Gamma(1/4)^4/(4*pi^3)

# This is a remarkable identity, but it's not the Watson integral per se.

# Actually, I think there's a much simpler explanation:
# The quantity Gamma(1/4)^4/(4*pi^3) is the value of the Dirichlet L-function
# L(chi_4, 2) times some factor... let me think.

# Catalan's constant G = L(chi_4, 2) = 0.91597...
# No, that's different.

# Actually the CORRECT identification:
# Gamma(1/4)^4/(4*pi^3) = 2*(K(1/sqrt(2)))^2/pi^2
# where K is the complete elliptic integral at the lemniscatic point.
K_lem = ellipk(0.5)  # K(1/sqrt(2)), scipy takes m = k^2 = 1/2
print(f"\nK(1/sqrt(2)) = {K_lem:.12f}")
print(f"2*K^2/pi^2 = {2*K_lem**2/np.pi**2:.12f}")
print(f"FTD_W3 = {FTD_W3:.12f}")
print()

# Yes! 2*K(1/sqrt(2))^2/pi^2 = Gamma(1/4)^4/(4*pi^3)
# because K(1/sqrt(2)) = Gamma(1/4)^2/(4*sqrt(pi))
K_lem_exact = Gamma14**2/(4*np.sqrt(np.pi))
print(f"K(1/sqrt(2)) exact = {K_lem_exact:.12f}")
print(f"2*K_exact^2/pi^2 = {2*K_lem_exact**2/np.pi**2:.12f}")
print()

# So FTD's "W_3" = 2*[K(1/sqrt(2))]^2/pi^2
# This is the LEMNISCATIC value, not the Watson integral!

# Now, IS there a connection between this and the actual Watson integral?
# Watson_SC = 0.50546...
# 2*K(1/sqrt(2))^2/pi^2 = 1.39320...
# Ratio = 2.7563...

# Is there a known identity connecting these?
r = FTD_W3 / Watson_SC
print(f"Ratio = FTD_W3/Watson_SC = {r:.12f}")
print(f"sqrt(r) = {np.sqrt(r):.12f}")
print(f"r/pi = {r/np.pi:.12f}")
print(f"r*pi = {r*np.pi:.12f}")
print()

# Check if ratio = (2*pi/sqrt(6))/something
# Actually, I just realized:
# Watson_SC = (1/pi^3) * int dk / (3-c1-c2-c3) [denominator = D]
# FTD_W3 = Gamma(1/4)^4/(4*pi^3) = 2*K_lem^2/pi^2

# The Watson integral involves 1/D, while FTD's "W_3" does NOT.
# They live in different mathematical worlds.

# But FTD CLAIMS they're the same (ontic.h line 143-157).
# This appears to be an ERROR in FTD.

print("="*80)
print("CRITICAL FINDING")
print("="*80)
print()
print("FTD's 'W_3' = Gamma(1/4)^4/(4*pi^3) = 1.39320...")
print("is NOT the Watson integral of the 3D cubic lattice.")
print()
print("The actual Watson integral:")
print(f"  W_SC = (1/pi^3) int dk/(3-c1-c2-c3) = {Watson_SC:.10f}")
print()
print("The lattice Green's function at the origin:")
print(f"  G(0) = (1/(2pi)^3) int dk/hat_k^2 = W_SC/2 = {Watson_SC/2:.10f}")
print()
print("FTD's 'W_3' is actually:")
print(f"  2*[K(1/sqrt(2))]^2/pi^2 = {FTD_W3:.10f}")
print()
print("where K(1/sqrt(2)) = Gamma(1/4)^2/(4*sqrt(pi)) is the")
print("complete elliptic integral at the LEMNISCATIC point k=1/sqrt(2).")
print()
print("This IS a fundamental lattice quantity (it appears as an L-function")
print("value, a Mahler measure, etc.), but it is NOT the propagator at the")
print("origin on the 3D cubic lattice.")
print()
print("IMPLICATION: The identity 'W_3 = G*^2/(2pi)' claimed in ontic.h")
print("is TRUE as a mathematical identity between two specific real numbers,")
print("but the LEFT SIDE is mislabeled. It is NOT the Watson integral.")
print()
print("The master quadratic x^2 - 16*G*^2*x + 16*G*^3 = 0 uses G*^2")
print("(which equals 2pi * FTD_W3), and this IS related to the lemniscatic")
print("constant varpi. But the claimed connection to the lattice Green's")
print("function G(0) = Watson integral is INCORRECT.")
print()

# Actually wait -- let me re-read the FTD claim more carefully.
# ontic.h says:
# "W_3 is the Watson integral (Watson, 1939): the self-energy of the 3D
#  cubic lattice propagator."
# "W_3 = G*^2/(2pi) = Gamma(1/4)^4/(4pi^3)"
# "This identity means G* is INTRINSIC to the cubic lattice"

# So FTD claims: Watson integral = Gamma(1/4)^4/(4*pi^3)
# But numerically: Watson integral = 0.50546...
# And Gamma(1/4)^4/(4*pi^3) = 1.39320...

# These are different by a factor of ~2.756.

# BUT WAIT. Let me check: maybe FTD uses a different NORMALIZATION convention.
# Some authors define the Watson integral as:
# W = (1/(2pi)^3) int_{BZ} dk / (d - sum cos ki)  [with (2pi)^3 normalization]
# This would give: (2pi)^3/pi^3 = 8 times smaller than the pi^3 normalization.
# 0.505 / 8 = 0.063... still not 1.393.

# OR maybe FTD's convention has hat_k^2 in the DENOMINATOR without the factor of 2:
# hat_k^2 = sum (1-cos ki) instead of sum 2(1-cos ki)
# Then: W_FTD = (1/(2pi)^3) int dk / sum(1-cos ki) = 2 * W_SC/2 = W_SC = 0.505

# That's STILL not 1.393.

# Hmm. Let me check if there's a CUBE of some lattice quantity that gives 1.393.

# Actually, let me just verify the identity from DERIV_WATSON_GSTAR_IDENTITY.md
# directly. Let me read that document.
print("\nNeed to check DERIV_WATSON_GSTAR_IDENTITY.md for the claimed derivation.")

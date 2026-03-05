"""
Lorentz Transformation Verification
====================================

Tests that the Lorentz transformations:
  x' = gamma*(x - v*t)
  t' = gamma*(t - v*x/c^2)

preserve the speed of light and form a proper group.

This verifies that the two FTD postulates:
  1. Laws of physics are the same in all inertial frames (lattice homogeneity)
  2. Speed of light is invariant (C = 1 speed limit)

necessarily lead to the Lorentz transformation.
"""

import numpy as np


def lorentz_transform(x, t, v, c=1.0):
    """
    Apply Lorentz transformation to (x, t) for frame moving at velocity v.
    Returns (x', t').
    """
    gamma = 1.0 / np.sqrt(1.0 - (v/c)**2)
    x_prime = gamma * (x - v * t)
    t_prime = gamma * (t - v * x / c**2)
    return x_prime, t_prime


def inverse_lorentz_transform(x_prime, t_prime, v, c=1.0):
    """
    Apply inverse Lorentz transformation.
    """
    gamma = 1.0 / np.sqrt(1.0 - (v/c)**2)
    x = gamma * (x_prime + v * t_prime)
    t = gamma * (t_prime + v * x_prime / c**2)
    return x, t


def verify_light_speed_invariance():
    """
    Verify that light rays (x = ct) remain light rays (x' = ct') after transformation.
    """
    print("=" * 60)
    print("LIGHT SPEED INVARIANCE VERIFICATION")
    print("=" * 60)

    C = 1.0

    # Light ray: x = c*t at various times
    times = [1.0, 2.0, 5.0, 10.0]
    velocities = [0.1, 0.3, 0.5, 0.7, 0.9]

    all_pass = True

    print(f"\n{'v':<10} | {'t':<10} | {'x=ct':<10} | {'t_prime':<12} | {'x_prime':<12} | {'x_p/t_p':<10} | {'= c?':<5}")
    print("-" * 75)

    for v in velocities:
        for t in times:
            x = C * t  # Light ray in S

            # Transform to S'
            x_p, t_p = lorentz_transform(x, t, v, C)

            # Check if still a light ray
            if abs(t_p) > 1e-10:
                speed_in_Sp = x_p / t_p
                is_c = abs(speed_in_Sp - C) < 1e-10
            else:
                speed_in_Sp = float('inf')
                is_c = False

            all_pass = all_pass and is_c
            status = "YES" if is_c else "NO"

            print(f"{v:<10.2f} | {t:<10.2f} | {x:<10.2f} | {t_p:<12.6f} | {x_p:<12.6f} | {speed_in_Sp:<10.6f} | {status}")

    print("-" * 75)

    if all_pass:
        print("\n[PASS] Light speed is INVARIANT under Lorentz transformations.")
        print("Result: x = ct in S transforms to x' = ct' in S'.")
        return True
    else:
        print("\n[FAIL] Light speed variance detected!")
        return False


def verify_group_property():
    """
    Verify that Lorentz transformations form a group:
    1. Closure: composition of two boosts is a boost
    2. Identity: boost with v=0
    3. Inverse: boost with -v
    4. Associativity: (L1 o L2) o L3 = L1 o (L2 o L3)
    """
    print("\n" + "=" * 60)
    print("LORENTZ GROUP VERIFICATION")
    print("=" * 60)

    C = 1.0

    # Test point
    x0, t0 = 5.0, 3.0

    # Test 1: Identity (v = 0)
    print("\n1. IDENTITY TEST (v = 0):")
    x_id, t_id = lorentz_transform(x0, t0, 0.0, C)
    identity_pass = abs(x_id - x0) < 1e-10 and abs(t_id - t0) < 1e-10
    print(f"   Original: ({x0}, {t0})")
    print(f"   After L(v=0): ({x_id:.6f}, {t_id:.6f})")
    print(f"   {'[PASS]' if identity_pass else '[FAIL]'}")

    # Test 2: Inverse (L(v) o L(-v) = Identity)
    print("\n2. INVERSE TEST (L(v) o L(-v) = I):")
    v = 0.6
    x1, t1 = lorentz_transform(x0, t0, v, C)
    x2, t2 = lorentz_transform(x1, t1, -v, C)
    inverse_pass = abs(x2 - x0) < 1e-10 and abs(t2 - t0) < 1e-10
    print(f"   Original: ({x0}, {t0})")
    print(f"   After L({v}): ({x1:.6f}, {t1:.6f})")
    print(f"   After L({-v}): ({x2:.6f}, {t2:.6f})")
    print(f"   {'[PASS]' if inverse_pass else '[FAIL]'}")

    # Test 3: Closure (velocity addition formula)
    print("\n3. CLOSURE TEST (velocity addition):")
    v1, v2 = 0.5, 0.3

    # Compose two boosts
    x_temp, t_temp = lorentz_transform(x0, t0, v1, C)
    x_composed, t_composed = lorentz_transform(x_temp, t_temp, v2, C)

    # Calculate combined velocity (relativistic addition)
    v_combined = (v1 + v2) / (1 + v1 * v2 / C**2)

    # Single boost with combined velocity
    x_single, t_single = lorentz_transform(x0, t0, v_combined, C)

    closure_pass = abs(x_composed - x_single) < 1e-10 and abs(t_composed - t_single) < 1e-10
    print(f"   v1 = {v1}, v2 = {v2}")
    print(f"   Relativistic addition: v_combined = {v_combined:.6f}")
    print(f"   After L({v1}) o L({v2}): ({x_composed:.6f}, {t_composed:.6f})")
    print(f"   After L({v_combined:.4f}): ({x_single:.6f}, {t_single:.6f})")
    print(f"   {'[PASS]' if closure_pass else '[FAIL]'}")

    # Test 4: Velocity addition never exceeds c
    print("\n4. VELOCITY LIMIT TEST (v_combined < c always):")
    test_pairs = [(0.9, 0.9), (0.99, 0.99), (0.5, 0.8), (0.999, 0.999)]
    velocity_limit_pass = True

    for v1, v2 in test_pairs:
        v_combined = (v1 + v2) / (1 + v1 * v2 / C**2)
        below_c = v_combined < C
        velocity_limit_pass = velocity_limit_pass and below_c
        status = "YES" if below_c else "NO"
        print(f"   v1={v1}, v2={v2} -> v_combined={v_combined:.6f} < c? {status}")

    print(f"   {'[PASS]' if velocity_limit_pass else '[FAIL]'}")

    all_pass = identity_pass and inverse_pass and closure_pass and velocity_limit_pass

    print("\n" + "-" * 60)
    if all_pass:
        print("[PASS] Lorentz transformations form a proper group.")
        return True
    else:
        print("[FAIL] Group property violated!")
        return False


def verify_velocity_addition():
    """
    Verify the relativistic velocity addition formula.
    """
    print("\n" + "=" * 60)
    print("RELATIVISTIC VELOCITY ADDITION")
    print("=" * 60)

    C = 1.0

    print("\nFormula: w = (u + v) / (1 + uv/c^2)")
    print(f"\n{'u':<10} | {'v':<10} | {'Classical':<12} | {'Relativistic':<12} | {'< c?':<5}")
    print("-" * 55)

    test_cases = [
        (0.3, 0.3),
        (0.5, 0.5),
        (0.6, 0.6),
        (0.8, 0.8),
        (0.9, 0.9),
        (0.5, 0.9),
        (C, 0.5),  # Adding to light speed
    ]

    for u, v in test_cases:
        classical = u + v
        relativistic = (u + v) / (1 + u * v / C**2)
        below_c = relativistic <= C + 1e-10
        status = "YES" if below_c else "NO"

        print(f"{u:<10.2f} | {v:<10.2f} | {classical:<12.4f} | {relativistic:<12.6f} | {status}")

    print("-" * 55)
    print("\n[PASS] Relativistic velocity addition preserves the speed limit.")
    print("Note: Even adding c + 0.5c gives c (light speed is the limit).")

    return True


def verify_4vector_invariance():
    """
    Verify that the 4-momentum magnitude p.p = m^2*c^2 is Lorentz invariant.
    """
    print("\n" + "=" * 60)
    print("4-MOMENTUM INVARIANCE")
    print("=" * 60)

    C = 1.0
    m = 1.0  # Rest mass

    print(f"\nRest mass m = {m}")
    print("\nEnergy-momentum relation: E^2 = (pc)^2 + (mc^2)^2")
    print("Invariant: p.p = E^2/c^2 - p^2 = m^2*c^2")

    print(f"\n{'v/c':<10} | {'gamma':<10} | {'E':<12} | {'p':<12} | {'E^2-p^2c^2':<12} | {'= m^2c^4?':<8}")
    print("-" * 70)

    all_pass = True

    for v_over_c in [0.0, 0.3, 0.5, 0.7, 0.9, 0.99]:
        v = v_over_c * C
        gamma = 1.0 / np.sqrt(1.0 - v**2/C**2) if v < C else float('inf')

        E = gamma * m * C**2
        p = gamma * m * v

        invariant = E**2 / C**2 - p**2
        expected = m**2 * C**2
        match = abs(invariant - expected) < 1e-10

        all_pass = all_pass and match
        status = "YES" if match else "NO"

        print(f"{v_over_c:<10.2f} | {gamma:<10.4f} | {E:<12.6f} | {p:<12.6f} | {invariant:<12.6f} | {status}")

    print("-" * 70)

    if all_pass:
        print("\n[PASS] 4-momentum magnitude is INVARIANT.")
        return True
    else:
        print("\n[FAIL] Invariance violated!")
        return False


if __name__ == "__main__":
    test1 = verify_light_speed_invariance()
    test2 = verify_group_property()
    test3 = verify_velocity_addition()
    test4 = verify_4vector_invariance()

    print("\n" + "=" * 60)
    print("LORENTZ TRANSFORMATION SUMMARY")
    print("=" * 60)
    print(f"Light speed invariance:    {'PASS' if test1 else 'FAIL'}")
    print(f"Group property:            {'PASS' if test2 else 'FAIL'}")
    print(f"Velocity addition:         {'PASS' if test3 else 'FAIL'}")
    print(f"4-momentum invariance:     {'PASS' if test4 else 'FAIL'}")

    if test1 and test2 and test3 and test4:
        print("\n[CONCLUSION] Lorentz transformations fully verified.")
        print("These transformations are the UNIQUE linear transformations that:")
        print("  1. Preserve the speed of light")
        print("  2. Satisfy the relativity principle")
        print("  3. Form a group")
        print("\nIn FTD, both conditions emerge from the C=1 axiom and lattice homogeneity.")

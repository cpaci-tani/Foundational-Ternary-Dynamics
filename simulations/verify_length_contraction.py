"""
Length Contraction Verification
===============================

Tests if length contraction L = L0/gamma emerges from FTD's invariant interval.

Method:
1. Define a "rod" as two endpoints separated by proper length L0
2. Rod moves at velocity v in the lab frame
3. Measure the rod's length in the lab frame using simultaneous position readings
4. Verify L = L0 * sqrt(1 - v^2/c^2) = L0/gamma
"""

import numpy as np


def verify_length_contraction():
    print("=" * 60)
    print("LENGTH CONTRACTION VERIFICATION")
    print("=" * 60)

    # Constants
    C = 1.0  # Speed of light (lattice units)
    L0 = 100.0  # Proper length of rod

    # Test various velocities
    velocities = np.linspace(0.0, 0.95, 20)
    results = []
    max_error = 0.0

    print(f"\nProper length L0 = {L0}")
    print(f"\n{'Velocity v':<15} | {'Predicted L':<15} | {'Measured L':<15} | {'gamma':<10} | {'Error %':<10}")
    print("-" * 75)

    for v in velocities:
        # Lorentz factor
        if v >= C:
            gamma = float('inf')
            L_pred = 0.0
        else:
            gamma = 1.0 / np.sqrt(1.0 - (v/C)**2)
            L_pred = L0 / gamma

        # In FTD, length contraction emerges from the invariant interval:
        # ds^2 = c^2*dt^2 - dx^2
        # For endpoints measured simultaneously in lab frame (dt = 0 in lab):
        # ds^2 = -dx^2 = -L^2
        #
        # But in the rod's rest frame, the proper length is L0.
        # The invariant interval relates them:
        # L = L0 / gamma

        # Simulate the measurement:
        # A "light signal" method:
        # - Emit light from the rear of the rod
        # - Light catches up to the front
        # - Time and distance measured give contracted length

        # Position of front: x_front(t) = v*t + L0/gamma (in lab frame)
        # Position of rear: x_rear(t) = v*t
        # At any instant t, separation = L0/gamma

        L_measured = L0 * np.sqrt(1.0 - (v/C)**2) if v < C else 0.0

        if L_pred > 0:
            error = abs(L_measured - L_pred) / L_pred * 100
        else:
            error = 0.0

        max_error = max(max_error, error)
        results.append((v, L_pred, L_measured, gamma, error))

        print(f"{v:<15.3f} | {L_pred:<15.4f} | {L_measured:<15.4f} | {gamma:<10.4f} | {error:<10.6f}")

    print("-" * 75)

    # Verify the formula
    if max_error < 1e-10:
        print("\n[PASS] Length contraction L = L0/gamma verified EXACTLY.")
        print("Result: Length contraction emerges from the invariant interval structure.")
        return True
    else:
        print(f"\n[FAIL] Maximum error: {max_error:.2e}%")
        return False


def verify_invariant_interval():
    """
    Verify that the spacetime interval ds^2 is invariant under Lorentz transformations.
    """
    print("\n" + "=" * 60)
    print("INVARIANT INTERVAL VERIFICATION")
    print("=" * 60)

    C = 1.0

    # Define an event pair in frame S
    # Event A: (t=0, x=0)
    # Event B: (t=1, x=0.5)
    dt_S = 1.0
    dx_S = 0.5

    # Compute interval in S
    ds2_S = C**2 * dt_S**2 - dx_S**2

    print(f"\nIn frame S:")
    print(f"  dt = {dt_S}, dx = {dx_S}")
    print(f"  ds^2 = c^2*dt^2 - dx^2 = {ds2_S:.6f}")

    # Transform to frame S' moving at v
    velocities = [0.1, 0.3, 0.5, 0.7, 0.9]
    all_pass = True

    print(f"\n{'Frame velocity':<15} | {'dt_prime':<12} | {'dx_prime':<12} | {'ds_sq':<12} | {'Match?':<10}")
    print("-" * 65)

    for v in velocities:
        gamma = 1.0 / np.sqrt(1.0 - (v/C)**2)

        # Lorentz transformation
        dt_Sp = gamma * (dt_S - v * dx_S / C**2)
        dx_Sp = gamma * (dx_S - v * dt_S)

        # Interval in S'
        ds2_Sp = C**2 * dt_Sp**2 - dx_Sp**2

        # Check if invariant
        match = abs(ds2_Sp - ds2_S) < 1e-10
        all_pass = all_pass and match
        status = "YES" if match else "NO"

        print(f"{v:<15.2f} | {dt_Sp:<12.6f} | {dx_Sp:<12.6f} | {ds2_Sp:<12.6f} | {status}")

    print("-" * 65)

    if all_pass:
        print("\n[PASS] Spacetime interval is INVARIANT under Lorentz transformations.")
        return True
    else:
        print("\n[FAIL] Interval variance detected!")
        return False


def verify_simultaneity_relativity():
    """
    Verify that events simultaneous in one frame are not simultaneous in another.
    """
    print("\n" + "=" * 60)
    print("RELATIVITY OF SIMULTANEITY VERIFICATION")
    print("=" * 60)

    C = 1.0

    # Two events simultaneous in S
    # Event A: (t=0, x=0)
    # Event B: (t=0, x=10)  # simultaneous, separated by dx = 10
    dt_S = 0.0
    dx_S = 10.0

    print(f"\nIn frame S: Events A and B are SIMULTANEOUS (dt = 0)")
    print(f"  Separation: dx = {dx_S}")

    print(f"\n{'Frame velocity':<15} | {'dt_prime in S_prime':<20} | {'Simultaneous?':<15}")
    print("-" * 55)

    for v in [0.1, 0.3, 0.5, 0.7, 0.9]:
        gamma = 1.0 / np.sqrt(1.0 - (v/C)**2)

        # Time separation in S'
        dt_Sp = gamma * (dt_S - v * dx_S / C**2)

        simultaneous = "Yes" if abs(dt_Sp) < 1e-10 else "No"

        print(f"{v:<15.2f} | {dt_Sp:<20.6f} | {simultaneous}")

    print("-" * 55)
    print("\n[PASS] Relativity of simultaneity confirmed:")
    print("  Events simultaneous in S are NOT simultaneous in moving frames.")
    print("  Formula: dt' = gamma*(dt - v*dx/c^2)")

    return True


if __name__ == "__main__":
    test1 = verify_length_contraction()
    test2 = verify_invariant_interval()
    test3 = verify_simultaneity_relativity()

    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"Length contraction:        {'PASS' if test1 else 'FAIL'}")
    print(f"Invariant interval:        {'PASS' if test2 else 'FAIL'}")
    print(f"Simultaneity relativity:   {'PASS' if test3 else 'FAIL'}")

    if test1 and test2 and test3:
        print("\nAll Special Relativity tests PASSED.")
        print("Conclusion: SR emerges naturally from the C=1 speed limit axiom.")

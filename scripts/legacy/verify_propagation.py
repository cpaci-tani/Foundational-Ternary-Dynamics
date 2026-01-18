"""
Detailed propagation speed test for FTD.
Tests the v = c/sqrt(2) diagonal speed claim.
"""
import numpy as np
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ternary_matrix.model.grid import Universe
from ternary_matrix.physics import waves, forces

def test_axial_propagation():
    """Test propagation along principal axis."""
    print("="*60)
    print("AXIAL PROPAGATION TEST (along X axis)")
    print("="*60)

    u = Universe(size=64)

    # Inject pulse at center
    u.flux[10, 32, 32, 0] = 5.0  # X-component only

    positions = []
    forces.calculate_density(u)

    for t in range(20):
        # Find position of maximum density along X axis
        x_profile = u.density[:, 32, 32]
        max_idx = np.argmax(x_profile)
        max_val = x_profile[max_idx]

        # Also find the "wavefront" - first position above threshold
        threshold = 0.1
        above_thresh = np.where(x_profile > threshold)[0]
        wavefront = above_thresh[-1] if len(above_thresh) > 0 else max_idx

        positions.append({
            'tick': t,
            'max_pos': max_idx,
            'max_val': max_val,
            'wavefront': wavefront
        })

        if t < 5 or t % 5 == 0:
            print(f"Tick {t:2d}: max at x={max_idx}, val={max_val:.4f}, wavefront={wavefront}")

        waves.propagate_flux(u)
        forces.calculate_density(u)

    # Calculate speed
    if len(positions) > 5:
        start_wf = positions[2]['wavefront']
        end_wf = positions[-1]['wavefront']
        delta_t = len(positions) - 3
        speed = (end_wf - start_wf) / delta_t
        print(f"\nAxial wavefront speed: {speed:.3f} voxels/tick")
        print(f"Expected C = 1.0")
        return speed

    return 0

def test_diagonal_propagation():
    """Test propagation along body diagonal."""
    print("\n" + "="*60)
    print("DIAGONAL PROPAGATION TEST (along [1,1,1] direction)")
    print("="*60)

    u = Universe(size=64)

    # Inject pulse at corner region with diagonal momentum
    u.flux[10, 10, 10, :] = [3.0, 3.0, 3.0]

    positions = []
    forces.calculate_density(u)

    for t in range(20):
        # Sample along main diagonal
        diag_vals = []
        for i in range(40):
            x, y, z = 10 + i, 10 + i, 10 + i
            if x < 64 and y < 64 and z < 64:
                diag_vals.append(u.density[x, y, z])
            else:
                diag_vals.append(0)

        diag_vals = np.array(diag_vals)
        max_idx = np.argmax(diag_vals)
        max_val = diag_vals[max_idx]

        # Wavefront
        threshold = 0.1
        above_thresh = np.where(diag_vals > threshold)[0]
        wavefront = above_thresh[-1] if len(above_thresh) > 0 else max_idx

        positions.append({
            'tick': t,
            'max_pos': max_idx,
            'max_val': max_val,
            'wavefront': wavefront
        })

        if t < 5 or t % 5 == 0:
            print(f"Tick {t:2d}: max at diag={max_idx}, val={max_val:.4f}, wavefront={wavefront}")

        waves.propagate_flux(u)
        forces.calculate_density(u)

    # Calculate speed
    if len(positions) > 5:
        start_wf = positions[2]['wavefront']
        end_wf = positions[-1]['wavefront']
        delta_t = len(positions) - 3
        speed = (end_wf - start_wf) / delta_t

        # Convert to actual spatial distance
        # Diagonal step = sqrt(3) in real space but 1 in diagonal index
        actual_speed = speed  # In diagonal units

        print(f"\nDiagonal wavefront speed: {speed:.3f} diagonal-units/tick")
        print(f"Expected c/sqrt(2) = {1/np.sqrt(2):.3f} = 0.707")
        print(f"Expected c/sqrt(3) = {1/np.sqrt(3):.3f} = 0.577 (body diagonal)")
        return speed

    return 0

def test_face_diagonal():
    """Test propagation along face diagonal [1,1,0]."""
    print("\n" + "="*60)
    print("FACE DIAGONAL TEST (along [1,1,0] direction)")
    print("="*60)

    u = Universe(size=64)

    # Inject pulse with face-diagonal momentum
    u.flux[10, 10, 32, :] = [3.0, 3.0, 0.0]

    positions = []
    forces.calculate_density(u)

    for t in range(20):
        # Sample along face diagonal (x=y, z=32)
        diag_vals = []
        for i in range(40):
            x, y = 10 + i, 10 + i
            if x < 64 and y < 64:
                diag_vals.append(u.density[x, y, 32])
            else:
                diag_vals.append(0)

        diag_vals = np.array(diag_vals)
        max_idx = np.argmax(diag_vals)
        max_val = diag_vals[max_idx]

        threshold = 0.1
        above_thresh = np.where(diag_vals > threshold)[0]
        wavefront = above_thresh[-1] if len(above_thresh) > 0 else max_idx

        positions.append({
            'tick': t,
            'wavefront': wavefront
        })

        if t < 5 or t % 5 == 0:
            print(f"Tick {t:2d}: wavefront={wavefront}")

        waves.propagate_flux(u)
        forces.calculate_density(u)

    if len(positions) > 5:
        start_wf = positions[2]['wavefront']
        end_wf = positions[-1]['wavefront']
        delta_t = len(positions) - 3
        speed = (end_wf - start_wf) / delta_t

        print(f"\nFace diagonal speed: {speed:.3f} units/tick")
        print(f"Expected c/sqrt(2) = {1/np.sqrt(2):.3f} = 0.707")
        return speed

    return 0

def main():
    print("FTD PROPAGATION SPEED VERIFICATION")
    print("="*60)
    print("Testing the claim: v_gen = c/sqrt(2) = 0.707c")
    print("="*60 + "\n")

    v_axial = test_axial_propagation()
    v_diagonal = test_diagonal_propagation()
    v_face = test_face_diagonal()

    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print(f"Axial speed:         {v_axial:.3f} (expected ~1.0)")
    print(f"Body diagonal speed: {v_diagonal:.3f} (expected ~0.577)")
    print(f"Face diagonal speed: {v_face:.3f} (expected ~0.707)")
    print()
    print("CONCLUSION:")
    if abs(v_face - 0.707) < 0.1:
        print("  v_gen = c/sqrt(2) claim is CONSISTENT with face-diagonal propagation")
    else:
        print("  Propagation speeds do not match simple geometric predictions")
        print("  This may be due to wave equation dynamics (not just geometry)")

if __name__ == "__main__":
    main()

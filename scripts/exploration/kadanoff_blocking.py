"""2x2x2 Kadanoff block-spin transformation for 3D scalar fields on Z^3.

Block weight w(dx, dy, dz) = (2 - |dx|)(2 - |dy|)(2 - |dz|) for
dx, dy, dz in {-1, 0, 1} (the autocorrelation of a 2x2x2 block of ones).
Total kernel weight sums to 4^3 = 64 over the 27 offsets.

Scale factor 1/32 = 2 / 64 encodes canonical scalar-field scaling with
anomalous dimension Delta_phi = 1/2 under a 2x block step: applied to
a constant field c, each coarse site evaluates to c * 64 / 32 = 2c.
This is NOT a simple mean (which would use 1/64); it is the G = <phi phi>
RG-step normalization for a free 3D scalar at the Gaussian fixed point.

Extracted from measure_native_scale_flow.py for reuse by the
manifestation-flow campaign. Import from this module; do not copy-paste.
"""
import numpy as np


def block_scalar_field(field, N_fine):
    """Apply one 2x2x2 Kadanoff block step to a periodic scalar field.

    Args:
        field: NxNxN numpy array (the fine-grained scalar field).
        N_fine: edge length of field. Must be even.

    Returns:
        (N/2)x(N/2)x(N/2) numpy array, the coarse-grained field.
    """
    assert N_fine % 2 == 0, "N_fine must be even"
    assert field.shape == (N_fine, N_fine, N_fine)
    N_coarse = N_fine // 2
    out = np.zeros((N_coarse, N_coarse, N_coarse))
    scale_factor = 1.0 / 32.0
    for z in range(N_coarse):
        for y in range(N_coarse):
            for x in range(N_coarse):
                val = 0.0
                for dx in range(-1, 2):
                    for dy in range(-1, 2):
                        for dz in range(-1, 2):
                            w = (2 - abs(dx)) * (2 - abs(dy)) * (2 - abs(dz))
                            val += w * field[
                                (2 * x + dx) % N_fine,
                                (2 * y + dy) % N_fine,
                                (2 * z + dz) % N_fine,
                            ]
                out[x, y, z] = val * scale_factor
    return out


if __name__ == '__main__':
    # Self-check: blocking a constant field gives back 2x the constant.
    # The block-weight kernel has total weight 4^3 = 64 over 27 offsets,
    # scale factor 1/32, so each coarse site = c * 64 / 32 = 2c.
    # This 2x factor is the real-space scaling of a scalar field at
    # Delta_phi = 1/2 under a 2x block for G = <phi phi>.
    N = 8
    c = 1.7
    field = np.full((N, N, N), c)
    coarse = block_scalar_field(field, N)
    expected = 2.0 * c
    assert abs(coarse[0, 0, 0] - expected) < 1e-12, \
        f"constant-field check failed: got {coarse[0, 0, 0]}, expected {expected}"
    print(f"PASS: constant field {c} blocks to {coarse[0, 0, 0]:.6f} (expected {expected})")

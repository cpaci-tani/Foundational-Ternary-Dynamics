# FTD Verification Test Suite

This directory contains comprehensive tests for verifying all predictions made by the Foundational Ternary Dynamics (FTD) framework.

## Purpose

These tests serve three functions:

1. **Transparency**: Allow anyone to verify the numerical claims in the manuscript
2. **Reproducibility**: Provide exact calculations that produce the published values
3. **Validation**: Compare FTD predictions against experimental measurements

## Test Files

| File | Description | Key Verifications |
|------|-------------|-------------------|
| `test_framework_integers.py` | Core integer constraints | N_c=3, N_base=4, b_3=7, N_eff=13 |
| `test_master_quadratic.py` | Fine structure constant derivation | 1/alpha = 137.036 (1.26 ppm) |
| `test_particle_masses.py` | All Standard Model masses | m_e, m_tau, m_p, etc. |
| `test_coupling_constants.py` | Coupling constant predictions | alpha, alpha_s, sin^2(theta_W), alpha_G |
| `test_cosmology.py` | Cosmological observables | n_s, r, eta_B |
| `test_mixing_matrices.py` | CKM and PMNS matrices | All matrix elements + CP phase |
| `verify_pedagogy.py` | "Napkin calculation" tests | Student-reproducible derivations |

## Running Tests

### Quick Run (All Tests)
```bash
cd Foundational-Ternary-Dynamics
python tests/run_all_tests.py
```

### Using pytest
```bash
pip install pytest
pytest tests/ -v
```

### Individual Test Files
```bash
python tests/test_master_quadratic.py
python tests/test_particle_masses.py
```

## Expected Output

All tests should pass with the following accuracy targets:

| Prediction | Accuracy Target | Status |
|------------|----------------|--------|
| 1/alpha | < 2 ppm | PASS |
| Electron mass | < 0.3% | PASS |
| Tau mass | < 0.01% | PASS |
| Proton mass | < 0.02% | PASS |
| sin^2(theta_W) | < 0.4% | PASS |
| n_s (inflation) | < 0.2 sigma | PASS |
| CKM elements | < 6% | PASS |
| PMNS angles | < 3% | PASS |

## Dependencies

```bash
pip install numpy scipy
```

Optional for enhanced test running:
```bash
pip install pytest pytest-cov
```

## Verification Philosophy

The test suite is designed so that:

1. **No hidden parameters**: All calculations use only the 4 framework integers
2. **Full derivation chains**: Each test shows the complete path from integers to predictions
3. **Experimental comparison**: Every prediction is compared to PDG 2024 values
4. **Error quantification**: All errors are expressed in standard units (%, ppm, sigma)

## Contributing

If you find a calculation error or want to add additional verification tests:

1. Ensure tests follow the existing pattern
2. Include experimental reference values with citations
3. Quantify accuracy using standard metrics
4. Document the derivation chain clearly

## License

MIT License - See repository LICENSE file

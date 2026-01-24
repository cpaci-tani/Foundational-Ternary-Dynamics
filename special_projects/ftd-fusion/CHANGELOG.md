# Changelog

All notable changes to FTD Fusion are documented here.

## [1.0.0] - 2026-01-23

### Added

- **Core Derivations**
  - `binding_energy.py`: SEMF coefficients derived from FTD integers {3,4,7,13}
  - `mass_defect.py`: Q-value calculations for fusion and fission reactions
  - `fusion_fission.py`: Analysis of iron boundary and energy release regimes

- **Documentation**
  - `DERIVATION_CHAIN.md`: Complete mathematical derivation
  - `VERIFICATION_REPORT.md`: Test results with accuracy metrics
  - `SCIENTIFIC_SIGNIFICANCE.md`: Physics implications
  - `USAGE_GUIDE.md`: API reference and examples
  - `INDEX.md`: Documentation overview

- **Key Results**
  - D-T fusion Q-value: 17.59 MeV (0.0% error)
  - Fe-56 binding energy: 492.22 MeV (0.01% error)
  - Iron peak emergence at A=52 (within 7% of experimental)
  - Overall binding energy RMS: 3.10%

### Technical Details

- Shell model corrections for light nuclei (A <= 4)
- Magic number bonuses for closed shells
- Asymmetry term with exponential softening

### Framework

- MIT License
- Python 3.8+ compatible
- Dependencies: numpy, scipy, matplotlib

---

## Roadmap

### [1.1.0] - Planned

- Coulomb barrier derivation
- Gamow tunneling probability
- Temperature-dependent cross-sections

### [1.2.0] - Planned

- CNO cycle implementation
- r-process nucleosynthesis
- Stellar burning sequences

### [2.0.0] - Future

- Full QCD connection
- Nuclear structure from triads
- Lattice verification of shell closures

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| 1.0.0 | 2026-01-23 | Initial release with binding energy and Q-value derivations |

---

## Contributing

See [docs/USAGE_GUIDE.md](docs/USAGE_GUIDE.md) for contribution guidelines.

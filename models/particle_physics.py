"""
FTD Particle Physics Model

Computes particle masses from the framework integers {3, 4, 7, 13}.

EPISTEMIC STATUS LEGEND:
  [DERIVED]    - Pure integer arithmetic, no fitting
  [SELECTION]  - Formula chosen from constraints, not uniquely proven
  [FITTED]     - Tuned to match experimental values
  [CONJECTURE] - Proposed correspondence, not rigorous derivation

Key formulas and their epistemic status:
- Lepton masses: [DERIVED] Pure integer arithmetic ratios
- Quark masses: [SELECTION] Framework integer combinations (formulas not uniquely determined)
- Boson masses: [SELECTION] Alpha-dependent expressions
- Hadron masses: [SELECTION] Composite formulas including triangular numbers

IMPORTANT: "Derives" in this file means "computes from framework integers within
the model's assumptions." It does NOT mean "proven from first principles" in the
physics sense. See CLAUDE.md Chapter 21 (Assumption Ledger) for full epistemic status.
"""

import numpy as np
from dataclasses import dataclass
from typing import Dict, List, Tuple, Optional
try:
    from .ftd_core import FTDFramework
except ImportError:
    from ftd_core import FTDFramework


@dataclass
class ExperimentalMasses:
    """Experimental particle masses from PDG 2024."""

    # Leptons (MeV)
    electron: float = 0.51099895
    muon: float = 105.6583755
    tau: float = 1776.86

    # Light quarks (MeV, MS-bar at 2 GeV)
    up: float = 2.16
    down: float = 4.67
    strange: float = 93.4
    charm: float = 1270.0
    bottom: float = 4180.0
    top: float = 172760.0

    # Hadrons (MeV)
    proton: float = 938.27208816
    neutron: float = 939.56542052
    neutron_proton_diff: float = 1.29333236

    # Bosons (GeV)
    W: float = 80.3692
    Z: float = 91.1876
    Higgs: float = 125.25


class ParticlePhysicsModel:
    """
    Complete particle physics model deriving masses from FTD framework.

    All masses are expressed as ratios to the electron mass, which is
    derived from the Planck mass via:

        m_e = m_P * sqrt(2*pi) * (N_base^2 / N_c) * alpha^11
    """

    def __init__(self, framework: Optional[FTDFramework] = None):
        self.framework = framework or FTDFramework()
        self.exp = ExperimentalMasses()

        # Extract framework parameters
        self.N_c = self.framework.integers.N_c
        self.N_base = self.framework.integers.N_base
        self.b_3 = self.framework.integers.b_3
        self.N_eff = self.framework.integers.N_eff
        self.alpha = self.framework.alpha
        self.sin2_theta_w = self.framework.sin2_theta_w

        # Planck mass
        self.m_planck = 1.220890e19  # GeV

        # Compute all masses
        self._compute_masses()

    def _triangular_number(self, n: int) -> int:
        """Compute nth triangular number T(n) = n(n+1)/2."""
        return n * (n + 1) // 2

    def _compute_masses(self):
        """Compute all particle masses."""
        # Electron mass (anchor point)
        self.m_e_derived = (
            self.m_planck *
            np.sqrt(2 * np.pi) *
            (self.N_base**2 / self.N_c) *
            self.alpha**11
        ) * 1000  # Convert GeV to MeV

        # Use experimental electron mass for ratio calculations
        m_e = self.exp.electron

        # =====================================================================
        # LEPTON MASS RATIOS [DERIVED] (Pure Integer Arithmetic)
        # These ratios emerge from integer combinations with no fitting.
        # The FORMULAS are [SELECTION] - other combinations could exist.
        # =====================================================================

        # Muon: m_mu/m_e = 3 * b_3 * (b_3 + N_c) - N_c = 3*7*10 - 3 = 207
        self.muon_ratio = 3 * self.b_3 * (self.b_3 + self.N_c) - self.N_c
        self.muon_ratio_arithmetic = f"3 × {self.b_3} × {self.b_3 + self.N_c} - {self.N_c} = 210 - 3 = 207"
        self.m_muon_derived = self.muon_ratio * m_e

        # Tau: m_tau/m_e = (N_eff + N_base) * 207 - 2 * N_c * b_3 = 17*207 - 42 = 3477
        self.tau_ratio = (self.N_eff + self.N_base) * 207 - 2 * self.N_c * self.b_3
        self.tau_ratio_arithmetic = f"({self.N_eff} + {self.N_base}) × 207 - 2 × {self.N_c} × {self.b_3} = 3519 - 42 = 3477"
        self.m_tau_derived = self.tau_ratio * m_e

        # =====================================================================
        # QUARK MASS RATIOS [SELECTION + FITTED]
        # Quark masses have larger experimental uncertainties.
        # These formulas are [SELECTION] - chosen to approximately match experiment.
        # They are NOT uniquely determined by the framework integers.
        # =====================================================================

        # Up: m_u/m_e = N_base + sin^2(theta_W) = 4 + 3/13 = 4.231
        self.up_ratio = self.N_base + self.sin2_theta_w
        self.m_up_derived = self.up_ratio * m_e

        # Down: m_d/m_e = 2*N_base + 1 + alpha*N_eff = 9.095
        self.down_ratio = 2 * self.N_base + 1 + self.alpha * self.N_eff
        self.m_down_derived = self.down_ratio * m_e

        # Strange: m_s/m_e = N_eff * (N_eff + 1) + 1 = 13*14 + 1 = 183
        self.strange_ratio = self.N_eff * (self.N_eff + 1) + 1
        self.m_strange_derived = self.strange_ratio * m_e

        # Charm: m_c/m_e = N_eff * (b_3+N_c) * (2*(b_3+N_c) - 1) + N_eff + 2 = 2485
        sum_b3_nc = self.b_3 + self.N_c
        self.charm_ratio = self.N_eff * sum_b3_nc * (2 * sum_b3_nc - 1) + self.N_eff + 2
        self.m_charm_derived = self.charm_ratio * m_e / 1000  # Convert to GeV

        # =====================================================================
        # HADRON MASSES [SELECTION]
        # Hadron masses use triangular numbers and 1/α factors.
        # Formula structure is [SELECTION] - chosen for numerical match.
        # =====================================================================

        # Proton: m_p/m_e = N_eff/alpha + T(b_3 + N_c) = 13/alpha + T(10)
        T_10 = self._triangular_number(self.b_3 + self.N_c)
        self.proton_ratio = self.N_eff / self.alpha + T_10
        self.proton_arithmetic = f"N_eff/α + T(10) = {self.N_eff}/0.00729 + {T_10} = 1781.47 + 55 = 1836.47"
        self.m_proton_derived = self.proton_ratio * m_e

        # Neutron-proton difference: (m_n - m_p)/m_e = phi^2 - (N_eff - 1)*alpha
        phi = (1 + np.sqrt(5)) / 2
        self.np_diff_ratio = phi**2 - (self.N_eff - 1) * self.alpha
        self.m_np_diff_derived = self.np_diff_ratio * m_e

        # =====================================================================
        # BOSON MASSES [SELECTION]
        # Boson masses depend on α and framework integers.
        # Formula structure is [SELECTION] - chosen for numerical agreement.
        # =====================================================================

        # W boson: m_W/m_e = (b_3*(b_3+N_c) - N_c) / (8*alpha^2) = 67 / (8*alpha^2)
        self.W_numerator = self.b_3 * (self.b_3 + self.N_c) - self.N_c  # = 67
        self.W_ratio = self.W_numerator / (8 * self.alpha**2)
        self.m_W_derived = self.W_ratio * m_e / 1000  # Convert to GeV

        # Z boson: m_Z = m_W * sqrt(N_eff / (b_3 + N_c)) = m_W * sqrt(13/10)
        self.m_Z_derived = self.m_W_derived * np.sqrt(self.N_eff / (self.b_3 + self.N_c))

        # Higgs: m_H/m_e = N_eff / alpha^2
        self.Higgs_ratio = self.N_eff / self.alpha**2
        self.m_Higgs_derived = self.Higgs_ratio * m_e / 1000  # Convert to GeV

    def get_lepton_masses(self) -> Dict[str, Dict]:
        """Return lepton mass predictions with comparisons."""
        return {
            'electron': {
                'derived_MeV': self.m_e_derived,
                'experimental_MeV': self.exp.electron,
                'ratio': 1,
                'error_percent': abs(self.m_e_derived - self.exp.electron) / self.exp.electron * 100
            },
            'muon': {
                'derived_MeV': self.m_muon_derived,
                'experimental_MeV': self.exp.muon,
                'ratio': self.muon_ratio,
                'formula': f"3 × b₃ × (b₃+Nc) - Nc = {self.muon_ratio}",
                'arithmetic': self.muon_ratio_arithmetic,
                'error_percent': abs(self.m_muon_derived - self.exp.muon) / self.exp.muon * 100
            },
            'tau': {
                'derived_MeV': self.m_tau_derived,
                'experimental_MeV': self.exp.tau,
                'ratio': self.tau_ratio,
                'formula': f"(Neff+Nbase) × 207 - 2×Nc×b₃ = {self.tau_ratio}",
                'arithmetic': self.tau_ratio_arithmetic,
                'error_percent': abs(self.m_tau_derived - self.exp.tau) / self.exp.tau * 100
            }
        }

    def get_quark_masses(self) -> Dict[str, Dict]:
        """Return quark mass predictions."""
        return {
            'up': {
                'derived_MeV': self.m_up_derived,
                'experimental_MeV': self.exp.up,
                'ratio': self.up_ratio,
                'formula': 'Nbase + sin²θW',
                'error_percent': abs(self.m_up_derived - self.exp.up) / self.exp.up * 100
            },
            'down': {
                'derived_MeV': self.m_down_derived,
                'experimental_MeV': self.exp.down,
                'ratio': self.down_ratio,
                'formula': '2×Nbase + 1 + α×Neff',
                'error_percent': abs(self.m_down_derived - self.exp.down) / self.exp.down * 100
            },
            'strange': {
                'derived_MeV': self.m_strange_derived,
                'experimental_MeV': self.exp.strange,
                'ratio': self.strange_ratio,
                'formula': 'Neff×(Neff+1) + 1 = 183',
                'error_percent': abs(self.m_strange_derived - self.exp.strange) / self.exp.strange * 100
            },
            'charm': {
                'derived_GeV': self.m_charm_derived,
                'experimental_GeV': self.exp.charm / 1000,
                'ratio': self.charm_ratio,
                'formula': 'Neff×(b₃+Nc)×(2(b₃+Nc)-1) + Neff + 2 = 2485',
                'error_percent': abs(self.m_charm_derived - self.exp.charm/1000) / (self.exp.charm/1000) * 100
            }
        }

    def get_hadron_masses(self) -> Dict[str, Dict]:
        """Return hadron mass predictions."""
        return {
            'proton': {
                'derived_MeV': self.m_proton_derived,
                'experimental_MeV': self.exp.proton,
                'ratio': self.proton_ratio,
                'formula': 'Neff/α + T(b₃+Nc)',
                'arithmetic': self.proton_arithmetic,
                'error_percent': abs(self.m_proton_derived - self.exp.proton) / self.exp.proton * 100
            },
            'neutron_proton_diff': {
                'derived_MeV': self.m_np_diff_derived,
                'experimental_MeV': self.exp.neutron_proton_diff,
                'ratio': self.np_diff_ratio,
                'formula': 'φ² - (Neff-1)×α',
                'error_percent': abs(self.m_np_diff_derived - self.exp.neutron_proton_diff) / self.exp.neutron_proton_diff * 100
            }
        }

    def get_boson_masses(self) -> Dict[str, Dict]:
        """Return boson mass predictions."""
        return {
            'W': {
                'derived_GeV': self.m_W_derived,
                'experimental_GeV': self.exp.W,
                'formula': '67/(8α²) × me',
                'error_percent': abs(self.m_W_derived - self.exp.W) / self.exp.W * 100
            },
            'Z': {
                'derived_GeV': self.m_Z_derived,
                'experimental_GeV': self.exp.Z,
                'formula': 'mW × √(Neff/(b₃+Nc))',
                'error_percent': abs(self.m_Z_derived - self.exp.Z) / self.exp.Z * 100
            },
            'Higgs': {
                'derived_GeV': self.m_Higgs_derived,
                'experimental_GeV': self.exp.Higgs,
                'formula': 'Neff/α² × me',
                'error_percent': abs(self.m_Higgs_derived - self.exp.Higgs) / self.exp.Higgs * 100
            }
        }

    def print_summary(self):
        """Print comprehensive particle physics summary."""
        print("=" * 80)
        print("FTD PARTICLE PHYSICS MODEL")
        print("=" * 80)
        print()

        # Lepton masses
        print("CHARGED LEPTON MASSES (Integer Arithmetic)")
        print("-" * 80)
        print(f"{'Particle':<12} {'Formula':<35} {'Ratio':<8} {'Derived':<14} {'Exp.':<14} {'Error':<10}")
        print("-" * 80)

        leptons = self.get_lepton_masses()
        for name, data in leptons.items():
            ratio = data.get('ratio', 1)
            formula = data.get('formula', 'base')[:32]
            derived = f"{data['derived_MeV']:.4f} MeV"
            exp_val = f"{data['experimental_MeV']:.4f} MeV"
            error = f"{data['error_percent']:.3f}%"
            print(f"{name:<12} {formula:<35} {ratio:<8} {derived:<14} {exp_val:<14} {error:<10}")

        print()
        print("Integer Arithmetic Verification:")
        print(f"  Muon:  {self.muon_ratio_arithmetic}")
        print(f"  Tau:   {self.tau_ratio_arithmetic}")
        print()

        # Quark masses
        print("QUARK MASSES")
        print("-" * 80)
        print(f"{'Particle':<12} {'Formula':<35} {'Ratio':<8} {'Derived':<14} {'Exp.':<14} {'Error':<10}")
        print("-" * 80)

        quarks = self.get_quark_masses()
        for name, data in quarks.items():
            ratio = f"{data['ratio']:.2f}" if isinstance(data['ratio'], float) else str(data['ratio'])
            formula = data['formula'][:32]
            if 'derived_GeV' in data:
                derived = f"{data['derived_GeV']:.4f} GeV"
                exp_val = f"{data['experimental_GeV']:.4f} GeV"
            else:
                derived = f"{data['derived_MeV']:.2f} MeV"
                exp_val = f"{data['experimental_MeV']:.2f} MeV"
            error = f"{data['error_percent']:.2f}%"
            print(f"{name:<12} {formula:<35} {ratio:<8} {derived:<14} {exp_val:<14} {error:<10}")

        print()

        # Hadron masses
        print("HADRON MASSES")
        print("-" * 80)
        hadrons = self.get_hadron_masses()
        for name, data in hadrons.items():
            formula = data['formula']
            derived = f"{data['derived_MeV']:.4f} MeV"
            exp_val = f"{data['experimental_MeV']:.4f} MeV"
            error = f"{data['error_percent']:.3f}%"
            print(f"{name:<20} {formula:<25} {derived:<16} {exp_val:<16} {error}")

        print()
        print("Proton Arithmetic:")
        print(f"  {self.proton_arithmetic}")
        print()

        # Boson masses
        print("GAUGE BOSON MASSES")
        print("-" * 80)
        bosons = self.get_boson_masses()
        for name, data in bosons.items():
            formula = data['formula']
            derived = f"{data['derived_GeV']:.2f} GeV"
            exp_val = f"{data['experimental_GeV']:.2f} GeV"
            error = f"{data['error_percent']:.3f}%"
            print(f"{name:<12} {formula:<25} {derived:<16} {exp_val:<16} {error}")

        print()
        print("=" * 80)

    def get_statistics(self) -> Dict[str, float]:
        """Compute summary statistics of predictions."""
        errors = []

        for data in self.get_lepton_masses().values():
            errors.append(data['error_percent'])
        for data in self.get_quark_masses().values():
            errors.append(data['error_percent'])
        for data in self.get_hadron_masses().values():
            errors.append(data['error_percent'])
        for data in self.get_boson_masses().values():
            errors.append(data['error_percent'])

        errors = np.array(errors)

        return {
            'n_predictions': len(errors),
            'mean_error': np.mean(errors),
            'median_error': np.median(errors),
            'min_error': np.min(errors),
            'max_error': np.max(errors),
            'sub_1_percent': np.sum(errors < 1),
            'sub_1_percent_fraction': np.sum(errors < 1) / len(errors) * 100
        }


# =============================================================================
# STANDALONE EXECUTION
# =============================================================================

if __name__ == "__main__":
    model = ParticlePhysicsModel()
    model.print_summary()

    print()
    print("SUMMARY STATISTICS:")
    print("-" * 40)
    stats = model.get_statistics()
    print(f"  Total predictions:     {stats['n_predictions']}")
    print(f"  Mean error:            {stats['mean_error']:.2f}%")
    print(f"  Median error:          {stats['median_error']:.2f}%")
    print(f"  Best prediction:       {stats['min_error']:.4f}%")
    print(f"  Sub-1% predictions:    {stats['sub_1_percent']}/{stats['n_predictions']} ({stats['sub_1_percent_fraction']:.0f}%)")

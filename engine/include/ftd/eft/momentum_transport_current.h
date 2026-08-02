#pragma once
/**
 * @file momentum_transport_current.h
 * @brief Observer-only discrete momentum stress ledger T^(i) / S^(i).
 *
 * Implements, for the pre-registration
 * docs/theory/10_eft_program/preregistrations/
 * PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md:
 *
 *   Sec 2.2  restriction lemma  (skewness relation (S), chord form (R))
 *   Sec 2.3  unit-bond form (U) and the site-mask collapse
 *   Sec 2.4  localization L1 ("E-carries") and its regional identity (M1)
 *   Sec 2.5  localization L2 ("B-carries") and its regional identity (M2)
 *   Sec 2.7  cumulative moving-mask accumulators F_i, W_i, Q_i and identity (L)
 *   Sec 2.9  retention eta_i, transfer tau_i, coverage rho_i, enclosure kappa_i
 *
 * The frozen Sec 0 operators are transcribed directly from the engine source
 * (matched_gauss_transport.cpp:183-243, matched_face_energy_transaction.h:
 * 150-172, matched_face_momentum_transaction.h:46-92):
 *
 *   d_a^- = I-T_{-e_a},                 d_a^+ = T_{e_a}-I,
 *   (C B)_a   = eps_{abc} d_b^- B_c     (matched_curl,         edge -> face)
 *   (C^T E)_a = eps_{abc} d_b^+ E_c     (matched_curl_adjoint, face -> edge)
 *   D_i       = (1/2)(T_{e_i}-T_{-e_i}) componentwise
 *   B' = B-lambda C^T E,  E' = E+lambda C B',  E'' = E'-K
 *   u := C B'  (face),  w := C^T E  (edge),  M := C C^T,  M' := C^T C
 *
 * The four operators required by Sec 4 item 1 are
 *
 *   L1: N = D_i        on u = C B'   (after-step)   -> Phi^(u)
 *       N = D_i C C^T  on E          (before-step)  -> Phi^(E)
 *   L2: N = D_i        on w = C^T E  (before-step)  -> Phi^(w)
 *       N = D_i C^T C  on B'         (after-step)   -> Phi^(B')
 *
 * Banned move B1 is structural here: every masked reduction Phi_i[chi] is
 * summed directly from T^(i) / S^(i) on chords straddling dOmega, and is
 * never formed as region_after - region_before or any other residual
 * difference of regional totals.
 *
 * This translation unit touches no production path, no RenderBridge, no
 * scenario, and no existing test.  It is observer-only research
 * instrumentation.
 */

#include "ftd/eft/matched_face_momentum_transaction.h"

#include <array>
#include <cstddef>
#include <vector>

namespace ftd::eft {

// ---------------------------------------------------------------------------
// Field views
// ---------------------------------------------------------------------------

/**
 * Borrowed, non-owning view of one matched vector field.  MatchedFaceFlux and
 * MatchedEdgeField share the same (L; x,y,z) storage layout, so one view type
 * serves both families; the family is carried by the caller, not the view.
 */
struct MomentumFieldView {
  int L = 0;
  const double* component[3]{};

  bool valid() const {
    return L > 0 && component[0] != nullptr && component[1] != nullptr
        && component[2] != nullptr;
  }
  std::size_t index(int x, int y, int z) const;
  double at(int a, int x, int y, int z) const;
};

MomentumFieldView momentum_view(const MatchedFaceFlux& field);
MomentumFieldView momentum_view(const MatchedEdgeField& field);

// ---------------------------------------------------------------------------
// Masks
// ---------------------------------------------------------------------------

/**
 * Region mask on the augmented (component, site) graph of Sec 2.2.
 *
 * The production form is the component-independent Chebyshev site mask of
 * Sec 5, evaluated at the INTEGER lattice site (no staggered half-offsets):
 *
 *   Omega_R(tau) = { x : ||x-c(tau)||_inf <= R },  c(tau) integer.
 *
 * Under that mask the S^(i) channel drops out identically (Sec 2.3 site-mask
 * collapse).  The genuinely per-component form exists so that the Sec 6.4
 * L=11 exactness pre-check can exercise S^(i), which the production path never
 * reaches.
 */
struct MomentumMask {
  int L = 0;
  bool per_component = false;
  bool complemented = false;
  bool universal = false;
  int center[3]{};
  int radius = 0;
  std::array<std::vector<unsigned char>, 3> component{};

  bool valid() const;
  bool inside(int a, int x, int y, int z) const;
};

/** Component-independent Chebyshev site mask about an integer centre. */
MomentumMask make_momentum_site_mask(int L, int cx, int cy, int cz, int radius);
/** chi == 1 everywhere (the whole-domain reference mask of Sec 3's G_U). */
MomentumMask make_momentum_universal_mask(int L);
/** Deterministic per-component challenge mask for the Sec 6.4 pre-check. */
MomentumMask make_momentum_component_challenge_mask(int L, int cx, int cy,
                                                    int cz, int radius);
/** chi -> 1-chi.  Sec 2.2: Phi[chi]+Phi[1-chi] = 0 termwise. */
MomentumMask complement_momentum_mask(const MomentumMask& mask);

// ---------------------------------------------------------------------------
// Chord tables (Sec 2.2, Sec 2.3)
// ---------------------------------------------------------------------------

enum class MomentumOperatorKind {
  CentralDifference,  ///< N = D_i (componentwise; family-agnostic)
  FaceBinding,        ///< N = D_i C C^T (face -> face)
  EdgeBinding,        ///< N = D_i C^T C (edge -> edge)
};

/** One R+ representative (r,a,b) with coefficient N_r[a][b] (Sec 2.2). */
struct MomentumChordClass {
  int r[3]{};
  int a = 0;
  int b = 0;
  double coefficient = 0.0;
  int l1 = 0;
  int linf = 0;
};

/**
 * One generator of T^(i)_{a,d}(v) (Sec 2.3).  The contribution is
 *
 *   T^(i)_{a,d}(v) += weight * f_a(v-base) * f_b(v-base+r)
 *
 * with weight = (+-1)*N_r[a][b]; base = p_k for a +e_d step and base =
 * p_{k+1} for a -e_d step along the frozen lexicographic path (x, then y,
 * then z, from v toward v+r).  Changing this convention is Banned move B5.
 */
struct MomentumBondGenerator {
  int axis = 0;
  int base[3]{};
  int r[3]{};
  int a = 0;
  int b = 0;
  double weight = 0.0;
};

/**
 * One generator of S^(i)_{a,b}(v) (Sec 2.3):
 *
 *   S^(i)_{a,b}(v) += coefficient * f_a(v-r) * f_b(v-r+r) = N_r[a][b]
 *                     * f_a(v-r) * f_b(v)
 *
 * written here as W_{r,a,b}(v-r).  Entries with a == b are dropped: their
 * mask factor chi_a(v)-chi_a(v) vanishes identically.
 */
struct MomentumSiteGenerator {
  int r[3]{};
  int a = 0;
  int b = 0;
  double coefficient = 0.0;
};

/** Census + generators for one operator N and one component i. */
struct MomentumTransportCurrentTable {
  bool valid = false;
  MomentumOperatorKind kind = MomentumOperatorKind::CentralDifference;
  int component = 0;
  int probe_size = 0;
  int displacement_count = 0;   ///< distinct nonzero r
  int entry_count = 0;          ///< nonzero (r,a,b)
  int class_count = 0;          ///< |R+|
  int maximum_l1 = 0;
  int maximum_linf = 0;
  double skewness_residual = 0.0;   ///< max |N_{-r}+N_r^T| (Sec 2.2 (S))
  double aliasing_margin = 0.0;     ///< probe_size/2 - maximum_linf
  std::vector<MomentumChordClass> classes;
  std::vector<MomentumBondGenerator> bond;
  std::vector<MomentumSiteGenerator> site;
};

/**
 * Impulse-response extraction of N_r followed by the Sec 2.2 R+ selection and
 * the Sec 2.3 unit-bond path decomposition.  Nothing is hand-transcribed: the
 * stencil is measured from the frozen operators themselves on a probe torus.
 *
 * R+ representative rule (frozen with this implementation; Banned move B5):
 * pair (r,a,b) with (-r,b,a) and keep the member whose key
 * (r_x,r_y,r_z,a,b) is lexicographically greater.  The two keys are never
 * equal because N_0 is antisymmetric, so the diagonal r=0, a=b vanishes.
 */
MomentumTransportCurrentTable build_momentum_transport_current_table(
    MomentumOperatorKind kind, int component, int probe_size = 11);

// ---------------------------------------------------------------------------
// Masked reductions (Sec 2.3 (U))
// ---------------------------------------------------------------------------

/**
 * Phi_i[chi] = <chi f, N f>, computed ONLY by summing T^(i) and S^(i) on
 * chords that straddle dOmega in the augmented (component, site) graph.
 * Never a residual difference of regional totals (Banned move B1).
 */
double masked_chord_flux(const MomentumTransportCurrentTable& table,
                         const MomentumFieldView& field,
                         const MomentumMask& mask);

/** The T^(i) half of Phi_i[chi] alone (unit-bond channel). */
double masked_bond_flux(const MomentumTransportCurrentTable& table,
                        const MomentumFieldView& field,
                        const MomentumMask& mask);

/** The S^(i) half of Phi_i[chi] alone (on-site component-crossing channel). */
double masked_site_flux(const MomentumTransportCurrentTable& table,
                        const MomentumFieldView& field,
                        const MomentumMask& mask);

/**
 * Independent reference route for the Sec 6.4 flux checks: apply N densely
 * with the frozen operators and contract against chi f.  This is the quantity
 * the (T,S) construction must reproduce; it is a cross-check, never the
 * production path.
 */
double direct_masked_bilinear(MomentumOperatorKind kind, int component,
                              const MomentumFieldView& field,
                              const MomentumMask& mask);

/** Dense application of N to a field, using the frozen Sec 0 operators. */
std::array<std::vector<double>, 3> apply_momentum_operator(
    MomentumOperatorKind kind, int component, const MomentumFieldView& field);

/**
 * Dense T^(i)_{a,d}(v) and S^(i)_{a,b}(v) arrays -- the document's "stress
 * ledger" object.  Materialized only for the small pre-check lattices; the
 * production instrument evaluates the same generators on the fly.
 */
struct MomentumStressLedgerArrays {
  int L = 0;
  /// bond[d][a][site] = T^(i)_{a,d}(v)
  std::array<std::array<std::vector<double>, 3>, 3> bond{};
  /// site[a][b][site] = S^(i)_{a,b}(v)
  std::array<std::array<std::vector<double>, 3>, 3> site{};
};

MomentumStressLedgerArrays build_momentum_stress_ledger_arrays(
    const MomentumTransportCurrentTable& table,
    const MomentumFieldView& field);

double masked_flux_from_arrays(const MomentumStressLedgerArrays& arrays,
                               const MomentumMask& mask);

// ---------------------------------------------------------------------------
// Per-tick localization terms (Sec 2.4 M1, Sec 2.5 M2, Sec 2.7)
// ---------------------------------------------------------------------------

enum class MomentumLocalization {
  ECarries,  ///< L1, canonical: pi^(1)(a,v) = E_a(v) (D_i C B)_a(v)
  BCarries,  ///< L2, alternate: pi^(2)(a,v) = -B_a(v) (C^T D_i E)_a(v)
};

/** The five snapshots one staggered tick exposes. */
struct MomentumStepFields {
  const MatchedFaceFlux* electric_before = nullptr;      ///< E
  const MatchedEdgeField* magnetic_before = nullptr;     ///< B
  const MatchedEdgeField* magnetic_after = nullptr;      ///< B'
  const MatchedFaceFlux* electric_pre_current = nullptr; ///< E'
  const MatchedFaceFlux* electric_after = nullptr;       ///< E'' = E'-K
  double lambda = 0.0;
};

/**
 * One tick's contribution to the Sec 2.7 ledger, for one localization, one
 * component, and one mask pair (chi_t, chi_{t+1}).  Every member is a scalar
 * reduction; nothing here is a full field.
 */
struct MomentumLedgerTickTerms {
  bool valid = false;
  double phi_plain = 0.0;              ///< Phi^(u) [L1] / Phi^(w) [L2]
  double phi_binding = 0.0;            ///< Phi^(E) [L1] / Phi^(B') [L2]
  double phi_plain_complement = 0.0;   ///< same, mask 1-chi
  double phi_binding_complement = 0.0; ///< same, mask 1-chi
  double sweep = 0.0;                  ///< sum (chi_{t+1}-chi_t) pi^t
  double sweep_complement = 0.0;       ///< same, complement masks
  double source = 0.0;                 ///< Q increment (sign per Sec 2.7)
  double content_after = 0.0;          ///< sum chi_{t+1} pi^{t+1}
  double content_before = 0.0;         ///< sum chi_{t+1} pi^t
  double content_old = 0.0;            ///< sum chi_t     pi^t = Pi_i(R,t)

  /// lambda*(Phi_plain - Phi_binding): the F increment of Sec 2.7.
  double flux(double lambda) const { return lambda * (phi_plain - phi_binding); }
  double flux_complement(double lambda) const {
    return lambda * (phi_plain_complement - phi_binding_complement);
  }
  /// Left-hand side of (M1)/(M2) for the contemporaneous mask.
  double material() const { return content_after - content_before; }
  /// (M1)/(M2) residual: material - flux + source.
  double identity_residual(double lambda) const {
    return material() - flux(lambda) + source;
  }
  /// Sec 6.2 G1 Reynolds residual.
  double reynolds_residual() const {
    return (content_after - content_old) - material() - sweep;
  }
  /// Sec 6.1 G0 complementarity residual for the flux pair.
  double complementarity_residual(double lambda) const {
    return flux(lambda) + flux_complement(lambda);
  }
  double identity_scale(double lambda) const;
  double reynolds_scale() const;
};

/** Sec 3: every momentum-sector quantity carries the interaction_scale. */
void scale_momentum_ledger_tick_terms(MomentumLedgerTickTerms& terms,
                                      double scale);

/**
 * Host reference evaluation of one tick's terms.  Materializes the densities
 * and is intended for the small Sec 6.4 / Sec 8 lattices and CPU/GPU parity;
 * the production L=321 path uses the fused CUDA kernel.
 */
MomentumLedgerTickTerms observe_momentum_ledger_tick(
    const MomentumStepFields& fields,
    MomentumLocalization localization,
    int component,
    const MomentumMask& previous_mask,
    const MomentumMask& current_mask,
    const MomentumTransportCurrentTable& plain_table,
    const MomentumTransportCurrentTable& binding_table);

/** pi_i^(1) or pi_i^(2) on the before-side snapshot (Sec 2.4 / Sec 2.5). */
std::array<std::vector<double>, 3> momentum_density_before(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component);
/** pi_i^(1) or pi_i^(2) on the after-side snapshot. */
std::array<std::vector<double>, 3> momentum_density_after(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component);
/** Per-site Q density: K.(D_i C B') [L1] or -B'.(D_i C^T K) [L2]. */
std::array<std::vector<double>, 3> momentum_source_density(
    const MomentumStepFields& fields, MomentumLocalization localization,
    int component);

// ---------------------------------------------------------------------------
// Cumulative accumulators (Sec 2.7 (L)) and ratios (Sec 2.9)
// ---------------------------------------------------------------------------

/**
 * F_i(R,tau), W_i(R,tau), Q_i(R,tau) accumulated EVERY TICK from the boost.
 * Checkpoints are readout points of these accumulators, never sampling points
 * of the underlying rates (Sec 4 item 3, Banned move B10).
 */
struct MomentumLedgerAccumulator {
  bool initialized = false;
  long long ticks = 0;
  long double flux = 0.0L;             ///< F_i(R,tau)
  long double flux_complement = 0.0L;  ///< F_i[1-chi](R,tau)
  long double sweep = 0.0L;            ///< W_i(R,tau)
  long double sweep_complement = 0.0L; ///< W_i[1-chi](R,tau)
  long double source = 0.0L;           ///< Q_i(R,tau)
  double initial_content = 0.0;        ///< Pi_i(R,0)
  double content = 0.0;                ///< Pi_i(R,tau)
  double maximum_tick_identity_residual = 0.0;
  double maximum_tick_identity_ratio = 0.0;
  double maximum_reynolds_residual = 0.0;
  double maximum_reynolds_ratio = 0.0;
  double maximum_chain_residual = 0.0;
  double window_maximum_tick_identity_residual = 0.0;

  void add(const MomentumLedgerTickTerms& terms, double lambda);
  void begin_checkpoint_window();

  double flux_total() const { return static_cast<double>(flux); }
  double flux_complement_total() const {
    return static_cast<double>(flux_complement);
  }
  double sweep_total() const { return static_cast<double>(sweep); }
  double sweep_complement_total() const {
    return static_cast<double>(sweep_complement);
  }
  double source_total() const { return static_cast<double>(source); }
  double content_change() const { return content - initial_content; }
  double transfer_total() const { return flux_total() + sweep_total(); }
  /// Residual of (L): [Pi(tau)-Pi(0)] - F - W + Q.
  double ledger_residual() const;
  double ledger_scale() const;
  double complementarity_residual() const {
    return flux_total() + flux_complement_total();
  }
};

/** Sec 2.9 retention / transfer / coverage / enclosure diagnostics. */
struct MomentumRetentionRatios {
  bool resolved = false;
  double eta = 0.0;            ///< [Pi(R,tau)-Pi(R,0)] / Delta P^whole
  double transfer = 0.0;       ///< [F+W] / Delta P^whole  (tau_i)
  double rho = 0.0;            ///< |F+W| / max(|D|,1e-9)
  double rho_ceiling = 0.0;    ///< |Delta P^whole| / |D|
  double kappa = 0.0;          ///< Q(R,tau)/Q(R_out,tau)
  double identity_residual = 0.0;  ///< eta - transfer - 1  (Sec 2.9 (N))
};

MomentumRetentionRatios compute_momentum_retention_ratios(
    const MomentumLedgerAccumulator& region,
    double outer_source,
    double whole_domain_change,
    double matter_defect);

}  // namespace ftd::eft

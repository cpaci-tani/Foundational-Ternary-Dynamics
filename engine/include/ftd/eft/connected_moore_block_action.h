#pragma once
/**
 * @file connected_moore_block_action.h
 * @brief Runtime-size Moore-local integer-carrier common action (FTD-0622).
 */

#include "ftd/eft/constituent_complete_charged_trimer.h"
#include "ftd/eft/quadratic_coat_orbit_gather.h"
#include "ftd/eft/spline_poynting_momentum.h"

#include <cstddef>
#include <functional>
#include <vector>

namespace ftd::eft {

struct MooreBindingEdge {
  std::size_t first = 0;
  std::size_t second = 0;
  Coord reference_delta{};
  double rest_length_squared = 0.0;
};

struct ConnectedMooreBlockState {
  MatchedFaceFlux electric;
  MatchedEdgeField magnetic_half;
  std::vector<MatchedMatterPoint> constituents;
  std::vector<int> charges;
  std::vector<MooreBindingEdge> edges;
  int width = 0;
  int orientation_axis = -1;

  explicit ConnectedMooreBlockState(int size = 0)
      : electric(size), magnetic_half(size) {}
};

enum class ConnectedBindingLaw {
  FixedEdgeQuartic,
  DerivedCompactPair,
};

using ConnectedMooreBlockLocalOrbitGather = std::function<
    std::vector<QuadraticCoatOrbitGatherResult>(
        const std::vector<QuadraticCoatFaceCurrent>&,
        const std::vector<Vec3>&,
        double current_scale,
        double temporal_scale,
        double beta,
        double polarity_scale)>;

struct ConnectedMooreBlockOptions {
  double wave_speed = C_SPEED;
  double dt = 1.0;
  double binding_stiffness = 1.0;
  // Default-preserving FTD-0722 research branch.  The fixed-edge law retains
  // every established connected-block equation.  DerivedCompactPair requires
  // exactly two opposite-polarity constituents and no stored edges; its
  // interaction graph is recomputed from instantaneous separation.
  ConnectedBindingLaw binding_law = ConnectedBindingLaw::FixedEdgeQuartic;
  double compact_pair_well_depth = 0.01;
  double compact_pair_cutoff_distance_squared = 1.5;
  // Observer-only resolution parameters (FTD-0649).  Defaults preserve every
  // earlier connected-block equation exactly.  A scaled branch must apply
  // polarity_scale to source, current, density diagnostics, and gather;
  // field_energy_scale multiplies the common field/work coefficient.
  double constituent_mass_scale = 1.0;
  double polarity_scale = 1.0;
  double field_energy_scale = 1.0;
  double gate_tolerance = 1e-10;
  double solve_tolerance = 2e-11;
  double finite_difference_scale = 2e-7;
  int max_iterations = 48;
  // Research-only nonlinear-solver acceleration.  The exact residual, root
  // tolerance, and accepted state are unchanged.  The quasi-Newton Jacobian
  // starts from d(p1-p0)/dp1 = I and stores exact Broyden corrections in
  // low-rank form, solved through Woodbury rather than a dense 3N matrix.
  // False preserves the established central-difference solver path.
  bool use_low_rank_identity_broyden = false;
  // Research-only Jacobian-free Newton--Krylov path.  Jacobian-vector products
  // are directional differences of the unchanged exact residual; accepted
  // states still satisfy solve_tolerance.  Intended for refined composites
  // where a dense 3N Jacobian is computationally inappropriate.
  bool use_matrix_free_newton_krylov = false;
  // Default-off exact storage optimization for large empty causal buffers.
  // The deposited current and common-action residual are unchanged; only the
  // per-constituent five-array dense materialization is omitted.
  bool use_sparse_local_current = false;
  // Default-off exact residual-storage optimization.  Nonlinear probes use
  // the same sparse deposited current and local orbit gather without copying
  // full L^3 candidate fields.  The accepted root is materialized once and
  // rechecked by the established complete transaction before it can pass.
  bool use_local_residual_evaluation = false;
  // FTD-0759 default-empty implementation callback.  When supplied, local
  // nonlinear-root probes evaluate the unchanged sparse midpoint orbit gather
  // through the caller's resident-field backend.  It changes no equation,
  // tolerance, root seed, or production default.
  ConnectedMooreBlockLocalOrbitGather resident_local_orbit_gather;
  // CUDA/research orchestration hook.  When true, the host solver returns an
  // accepted, fully materialized matter/field transaction with only local
  // algebraic diagnostics completed.  The caller must then supply independently
  // measured volume diagnostics through complete_connected_moore_block_volume_diagnostics.
  // False preserves the established solver and all production defaults.
  bool defer_volume_diagnostics = false;
  // Observer-only final-root regularity measurement (FTD-0735).  When true,
  // the accepted implicit momentum root is re-evaluated with centered
  // differences at h and h/2 and the singular spectrum of dR/dp is recorded.
  // The measured Jacobian is never used by the nonlinear solve and therefore
  // cannot change the selected endpoint.
  bool measure_final_root_regularity = false;
  // Observer-only chart-fibre continuation (FTD-0626). False preserves the
  // independent one-record-per-anchor gate used by FTD-0622--0625.
  bool allow_shared_anchor_chart = false;
  // Observer-only nonlinear-root seed (FTD-0720).  Empty preserves the
  // established incoming-momentum seed.  A nonempty vector changes only the
  // initial guess supplied to the unchanged exact residual solver; it must
  // contain one finite momentum per constituent.
  std::vector<Vec3> root_momentum_seed;
};

struct ConnectedMooreBlockInitialization {
  bool valid = false;
  bool graph_connected = false;
  bool graph_local = false;
  bool site_projection_valid = false;
  int poisson_iterations = 0;
  double poisson_residual = 0.0;
  double gauss_residual = 0.0;
  double curl_adjoint_residual = 0.0;
  ConnectedMooreBlockState state;

  explicit ConnectedMooreBlockInitialization(int size = 0) : state(size) {}
};

// Observer-only compact Gauss preparation (FTD-0739).  The neutral density is
// solved on a finite induced cubic graph with zero crossing flux.  This is the
// unique minimum-energy face field under that selected support constraint; it
// is not the quotient-wide longitudinal minimum and is never used by the
// production tick.
struct FiniteSupportPairPreparation {
  bool valid = false;
  bool neutral = false;
  bool density_contained = false;
  bool compact_support = false;
  bool zero_boundary_crossing = false;
  int support_half_width = 0;
  int support_site_count = 0;
  int poisson_iterations = 0;
  // Physical constituent centroid and the selected integer support chart.
  // They coincide for the historical/default preparation. FTD-0763 may
  // explicitly admit a fractional physical center while retaining the same
  // finite induced lattice graph about support_center.
  Vec3 center{};
  Vec3 support_center{};
  Vec3 fractional_center_offset{};
  bool fractional_center_enabled = false;
  double poisson_residual = 0.0;
  double gauss_residual = 0.0;
  double outside_maximum = 0.0;
  double boundary_crossing_maximum = 0.0;
  double internal_circulation_residual = 0.0;
  double curl_adjoint_residual = 0.0;
  double electric_energy = 0.0;
  ConnectedMooreBlockState state;

  explicit FiniteSupportPairPreparation(int size = 0) : state(size) {}
};

struct ConnectedMooreBlockSolveDiagnostics {
  bool attempted = false;
  bool converged = false;
  int iterations = 0;
  int residual_evaluations = 0;
  int rejected_steps = 0;
  double residual = 0.0;
  double step_residual = 0.0;
  double minimum_abs_jacobian_pivot = 0.0;
  int jacobian_refreshes = 0;
  int jacobian_reuses = 0;
  int identity_broyden_seeds = 0;
  int krylov_matvecs = 0;
  int full_candidate_materializations = 0;
  double materialized_residual_difference = 0.0;
  // A supplied acceleration cache may fail to converge even when the
  // canonical uncached Newton solve succeeds.  Such a miss is retried once
  // with the unchanged residual equation and recorded explicitly.
  int cache_fallbacks = 0;
  int discarded_cache_residual_evaluations = 0;
  bool final_root_regularity_measured = false;
  int regularity_residual_evaluations = 0;
  double final_minimum_singular_value = 0.0;
  double final_maximum_singular_value = 0.0;
  double final_condition_number = 0.0;
  double regularity_scale_relative_difference = 0.0;
};

// Observer/research acceleration for repeated nearby solves.  The cached
// matrix changes only the nonlinear root-finding path: every accepted state is
// still evaluated by the exact common-action residual and must satisfy the
// unchanged solve tolerance.  Passing no cache preserves the production path.
struct ConnectedMooreBlockSolveCache {
  bool valid = false;
  std::size_t dimension = 0;
  std::vector<double> jacobian;

  void reset() {
    valid = false;
    dimension = 0;
    jacobian.clear();
  }
};

struct ConnectedMooreBlockStepResult {
  bool valid = false;
  bool common_action_gates_pass = false;
  bool volume_diagnostics_pending = false;
  bool forward = true;
  bool graph_connected = false;
  bool graph_local = false;
  bool relational_edge_before = false;
  bool relational_edge_after = false;
  bool relational_graph_changed = false;
  bool site_projection_valid = false;
  int net_charge = 0;
  int site_hops = 0;
  FaceFluxNormalization normalization{};
  double interaction_scale = 0.0;
  double constituent_mass_scale = 1.0;
  double polarity_scale = 1.0;
  double field_energy_scale = 1.0;
  ConnectedMooreBlockState earlier;
  ConnectedMooreBlockState later;
  ConnectedMooreBlockSolveDiagnostics solve{};
  std::vector<QuadraticCoatFaceCurrent> segments;
  std::vector<QuadraticCoatOrbitGatherResult> gathers;
  std::vector<Vec3> velocities;
  std::vector<Vec3> electric_impulses;
  std::vector<Vec3> magnetic_impulses;
  std::vector<Vec3> binding_impulses;
  std::vector<Vec3> total_impulses;

  double kinetic_energy_before = 0.0;
  double kinetic_energy_after = 0.0;
  double binding_energy_before = 0.0;
  double binding_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_work = 0.0;
  Vec3 center_before{};
  Vec3 center_after{};
  Vec3 matter_momentum_before{};
  Vec3 matter_momentum_after{};
  Vec3 local_field_momentum_before{};
  Vec3 local_field_momentum_after{};
  Vec3 spline_field_momentum_before{};
  Vec3 spline_field_momentum_after{};
  Vec3 local_total_defect{};
  Vec3 spline_total_defect{};

  double root_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double force_residual = 0.0;
  double kinematic_residual = 0.0;
  double kinetic_discrete_gradient_residual = 0.0;
  double electric_adjoint_residual = 0.0;
  double magnetic_work_residual = 0.0;
  double binding_work_residual = 0.0;
  double binding_impulse_sum_residual = 0.0;
  double matter_work_residual = 0.0;
  double field_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double causal_speed_excess = 0.0;
  double center_displacement = 0.0;
  double maximum_edge_strain = 0.0;
  double local_defect_norm = 0.0;
  double spline_defect_norm = 0.0;
  double normalized_spline_defect = 0.0;

  explicit ConnectedMooreBlockStepResult(int size = 0)
      : earlier(size), later(size) {}
};

struct ConnectedMooreBlockVolumeDiagnostics {
  bool valid = false;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  Vec3 local_field_momentum_before{};
  Vec3 local_field_momentum_after{};
  Vec3 spline_field_momentum_before{};
  Vec3 spline_field_momentum_after{};
};

ConnectedMooreBlockInitialization initialize_connected_moore_block(
    int L, int width, int orientation_axis, int phase_axis, double phase,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

// Observer-only minimum-energy longitudinal redressing for an already supplied
// constituent geometry (FTD-0628). Constituent state and graph metadata are
// copied unchanged; electric and magnetic fields are rebuilt from scratch.
ConnectedMooreBlockInitialization redress_connected_moore_block(
    const ConnectedMooreBlockState& geometry,
    bool allow_shared_anchor_chart = false,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

// Observer-only finite-fibre form (FTD-0632). The cap constrains chart records
// sharing an anchor; it does not add an interaction or alter effective
// positions. The legacy Boolean API above remains exactly cap one/cap two.
ConnectedMooreBlockInitialization redress_connected_moore_block_with_fibre_limit(
    const ConnectedMooreBlockState& geometry,
    int maximum_anchor_multiplicity,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

// Observer-only minimum-energy longitudinal dressing for the FTD-0722
// two-constituent derived-graph branch.  The supplied geometry must have two
// opposite charges and an empty stored edge list.
ConnectedMooreBlockInitialization redress_derived_compact_pair(
    const ConnectedMooreBlockState& geometry,
    const ConnectedMooreBlockOptions& options,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

FiniteSupportPairPreparation prepare_finite_support_derived_compact_pair(
    const ConnectedMooreBlockState& geometry,
    const ConnectedMooreBlockOptions& options,
    int support_half_width,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096,
    bool allow_fractional_center = false);

double connected_moore_block_binding_energy(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options = {});

ConnectedMooreBlockStepResult solve_connected_moore_block_forward(
    const ConnectedMooreBlockState& earlier,
    const ConnectedMooreBlockOptions& options = {},
    ConnectedMooreBlockSolveCache* cache = nullptr);

// Accelerator entry point.  magnetic_later and electric_pre_current must be
// the exact matched source-free update of earlier and must already have passed
// finite-value validation in the supplying backend.  Requires local residual
// evaluation and deferred volume diagnostics.
ConnectedMooreBlockStepResult solve_connected_moore_block_forward_prepared(
    const ConnectedMooreBlockState& earlier,
    MatchedEdgeField&& magnetic_later,
    MatchedFaceFlux&& electric_pre_current,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache = nullptr);

// FTD-0759 matter-only accelerator entry point.  The input carries L and
// constituent/charge/graph metadata; its dense field arrays must be empty.
// Canonical fields live in the backend captured by resident_local_orbit_gather.
// Requires sparse local residuals and deferred volume diagnostics.
ConnectedMooreBlockStepResult solve_connected_moore_block_forward_resident(
    const ConnectedMooreBlockState& matter_only_earlier,
    const ConnectedMooreBlockOptions& options,
    ConnectedMooreBlockSolveCache* cache = nullptr);

ConnectedMooreBlockStepResult complete_connected_moore_block_volume_diagnostics(
    ConnectedMooreBlockStepResult step,
    const ConnectedMooreBlockVolumeDiagnostics& diagnostics,
    const ConnectedMooreBlockOptions& options = {});

ConnectedMooreBlockStepResult solve_connected_moore_block_reverse(
    const ConnectedMooreBlockState& later,
    const ConnectedMooreBlockOptions& options = {},
    ConnectedMooreBlockSolveCache* cache = nullptr);

double connected_moore_block_state_max_difference(
    const ConnectedMooreBlockState& lhs,
    const ConnectedMooreBlockState& rhs);

}  // namespace ftd::eft

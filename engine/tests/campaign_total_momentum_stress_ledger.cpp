/**
 * Total momentum stress ledger campaign (Arc 2) on CUDA.
 *
 * Implements the required build of Sec 4 of
 * docs/theory/10_eft_program/preregistrations/
 * PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md.
 *
 * Parent, construction and evolution are inherited unchanged from the FTD-0768
 * campaign (Sec 5): the FTD-0761/0763 connected opposite-polarity parent
 * through tick 160, aged 128 further ticks, branched into a matched rest
 * (q=0) and moving (q=+0.030 along (0,0,1)) arm and evolved 768 ticks at
 * L=321, dt=0.25.  This file adds the momentum instrumentation and nothing
 * else: it does not touch production, RenderBridge, any scenario, or any
 * existing test.
 *
 * Runner modes:
 *   --exactness-precheck   Sec 6.4 L=11 exactness pre-check (writes nothing)
 *   --firewall             Sec 8 L=17 two-tick qualification (writes nothing)
 *   --run                  full campaign; writes results only after both pass
 */

#pragma push_macro("main")
#undef main
#define main ftd0768_reference_main
#include "campaign_long_transport_dynamic_response_cuda.cpp"
#undef main
#pragma pop_macro("main")

#include "ftd/eft/cuda_momentum_transport_current.h"
#include "ftd/eft/momentum_transport_current.h"

#include <cstdint>
#include <iostream>
#include <memory>

namespace {

using namespace ftd;
using namespace ftd::eft;

// ---------------------------------------------------------------------------
// Frozen protocol constants (Sec 5, Sec 6)
// ---------------------------------------------------------------------------

/// Sec 12 item 5: minted by scripts/audit/check_registry.py on scale1-revision
/// (762 ids referenced, max FTD-0768) after the instrumentation, the pre-check
/// and the firewall all passed.
constexpr char kMomentumFtdId[] = "FTD-0769";
constexpr char kMomentumResultSlug[] = "ftd_0769";
/// Sec 12 item 6: SHA-256 of the pre-registration byte-prefix preceding the
/// `protocol_sha256=` field (PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md,
/// header line 4), per REF_PREREGISTER_MANIFEST.md's convention. Independently
/// re-verified by scripts/proofs/proof_total_momentum_stress_ledger.py.
constexpr char kMomentumProtocolSha256[] =
    "215B03A85A76B706E91099CA24E276FAC3B57DE3852353981456F79F411D8A13";

constexpr int kMomentumVolume = 321;
constexpr int kMomentumFormation = 160;
constexpr int kMomentumAge = 128;
constexpr int kMomentumTicks = 768;
constexpr int kMomentumStride = 64;
constexpr int kMomentumCheckpoints = kMomentumTicks / kMomentumStride + 1;
constexpr double kMomentumBoost = 0.030;
constexpr int kMomentumSupportHalfWidth = 4;

/// Sec 5 radius scan; slot 0 is CLEARANCE_MARGINAL, slots 1..3 are the physics
/// radii, slot 4 is the escape gauge R_out.
constexpr int kMomentumRadii[kCudaMomentumMaximumRadii] = {8, 16, 24, 32, 48};
constexpr int kMomentumPhysicsSlots[3] = {1, 2, 3};
constexpr int kMomentumOuterSlot = 4;
constexpr int kMomentumMarginalSlot = 0;

// Sec 6 tolerances.  Sec 6.1 justifies the 1e-11 per-tick figure from the
// Sec 2.11 fixture closures; Banned move B3 forbids reusing FTD-0768's energy
// numbers, so none of these is inherited.
constexpr double kMomentumTickGate = 1e-11;
constexpr double kMomentumReynoldsGate = 1e-12;
constexpr double kMomentumComplementarityGate = 1e-11;
constexpr double kMomentumUnitGate = 1e-12;
constexpr double kMomentumUnitZeroGate = 1e-11;
constexpr double kMomentumKappaGate = 1e-13;
constexpr double kMomentumPrecheckGate = 1e-12;
// Sec 6.3: inherited FTD-0768 execution-validity gates, not loosened.
constexpr double kMomentumInheritedGate = 1e-12;
constexpr double kMomentumReverseGate = 1e-10;
constexpr double kMomentumMinimumSigma = 1e-3;
constexpr double kMomentumMaximumCondition = 1e4;
constexpr double kMomentumMinimumCoreMargin = 1e-6;
// Sec 6.6 bands, Sec 6.7 significance, Sec 6.8 rest-arm factor.
constexpr double kMomentumBandHalfWidth = 0.25;   // h
constexpr double kMomentumFlatness = 0.10;        // g
constexpr double kMomentumAccumulation = 0.25;    // G
constexpr double kMomentumSignificanceFloor = 1e-9;
constexpr double kMomentumArmSeparation = 1e3;
constexpr double kMomentumRestFactor = 0.1;
constexpr double kMomentumSignFloorFraction = 0.2;  // Sec 2.10 1/5 running max
// Sec 6.9 prior-comparability landmarks (reported, non-blocking).
constexpr double kMomentumPriorWholeChange = 2.0044e-3;
constexpr double kMomentumPriorDefect = 4.5931e-3;

// Sec 8 firewall.
constexpr int kMomentumFirewallVolume = 17;
constexpr int kMomentumFirewallTicks = 2;
constexpr int kMomentumFirewallProbeRadii[kCudaMomentumMaximumRadii] =
    {3, 5, -1, -1, -1};
/// Sec 6.4 pre-check lattice.
constexpr int kMomentumPrecheckVolume = 11;

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

double axis_value(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

/// Sec 5: c(tau) is the rounded lattice site, ties resolved toward +infinity.
int rounded_site(double value, int L) {
  const long long nearest =
      static_cast<long long>(std::floor(value + 0.5));
  long long wrapped = nearest % L;
  if (wrapped < 0) wrapped += L;
  return static_cast<int>(wrapped);
}

void rounded_center(const Vec3& center, int L, int out[3]) {
  out[0] = rounded_site(center.x, L);
  out[1] = rounded_site(center.y, L);
  out[2] = rounded_site(center.z, L);
}

int periodic_span(int value, int center, int L) {
  int delta = (value - center) % L;
  if (delta < 0) delta += L;
  if (delta > L / 2) delta -= L;
  return delta < 0 ? -delta : delta;
}

int localization_index(MomentumLocalization localization) {
  return localization == MomentumLocalization::ECarries ? 0 : 1;
}

const char* localization_name(int index) {
  return index == 0 ? "L1_E_carries" : "L2_B_carries";
}

const char* component_name(int component) {
  return component == 0 ? "x" : (component == 1 ? "y" : "z");
}

std::uint64_t splitmix64(std::uint64_t& state) {
  state += 0x9E3779B97F4A7C15ULL;
  std::uint64_t value = state;
  value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
  return value ^ (value >> 31);
}

double deterministic_uniform(std::uint64_t& state) {
  return static_cast<double>(splitmix64(state) >> 11)
      * (1.0 / 9007199254740992.0) * 2.0 - 1.0;
}

// ---------------------------------------------------------------------------
// Sec 6.4 exactness pre-check (L=11, both masks, both localizations)
// ---------------------------------------------------------------------------

struct ExactnessCheck {
  std::string name;
  double residual = INFINITY;
  double tolerance = kMomentumPrecheckGate;
  bool pass = false;
};

struct ChordCensusRecord {
  std::string operator_name;
  int component = 0;
  int displacement_count = 0;
  int entry_count = 0;
  int class_count = 0;
  int bond_generator_count = 0;
  int site_generator_count = 0;
  int maximum_l1 = 0;
  int maximum_linf = 0;
  double skewness_residual = 0.0;
};

struct ExactnessReport {
  bool pass = false;
  int L = kMomentumPrecheckVolume;
  double lambda = 0.0;
  double site_channel_under_site_mask = INFINITY;
  double site_channel_under_component_mask = 0.0;
  bool site_channel_exercised = false;
  std::vector<ChordCensusRecord> census;
  std::vector<ExactnessCheck> checks;

  void add(const std::string& name, double residual,
           double tolerance = kMomentumPrecheckGate) {
    ExactnessCheck check;
    check.name = name;
    check.residual = std::abs(residual);
    check.tolerance = tolerance;
    check.pass = std::isfinite(check.residual) && check.residual <= tolerance;
    checks.push_back(check);
  }
};

struct PrecheckFixture {
  int L = 0;
  double lambda = 0.0;
  MatchedFaceFlux electric_before;
  MatchedEdgeField magnetic_before;
  MatchedEdgeField magnetic_after;
  MatchedFaceFlux electric_pre_current;
  MatchedFaceFlux electric_after;
  explicit PrecheckFixture(int size)
      : L(size), electric_before(size), magnetic_before(size),
        magnetic_after(size), electric_pre_current(size),
        electric_after(size) {}
};

PrecheckFixture build_precheck_fixture(int L, double lambda,
                                       std::uint64_t seed) {
  PrecheckFixture fixture(L);
  fixture.lambda = lambda;
  std::uint64_t state = seed;
  const auto fill = [&](std::vector<double>& values) {
    for (auto& value : values) value = deterministic_uniform(state);
  };
  fill(fixture.electric_before.x);
  fill(fixture.electric_before.y);
  fill(fixture.electric_before.z);
  fill(fixture.magnetic_before.x);
  fill(fixture.magnetic_before.y);
  fill(fixture.magnetic_before.z);

  // B' = B-lambda C^T E
  fixture.magnetic_after = fixture.magnetic_before;
  const auto curl_electric = matched_curl_adjoint(fixture.electric_before);
  for (std::size_t i = 0; i < fixture.magnetic_after.x.size(); ++i) {
    fixture.magnetic_after.x[i] -= lambda * curl_electric.x[i];
    fixture.magnetic_after.y[i] -= lambda * curl_electric.y[i];
    fixture.magnetic_after.z[i] -= lambda * curl_electric.z[i];
  }
  // E' = E+lambda C B'
  fixture.electric_pre_current = fixture.electric_before;
  const auto curl_magnetic = matched_curl(fixture.magnetic_after);
  for (std::size_t i = 0; i < fixture.electric_pre_current.x.size(); ++i) {
    fixture.electric_pre_current.x[i] += lambda * curl_magnetic.x[i];
    fixture.electric_pre_current.y[i] += lambda * curl_magnetic.y[i];
    fixture.electric_pre_current.z[i] += lambda * curl_magnetic.z[i];
  }
  // E'' = E'-K with K compactly supported about the lattice centre.
  fixture.electric_after = fixture.electric_pre_current;
  const int centre = L / 2;
  for (int x = centre - 2; x <= centre + 2; ++x)
    for (int y = centre - 2; y <= centre + 2; ++y)
      for (int z = centre - 2; z <= centre + 2; ++z) {
        const auto index =
            static_cast<std::size_t>(fixture.electric_after.index(x, y, z));
        fixture.electric_after.x[index] -= 0.05 * deterministic_uniform(state);
        fixture.electric_after.y[index] -= 0.05 * deterministic_uniform(state);
        fixture.electric_after.z[index] -= 0.05 * deterministic_uniform(state);
      }
  return fixture;
}

double whole_domain_density_sum(
    const std::array<std::vector<double>, 3>& density) {
  long double total = 0.0L;
  for (int a = 0; a < 3; ++a)
    for (const double value : density[static_cast<std::size_t>(a)])
      total += value;
  return static_cast<double>(total);
}

ExactnessReport run_exactness_precheck() {
  ExactnessReport report;
  const int L = kMomentumPrecheckVolume;
  const auto action = forensic_options();
  const double lambda = action.wave_speed * action.dt;
  report.lambda = lambda;
  const auto fixture = build_precheck_fixture(L, lambda, 0x5EED0768ULL);

  MomentumStepFields fields;
  fields.electric_before = &fixture.electric_before;
  fields.magnetic_before = &fixture.magnetic_before;
  fields.magnetic_after = &fixture.magnetic_after;
  fields.electric_pre_current = &fixture.electric_pre_current;
  fields.electric_after = &fixture.electric_after;
  fields.lambda = lambda;

  const auto site_mask = make_momentum_site_mask(L, 5, 5, 5, 3);
  const auto component_mask =
      make_momentum_component_challenge_mask(L, 5, 5, 5, 2);
  const auto universal = make_momentum_universal_mask(L);
  const MomentumMask masks[2] = {site_mask, component_mask};
  const char* mask_names[2] = {"site_mask", "component_mask"};

  // K, u, w and the derived carriers used by the flux checks.
  MatchedFaceFlux current(L);
  for (std::size_t i = 0; i < current.x.size(); ++i) {
    current.x[i] = fixture.electric_pre_current.x[i]
        - fixture.electric_after.x[i];
    current.y[i] = fixture.electric_pre_current.y[i]
        - fixture.electric_after.y[i];
    current.z[i] = fixture.electric_pre_current.z[i]
        - fixture.electric_after.z[i];
  }
  const auto u_field = matched_curl(fixture.magnetic_after);          // u = C B'
  const auto w_field = matched_curl_adjoint(fixture.electric_before); // w = C^T E
  const auto adjoint_current = matched_curl_adjoint(current);         // C^T K

  double site_channel_site_mask = 0.0;
  double site_channel_component_mask = 0.0;

  for (int component = 0; component < 3; ++component) {
    const auto plain =
        build_momentum_transport_current_table(
            MomentumOperatorKind::CentralDifference, component, L);
    const auto face_binding = build_momentum_transport_current_table(
        MomentumOperatorKind::FaceBinding, component, L);
    const auto edge_binding = build_momentum_transport_current_table(
        MomentumOperatorKind::EdgeBinding, component, L);
    const MomentumTransportCurrentTable* tables[3] =
        {&plain, &face_binding, &edge_binding};
    const char* names[3] = {"D_i", "D_i_C_CT", "D_i_CT_C"};
    for (int t = 0; t < 3; ++t) {
      ChordCensusRecord record;
      record.operator_name = names[t];
      record.component = component;
      record.displacement_count = tables[t]->displacement_count;
      record.entry_count = tables[t]->entry_count;
      record.class_count = tables[t]->class_count;
      record.bond_generator_count = static_cast<int>(tables[t]->bond.size());
      record.site_generator_count = static_cast<int>(tables[t]->site.size());
      record.maximum_l1 = tables[t]->maximum_l1;
      record.maximum_linf = tables[t]->maximum_linf;
      record.skewness_residual = tables[t]->skewness_residual;
      report.census.push_back(record);
      report.add(std::string("skewness_") + names[t] + "_"
                 + component_name(component), tables[t]->skewness_residual,
                 1e-13);
    }

    // -- global check: the two densities agree on the whole domain ----------
    const auto before_l1 = momentum_density_before(
        fields, MomentumLocalization::ECarries, component);
    const auto before_l2 = momentum_density_before(
        fields, MomentumLocalization::BCarries, component);
    const auto after_l1 = momentum_density_after(
        fields, MomentumLocalization::ECarries, component);
    const auto after_l2 = momentum_density_after(
        fields, MomentumLocalization::BCarries, component);
    report.add(std::string("global_density_before_") + component_name(component),
               whole_domain_density_sum(before_l1)
                   - whole_domain_density_sum(before_l2));
    report.add(std::string("global_density_after_") + component_name(component),
               whole_domain_density_sum(after_l1)
                   - whole_domain_density_sum(after_l2));

    // -- source agreement: <B',D_i C^T K> + <K,D_i C B'> = 0 ----------------
    const auto drive_u = matched_central_derivative(u_field, component);
    long double source_l1 = 0.0L;
    long double source_l2 = 0.0L;
    {
      MatchedFaceFlux adjoint_carrier(L);
      adjoint_carrier.x = adjoint_current.x;
      adjoint_carrier.y = adjoint_current.y;
      adjoint_carrier.z = adjoint_current.z;
      const auto drive_ck =
          matched_central_derivative(adjoint_carrier, component);
      for (std::size_t i = 0; i < current.x.size(); ++i) {
        source_l1 += static_cast<long double>(current.x[i]) * drive_u.x[i]
            + static_cast<long double>(current.y[i]) * drive_u.y[i]
            + static_cast<long double>(current.z[i]) * drive_u.z[i];
        source_l2 +=
            static_cast<long double>(fixture.magnetic_after.x[i]) * drive_ck.x[i]
            + static_cast<long double>(fixture.magnetic_after.y[i])
                * drive_ck.y[i]
            + static_cast<long double>(fixture.magnetic_after.z[i])
                * drive_ck.z[i];
      }
    }
    report.add(std::string("source_agreement_") + component_name(component),
               static_cast<double>(source_l1 + source_l2));

    for (int mask_index = 0; mask_index < 2; ++mask_index) {
      const auto& mask = masks[mask_index];
      const std::string suffix =
          std::string("_") + mask_names[mask_index] + "_"
          + component_name(component);
      const auto complement = complement_momentum_mask(mask);

      struct FluxCase {
        const char* label;
        const MomentumTransportCurrentTable* table;
        MomentumFieldView field;
        MomentumOperatorKind kind;
      };
      const FluxCase cases[4] = {
          {"L1_phi_u", &plain, momentum_view(u_field),
           MomentumOperatorKind::CentralDifference},
          {"L1_phi_E", &face_binding, momentum_view(fixture.electric_before),
           MomentumOperatorKind::FaceBinding},
          {"L2_phi_w", &plain, momentum_view(w_field),
           MomentumOperatorKind::CentralDifference},
          {"L2_phi_Bprime", &edge_binding,
           momentum_view(fixture.magnetic_after),
           MomentumOperatorKind::EdgeBinding},
      };
      for (const auto& item : cases) {
        const double chord = masked_chord_flux(*item.table, item.field, mask);
        const double direct = direct_masked_bilinear(item.kind, component,
                                                     item.field, mask);
        report.add(std::string(item.label) + "_chord_vs_direct" + suffix,
                   chord - direct);
        const double complement_flux =
            masked_chord_flux(*item.table, item.field, complement);
        report.add(std::string(item.label) + "_complementarity" + suffix,
                   chord + complement_flux);
        const auto arrays =
            build_momentum_stress_ledger_arrays(*item.table, item.field);
        report.add(std::string(item.label) + "_arrays_vs_chord" + suffix,
                   masked_flux_from_arrays(arrays, mask) - chord);
        const double site_channel =
            masked_site_flux(*item.table, item.field, mask);
        if (mask_index == 0)
          site_channel_site_mask =
              std::max(site_channel_site_mask, std::abs(site_channel));
        else
          site_channel_component_mask =
              std::max(site_channel_component_mask, std::abs(site_channel));
      }

      // -- identity checks: residual of (M1) and (M2) ----------------------
      for (int localization = 0; localization < 2; ++localization) {
        const auto kind = localization == 0 ? MomentumLocalization::ECarries
                                            : MomentumLocalization::BCarries;
        const auto& binding = localization == 0 ? face_binding : edge_binding;
        const auto terms = observe_momentum_ledger_tick(
            fields, kind, component, mask, mask, plain, binding);
        report.add(std::string(localization == 0 ? "M1" : "M2")
                       + "_identity" + suffix,
                   terms.identity_residual(lambda));
        report.add(std::string(localization == 0 ? "M1" : "M2")
                       + "_reynolds" + suffix, terms.reynolds_residual());
      }
    }

    // -- whole-domain limit of the identities ------------------------------
    for (int localization = 0; localization < 2; ++localization) {
      const auto kind = localization == 0 ? MomentumLocalization::ECarries
                                          : MomentumLocalization::BCarries;
      const auto& binding = localization == 0 ? face_binding : edge_binding;
      const auto terms = observe_momentum_ledger_tick(
          fields, kind, component, universal, universal, plain, binding);
      report.add(std::string(localization == 0 ? "M1" : "M2")
                     + "_whole_domain_flux_zero_" + component_name(component),
                 terms.flux(lambda), kMomentumUnitZeroGate);
      report.add(std::string(localization == 0 ? "M1" : "M2")
                     + "_whole_domain_identity_" + component_name(component),
                 terms.identity_residual(lambda));
    }
  }

  report.site_channel_under_site_mask = site_channel_site_mask;
  report.site_channel_under_component_mask = site_channel_component_mask;
  // Sec 2.3: exactly zero under any site mask; Sec 6.4 demands a non-zero
  // per-component contribution so the S^(i) array is not untested dead code.
  report.site_channel_exercised = site_channel_component_mask > 0.0;
  report.add("site_channel_zero_under_site_mask", site_channel_site_mask, 0.0);

  report.pass = report.site_channel_exercised;
  for (const auto& check : report.checks)
    report.pass = report.pass && check.pass;
  return report;
}

// ---------------------------------------------------------------------------
// Per-arm accumulators and stepper
// ---------------------------------------------------------------------------

struct MomentumArmAccumulators {
  MomentumLedgerAccumulator ledger[2][3][kCudaMomentumSlots];
};

struct MomentumLedgerStep {
  bool valid = false;
  bool common = false;
  bool member = false;
  bool regularity_measured = false;
  bool inverse_valid = true;
  bool ledger_valid = false;
  bool centre_changed = false;
  int centre[3]{};
  int source_half_width = 0;
  Vec3 matter_before{}, matter_after{};
  Vec3 local_before{}, local_after{};
  double common_residual = INFINITY;
  double energy_residual = INFINITY;
  double speed_excess = INFINITY;
  double sigma_min = 0.0;
  double condition = INFINITY;
  double inverse_residual = INFINITY;
  double graph_margin = -INFINITY;
  double energy_margin = -INFINITY;
  double maximum_tick_identity_ratio = 0.0;
  double maximum_reynolds_ratio = 0.0;
  int site_hops = 0;
  CudaMomentumTransportTelemetry telemetry;
};

class MomentumLedgerCudaStepper {
 public:
  MomentumLedgerCudaStepper(ConnectedMooreBlockState initial,
                            ConnectedMooreBlockOptions options,
                            double interaction_scale)
      : state_(std::move(initial)), options_(std::move(options)),
        interaction_scale_(interaction_scale),
        pipeline_(state_.electric.L), ledger_(state_.electric.L),
        prepared_b_(state_.electric.L), prepared_e_(state_.electric.L) {
    const double c = static_cast<double>(state_.electric.L / 2);
    diagnostic_center_ = {c, c, c};
    rounded_center(object_center(state_), state_.electric.L, previous_center_);
    options_.defer_volume_diagnostics = true;
    for (int slot = 0; slot < kCudaMomentumMaximumRadii; ++slot)
      radius_[slot] = kMomentumRadii[slot];
    valid_ = pipeline_.valid() && ledger_.valid()
        && pipeline_.upload(state_.electric, state_.magnetic_half);
  }

  bool valid() const { return valid_; }
  const char* ledger_error() const { return ledger_.error(); }
  const ConnectedMooreBlockState& state() const { return state_; }
  ConnectedMooreBlockState release_state() { return std::move(state_); }
  const MomentumArmAccumulators& accumulators() const { return totals_; }
  MomentumArmAccumulators& accumulators() { return totals_; }
  const CudaMomentumTransportLedger& ledger() const { return ledger_; }
  void set_radius(const int radius[kCudaMomentumMaximumRadii]) {
    for (int slot = 0; slot < kCudaMomentumMaximumRadii; ++slot)
      radius_[slot] = radius[slot];
  }
  /// Sec 8 wiring probe only; never used by the registered campaign.
  void enable_host_parity(bool enabled) { host_parity_ = enabled; }
  double maximum_host_parity_residual() const {
    return maximum_host_parity_residual_;
  }
  /// Sec 8 wiring probe only; never used by the registered campaign (Sec 6.3
  /// G2 requires full physics-gate validity there, unconditionally).  The
  /// firewall's own scope (Sec 8) is "verify the runner schema, CUDA calls,
  /// masked-kernel interface, per-tick accumulator wiring" -- not physics
  /// validity of a deliberately minimal, zero-formation-tick L=17 probe
  /// state, which is not expected to satisfy the state-only matter
  /// membership check (Sec 6.3's `core.member`) or the common-action gates.
  /// When disabled, a membership/common-action miss on one tick no longer
  /// poisons the stepper for subsequent ticks; the ledger/ CUDA-kernel
  /// wiring check (`result.ledger_valid`, gated earlier in `advance()`
  /// unconditionally) is untouched by this flag either way.
  void set_require_physics_gates(bool enabled) {
    require_physics_gates_ = enabled;
  }

  MomentumLedgerStep advance(bool checkpoint) {
    MomentumLedgerStep result;
    if (!valid_) return result;
    options_.measure_final_root_regularity = checkpoint;
    const double lambda = options_.wave_speed * options_.dt;
    if (!pipeline_.prepare_forward(lambda)
        || !pipeline_.download_prepared(prepared_b_, prepared_e_)) {
      valid_ = false;
      return result;
    }
    auto step = solve_connected_moore_block_forward_prepared(
        state_, std::move(prepared_b_), std::move(prepared_e_), options_,
        &forward_cache_);
    if (!step.volume_diagnostics_pending
        || !pipeline_.apply_ordered_sparse_current(
            step.segments, options_.polarity_scale)) {
      valid_ = false;
      return result;
    }

    const int L = state_.electric.L;
    int current_center[3]{};
    rounded_center(object_center(step.later), L, current_center);
    for (int axis = 0; axis < 3; ++axis) {
      result.centre[axis] = current_center[axis];
      if (current_center[axis] != previous_center_[axis])
        result.centre_changed = true;
    }
    result.source_half_width = measure_source_half_width(step, current_center,
                                                         L);

    CudaMomentumLedgerOptions ledger_options;
    ledger_options.lambda = lambda;
    ledger_options.interaction_scale = interaction_scale_;
    for (int axis = 0; axis < 3; ++axis) {
      ledger_options.previous_center[axis] = previous_center_[axis];
      ledger_options.current_center[axis] = current_center[axis];
    }
    for (int slot = 0; slot < kCudaMomentumMaximumRadii; ++slot)
      ledger_options.radius[slot] = radius_[slot];

    const auto views = pipeline_.resident_views();
    const auto tick = ledger_.observe(views, ledger_options, &result.telemetry);
    result.ledger_valid = tick.valid && result.telemetry.valid
        && result.telemetry.complete_field_downloads == 0;
    if (result.ledger_valid) {
      for (int localization = 0; localization < 2; ++localization)
        for (int component = 0; component < 3; ++component)
          for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
            const auto& terms = tick.terms[localization][component][slot];
            if (!terms.valid) continue;
            auto& accumulator = totals_.ledger[localization][component][slot];
            accumulator.add(terms, lambda);
            result.maximum_tick_identity_ratio = std::max(
                result.maximum_tick_identity_ratio,
                std::abs(terms.identity_residual(lambda))
                    / terms.identity_scale(lambda));
            result.maximum_reynolds_ratio = std::max(
                result.maximum_reynolds_ratio,
                std::abs(terms.reynolds_residual()) / terms.reynolds_scale());
          }
      if (host_parity_)
        measure_host_parity(tick, lambda, current_center, L);
    }

    const auto profile = pipeline_.observe_deterministic(
        lambda, diagnostic_center_, {8}, kMomentumInheritedGate);
    if (!profile.valid || !result.ledger_valid) {
      valid_ = false;
      return result;
    }
    const auto diagnostics = pipeline_.diagnose_common_action(
        step.segments, options_.polarity_scale, interaction_scale_,
        options_.wave_speed, options_.dt, kMomentumInheritedGate);
    step = complete_connected_moore_block_volume_diagnostics(
        std::move(step), diagnostics, options_);

    result.valid = step.valid && step.common_action_gates_pass;
    result.common = step.common_action_gates_pass;
    result.matter_before = step.matter_momentum_before;
    result.matter_after = step.matter_momentum_after;
    result.local_before = step.local_field_momentum_before;
    result.local_after = step.local_field_momentum_after;
    result.common_residual = common_residual_0764(step);
    result.energy_residual = std::abs(step.total_energy_residual);
    result.speed_excess = step.causal_speed_excess;
    result.regularity_measured = step.solve.final_root_regularity_measured;
    result.sigma_min = step.solve.final_minimum_singular_value;
    result.condition = step.solve.final_condition_number;
    result.site_hops = step.site_hops;
    const auto core = observe_support_invariant_matter(step.later, options_);
    result.member = core.valid && core.member;
    result.graph_margin = core.graph_margin;
    result.energy_margin = core.energy_margin;
    if (checkpoint && result.valid) {
      auto reverse_options = options_;
      reverse_options.defer_volume_diagnostics = false;
      reverse_options.measure_final_root_regularity = false;
      ConnectedMooreBlockSolveCache reverse_cache;
      const auto reverse = solve_connected_moore_block_reverse(
          step.later, reverse_options, &reverse_cache);
      result.inverse_valid = reverse.valid && reverse.common_action_gates_pass
          && discrete_state_equal(state_, reverse.earlier);
      result.inverse_residual = result.inverse_valid
          ? connected_moore_block_state_max_difference(state_, reverse.earlier)
          : INFINITY;
    } else {
      result.inverse_valid = true;
      result.inverse_residual = 0.0;
    }
    state_ = std::move(step.later);
    for (int axis = 0; axis < 3; ++axis)
      previous_center_[axis] = current_center[axis];
    if (!pipeline_.advance()) valid_ = false;
    valid_ = valid_
        && (!require_physics_gates_ || (result.valid && result.member));
    return result;
  }

 private:
  static int measure_source_half_width(
      const ConnectedMooreBlockStepResult& step, const int center[3], int L) {
    int span = 0;
    for (const auto& segment : step.segments)
      for (const auto& entry : segment.sparse_current) {
        if (entry.value == 0.0) continue;
        span = std::max({span, periodic_span(entry.face.x, center[0], L),
                         periodic_span(entry.face.y, center[1], L),
                         periodic_span(entry.face.z, center[2], L)});
      }
    return span;
  }

  /// Sec 8 wiring probe: recompute one tick on the host and compare.  The
  /// host route derives K first and then C^T K, the device route differences
  /// C^T E' and C^T E''; the two agree only to floating-point rounding.
  void measure_host_parity(const CudaMomentumLedgerTick& tick, double lambda,
                           const int current_center[3], int L) {
    if (host_electric_before_.L != L || host_magnetic_before_.L != L) return;
    MatchedFaceFlux electric_after(L);
    MatchedEdgeField magnetic_after(L);
    // Firewall wiring probe only: this is the one place a complete field
    // leaves the device, and it never runs in the registered campaign.
    if (!pipeline_.download_after(electric_after, magnetic_after)) return;
    const auto& electric_before = host_electric_before_;
    const auto& magnetic_before = host_magnetic_before_;
    // B' comes from the device; E' is re-derived from the device-exact B',
    // so the two routes differ only by floating-point rounding.
    MatchedFaceFlux electric_pre = electric_before;
    const auto curl_b = matched_curl(magnetic_after);
    for (std::size_t i = 0; i < electric_pre.x.size(); ++i) {
      electric_pre.x[i] += lambda * curl_b.x[i];
      electric_pre.y[i] += lambda * curl_b.y[i];
      electric_pre.z[i] += lambda * curl_b.z[i];
    }
    MomentumStepFields fields;
    fields.electric_before = &electric_before;
    fields.magnetic_before = &magnetic_before;
    fields.magnetic_after = &magnetic_after;
    fields.electric_pre_current = &electric_pre;
    fields.electric_after = &electric_after;
    fields.lambda = lambda;
    for (int localization = 0; localization < 2; ++localization) {
      const auto kind = localization == 0 ? MomentumLocalization::ECarries
                                          : MomentumLocalization::BCarries;
      for (int component = 0; component < 3; ++component) {
        const auto& plain = ledger_.plain_table(component);
        const auto& binding = ledger_.binding_table(kind, component);
        for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
          if (slot < kCudaMomentumMaximumRadii && radius_[slot] < 0) continue;
          const auto previous = slot == kCudaMomentumWholeDomainSlot
              ? make_momentum_universal_mask(L)
              : make_momentum_site_mask(L, previous_center_[0],
                                        previous_center_[1],
                                        previous_center_[2], radius_[slot]);
          const auto current = slot == kCudaMomentumWholeDomainSlot
              ? make_momentum_universal_mask(L)
              : make_momentum_site_mask(L, current_center[0],
                                        current_center[1], current_center[2],
                                        radius_[slot]);
          auto host = observe_momentum_ledger_tick(
              fields, kind, component, previous, current, plain, binding);
          scale_momentum_ledger_tick_terms(host, interaction_scale_);
          const auto& device = tick.terms[localization][component][slot];
          const double scale = std::max({1.0, std::abs(host.content_after),
                                         std::abs(host.content_before),
                                         std::abs(host.phi_binding)});
          const double residual = std::max({
              std::abs(host.phi_plain - device.phi_plain),
              std::abs(host.phi_binding - device.phi_binding),
              std::abs(host.sweep - device.sweep),
              std::abs(host.source - device.source),
              std::abs(host.content_after - device.content_after),
              std::abs(host.content_before - device.content_before),
              std::abs(host.content_old - device.content_old)}) / scale;
          maximum_host_parity_residual_ =
              std::max(maximum_host_parity_residual_, residual);
        }
      }
    }
  }

 public:
  /// Sec 8 wiring probe support: keep a host mirror of the before-state.
  void capture_host_before_state() {
    host_electric_before_ = state_.electric;
    host_magnetic_before_ = state_.magnetic_half;
  }

 private:
  ConnectedMooreBlockState state_;
  ConnectedMooreBlockOptions options_;
  double interaction_scale_ = 0.0;
  Vec3 diagnostic_center_{};
  CudaMatchedFieldPipeline pipeline_;
  CudaMomentumTransportLedger ledger_;
  MatchedEdgeField prepared_b_;
  MatchedFaceFlux prepared_e_;
  ConnectedMooreBlockSolveCache forward_cache_;
  MomentumArmAccumulators totals_;
  int previous_center_[3]{};
  int radius_[kCudaMomentumMaximumRadii]{};
  bool host_parity_ = false;
  double maximum_host_parity_residual_ = 0.0;
  MatchedFaceFlux host_electric_before_;
  MatchedEdgeField host_magnetic_before_;
  bool require_physics_gates_ = true;
  bool valid_ = false;
};

// ---------------------------------------------------------------------------
// Checkpoint records (Sec 4 per-checkpoint payload)
// ---------------------------------------------------------------------------

struct MomentumRegionRecord {
  int radius = 0;
  bool used = false;
  bool clearance_marginal = false;
  double content = 0.0;
  double initial_content = 0.0;
  double content_change = 0.0;
  double flux = 0.0;
  double sweep = 0.0;
  double source = 0.0;
  double flux_complement = 0.0;
  double sweep_complement = 0.0;
  double flux_quadrature = 0.0;
  double sweep_quadrature = 0.0;
  double ledger_residual = 0.0;
  double window_maximum_tick_residual = 0.0;
  double maximum_chain_residual = 0.0;
  double eta = 0.0;
  double transfer = 0.0;
  double rho = 0.0;
  double rho_ceiling = 0.0;
  double kappa = 0.0;
  double retention_identity_residual = 0.0;
  bool gate_identity = false;
  bool gate_complementarity = false;
  bool gate_enclosure = false;
};

struct MomentumComponentRecord {
  int component = 0;
  MomentumRegionRecord region[kCudaMomentumSlots];
};

struct MomentumArmCheckpoint {
  int tau = 0;
  Vec3 center{};
  int mask_center[3]{};
  double displacement = 0.0;
  std::string state_hash;
  Vec3 matter_momentum{};
  Vec3 local_field_momentum{};
  Vec3 matter_change{};
  Vec3 local_change{};
  Vec3 defect{};
  int source_half_width = 0;
  MomentumComponentRecord localization[2][3];
  double maximum_unit_residual = 0.0;
  bool units_pass = false;
  bool inherited_pass = false;
};

struct MomentumCheckpoint {
  int tau = 0;
  bool valid = false;
  MomentumArmCheckpoint rest;
  MomentumArmCheckpoint moving;
};

struct MomentumArmRunningGates {
  bool inherited = true;
  double maximum_common_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_speed_excess = 0.0;
  double minimum_sigma = INFINITY;
  double maximum_condition = 0.0;
  double maximum_inverse_residual = 0.0;
  double minimum_graph_margin = INFINITY;
  double minimum_energy_margin = INFINITY;
  double maximum_tick_identity_ratio = 0.0;
  double maximum_reynolds_ratio = 0.0;
  int site_hops = 0;
  int centre_changes = 0;
  int maximum_source_half_width = 0;
  std::vector<std::array<int, 4>> sweep_events;  // {tick, cx, cy, cz}
};

void accumulate_gates(MomentumArmRunningGates& gates,
                      const MomentumLedgerStep& step, int tick) {
  gates.maximum_common_residual =
      std::max(gates.maximum_common_residual, step.common_residual);
  gates.maximum_energy_residual =
      std::max(gates.maximum_energy_residual, step.energy_residual);
  gates.maximum_speed_excess =
      std::max(gates.maximum_speed_excess, step.speed_excess);
  gates.minimum_graph_margin =
      std::min(gates.minimum_graph_margin, step.graph_margin);
  gates.minimum_energy_margin =
      std::min(gates.minimum_energy_margin, step.energy_margin);
  gates.maximum_tick_identity_ratio = std::max(
      gates.maximum_tick_identity_ratio, step.maximum_tick_identity_ratio);
  gates.maximum_reynolds_ratio =
      std::max(gates.maximum_reynolds_ratio, step.maximum_reynolds_ratio);
  gates.maximum_source_half_width =
      std::max(gates.maximum_source_half_width, step.source_half_width);
  gates.site_hops += step.site_hops;
  if (step.regularity_measured) {
    gates.minimum_sigma = std::min(gates.minimum_sigma, step.sigma_min);
    gates.maximum_condition = std::max(gates.maximum_condition, step.condition);
  }
  gates.maximum_inverse_residual =
      std::max(gates.maximum_inverse_residual, step.inverse_residual);
  if (step.centre_changed) {
    ++gates.centre_changes;
    gates.sweep_events.push_back({tick, step.centre[0], step.centre[1],
                                  step.centre[2]});
  }
  gates.inherited = gates.inherited && step.valid && step.common && step.member
      && step.ledger_valid
      && step.common_residual <= kMomentumInheritedGate
      && step.energy_residual <= kMomentumInheritedGate
      && step.speed_excess <= kMomentumInheritedGate
      && step.graph_margin >= kMomentumMinimumCoreMargin
      && step.energy_margin >= kMomentumMinimumCoreMargin
      && step.maximum_tick_identity_ratio <= kMomentumTickGate
      && step.maximum_reynolds_ratio <= kMomentumReynoldsGate
      && (!step.regularity_measured
          || (step.sigma_min >= kMomentumMinimumSigma
              && step.condition <= kMomentumMaximumCondition
              && step.inverse_valid
              && step.inverse_residual <= kMomentumInheritedGate));
}

MomentumArmCheckpoint make_arm_checkpoint(
    int tau, const ConnectedMooreBlockState& state,
    MomentumLedgerCudaStepper& stepper, const MomentumArmRunningGates& gates,
    const Vec3& laboratory_center, const Vec3& direction,
    const Vec3& initial_matter, const Vec3& initial_local,
    const Vec3& local_now) {
  MomentumArmCheckpoint result;
  result.tau = tau;
  result.center = object_center(state);
  rounded_center(result.center, state.electric.L, result.mask_center);
  result.displacement = (result.center - laboratory_center).dot(direction);
  result.state_hash = state_hash(state);
  result.matter_momentum = matter_momentum(state);
  result.local_field_momentum = local_now;
  result.matter_change = result.matter_momentum - initial_matter;
  result.local_change = result.local_field_momentum - initial_local;
  result.defect = result.matter_change + result.local_change;
  result.source_half_width = gates.maximum_source_half_width;
  result.inherited_pass = gates.inherited;

  const auto& totals = stepper.accumulators();
  double maximum_unit = 0.0;
  bool units_pass = true;
  for (int localization = 0; localization < 2; ++localization)
    for (int component = 0; component < 3; ++component) {
      auto& record = result.localization[localization][component];
      record.component = component;
      const auto& whole =
          totals.ledger[localization][component][kCudaMomentumWholeDomainSlot];
      const double whole_change = whole.content_change();
      const double outer_source =
          totals.ledger[localization][component][kMomentumOuterSlot]
              .source_total();
      const double defect = axis_value(result.defect, component);
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
        const auto& accumulator = totals.ledger[localization][component][slot];
        auto& region = record.region[slot];
        region.used = accumulator.initialized;
        region.radius = slot == kCudaMomentumWholeDomainSlot
            ? -1 : kMomentumRadii[slot];
        region.clearance_marginal = slot == kMomentumMarginalSlot;
        region.content = accumulator.content;
        region.initial_content = accumulator.initial_content;
        region.content_change = accumulator.content_change();
        region.flux = accumulator.flux_total();
        region.sweep = accumulator.sweep_total();
        region.source = accumulator.source_total();
        region.flux_complement = accumulator.flux_complement_total();
        region.sweep_complement = accumulator.sweep_complement_total();
        region.flux_quadrature = region.flux + region.flux_complement;
        region.sweep_quadrature = region.sweep + region.sweep_complement;
        region.ledger_residual = accumulator.ledger_residual();
        region.window_maximum_tick_residual =
            accumulator.window_maximum_tick_identity_residual;
        region.maximum_chain_residual = accumulator.maximum_chain_residual;
        const auto ratios = compute_momentum_retention_ratios(
            accumulator, outer_source, whole_change, defect);
        region.eta = ratios.eta;
        region.transfer = ratios.transfer;
        region.rho = ratios.rho;
        region.rho_ceiling = ratios.rho_ceiling;
        region.kappa = ratios.kappa;
        region.retention_identity_residual = ratios.identity_residual;
        const double cumulative_gate = kMomentumTickGate
            * static_cast<double>(std::max<long long>(accumulator.ticks, 1))
            * accumulator.ledger_scale();
        region.gate_identity =
            std::abs(region.ledger_residual) <= cumulative_gate;
        region.gate_complementarity = std::abs(region.flux_quadrature)
            <= kMomentumComplementarityGate
                * std::max(1.0, std::abs(region.flux));
        region.gate_enclosure = slot == kCudaMomentumWholeDomainSlot
            || std::abs(region.kappa - 1.0) <= kMomentumKappaGate;
      }
      // Sec 3 G_U: whole-domain reconciliation against the already-measured
      // reference columns, computed by this run for itself.
      const auto& reference = whole;
      const double measured = axis_value(result.local_field_momentum, component);
      const double measured_change = axis_value(result.local_change, component);
      const double unit_content = std::abs(reference.content - measured)
          / std::max(1.0, std::abs(measured));
      const double unit_change =
          std::abs(reference.content_change() - measured_change)
          / std::max(1.0, std::abs(measured_change));
      const double unit_flux = std::abs(reference.flux_total());
      const double unit_sweep = std::abs(reference.sweep_total());
      const double unit_source =
          std::abs(reference.source_total() + measured_change);
      maximum_unit = std::max({maximum_unit, unit_content, unit_change,
                               unit_flux, unit_sweep, unit_source});
      units_pass = units_pass && unit_content <= kMomentumUnitGate
          && unit_change <= kMomentumUnitGate
          && unit_flux <= kMomentumUnitZeroGate
          && unit_sweep == 0.0
          && unit_source <= kMomentumUnitZeroGate;
    }
  result.maximum_unit_residual = maximum_unit;
  result.units_pass = units_pass;
  for (int localization = 0; localization < 2; ++localization)
    for (int component = 0; component < 3; ++component)
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot)
        stepper.accumulators().ledger[localization][component][slot]
            .begin_checkpoint_window();
  return result;
}

// ---------------------------------------------------------------------------
// Verdict map (Sec 7)
// ---------------------------------------------------------------------------

struct MomentumComponentVerdict {
  int component = 0;
  bool significant = false;
  bool rest_arm_clean = true;
  bool instrument_limited = false;
  std::string bucket[2];       // per localization
  std::string verdict = "MOMENTUM_LEDGER_MIXED";
  bool exchange_sign_inverted = false;
  int qualifying_checkpoints = 0;
};

std::string classify_localization(const std::vector<MomentumCheckpoint>& record,
                                  int localization, int component,
                                  const std::vector<int>& qualifying) {
  bool core_retained = !qualifying.empty();
  bool through_flowing = !qualifying.empty();
  bool near_zone = !qualifying.empty();
  bool over_depleting = !qualifying.empty();
  for (const int index : qualifying) {
    const auto& arm = record[static_cast<std::size_t>(index)].moving;
    const auto& item = arm.localization[localization][component];
    double minimum = INFINITY;
    double maximum = -INFINITY;
    bool retained = true;
    bool flowing = true;
    bool depleting_high = true;
    bool depleting_low = true;
    for (const int slot : kMomentumPhysicsSlots) {
      const double eta = item.region[slot].eta;
      minimum = std::min(minimum, eta);
      maximum = std::max(maximum, eta);
      retained = retained && std::abs(eta - 1.0) <= kMomentumBandHalfWidth;
      flowing = flowing && std::abs(eta) <= kMomentumBandHalfWidth;
      depleting_high = depleting_high && eta >= 1.0 + kMomentumBandHalfWidth;
      depleting_low = depleting_low && eta <= -kMomentumBandHalfWidth;
    }
    const bool flat = (maximum - minimum) <= kMomentumFlatness;
    core_retained = core_retained && retained && flat;
    through_flowing = through_flowing && flowing && flat;
    near_zone = near_zone
        && (item.region[kMomentumPhysicsSlots[2]].eta
            - item.region[kMomentumPhysicsSlots[0]].eta)
               >= kMomentumAccumulation;
    over_depleting = over_depleting && (depleting_high || depleting_low);
  }
  if (core_retained) return "CORE_RETAINED";
  if (through_flowing) return "THROUGH_FLOWING";
  if (near_zone) return "NEAR_ZONE_ACCUMULATING";
  if (over_depleting) return "OVER_DEPLETING";
  return "MIXED";
}

// ---------------------------------------------------------------------------
// Campaign result
// ---------------------------------------------------------------------------

struct MomentumCampaign {
  bool precheck_pass = false;
  bool firewall_pass = false;
  bool parent_valid = false;
  bool aging_valid = false;
  bool rest_initialized = false;
  bool moving_initialized = false;
  bool forward_valid = false;
  bool boundary_clear = true;
  bool reverse_valid = false;
  bool reverse_discrete_exact = false;
  double reverse_recovery = INFINITY;
  double reverse_maximum_common = 0.0;
  int reverse_steps = 0;
  double boundary_margin = -INFINITY;
  double maximum_rest_displacement = 0.0;
  double interaction_scale = 0.0;
  double lambda = 0.0;
  Vec3 laboratory_center{};
  std::string moving_initial_hash;
  std::string moving_forward_final_hash;
  std::string moving_reversed_hash;
  MomentumArmRunningGates rest_gates;
  MomentumArmRunningGates moving_gates;
  std::vector<MomentumCheckpoint> checkpoints;
  MomentumComponentVerdict verdict[3];
  bool prior_mismatch = false;
  bool far_field_active = false;
  bool clearance_marginal = true;
  std::string outcome = "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED";
  ExactnessReport precheck;
};

/// Sec 2.10 magnitude floor: a checkpoint qualifies for the sign test only if
/// both magnitudes are at least one fifth of their own running maxima.
bool sign_floor_satisfied(const std::vector<MomentumCheckpoint>& record,
                          std::size_t index, int component) {
  double matter_running = 0.0;
  double source_running = 0.0;
  for (std::size_t i = 0; i <= index; ++i) {
    matter_running = std::max(matter_running,
        std::abs(axis_value(record[i].moving.matter_change, component)));
    source_running = std::max(source_running,
        std::abs(record[i].moving
                     .localization[0][component].region[kMomentumOuterSlot]
                     .source));
  }
  const double matter =
      std::abs(axis_value(record[index].moving.matter_change, component));
  const double source =
      std::abs(record[index].moving
                   .localization[0][component].region[kMomentumOuterSlot]
                   .source);
  return matter >= kMomentumSignFloorFraction * matter_running
      && source >= kMomentumSignFloorFraction * source_running;
}

void classify_campaign(MomentumCampaign& result) {
  const bool infrastructure = result.precheck_pass && result.firewall_pass
      && result.rest_gates.maximum_tick_identity_ratio <= kMomentumTickGate
      && result.moving_gates.maximum_tick_identity_ratio <= kMomentumTickGate
      && result.rest_gates.maximum_reynolds_ratio <= kMomentumReynoldsGate
      && result.moving_gates.maximum_reynolds_ratio <= kMomentumReynoldsGate
      && result.checkpoints.size()
          == static_cast<std::size_t>(kMomentumCheckpoints)
      && std::all_of(result.checkpoints.begin(), result.checkpoints.end(),
                     [](const MomentumCheckpoint& value) {
                       return value.valid && value.moving.units_pass
                           && value.rest.units_pass;
                     });
  const bool baseline = result.parent_valid && result.aging_valid
      && result.rest_initialized && result.moving_initialized
      && result.forward_valid && result.boundary_clear && result.reverse_valid
      && result.reverse_discrete_exact
      && result.reverse_recovery <= kMomentumReverseGate
      && result.reverse_steps == kMomentumTicks
      && result.maximum_rest_displacement <= kMomentumInheritedGate
      && result.rest_gates.inherited && result.moving_gates.inherited;

  if (!infrastructure) {
    result.outcome = "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED";
    for (auto& verdict : result.verdict)
      verdict.verdict = result.outcome;
    return;
  }
  if (!baseline) {
    result.outcome = "MOMENTUM_LEDGER_BASELINE_INVALID";
    for (auto& verdict : result.verdict)
      verdict.verdict = result.outcome;
    return;
  }

  // Sec 6.9 G8 prior-comparability flag and Sec 6.3 far-field flag.
  if (!result.checkpoints.empty()) {
    const auto& last = result.checkpoints.back();
    const double whole_change = std::abs(axis_value(last.moving.local_change, 2));
    const double defect = std::abs(axis_value(last.moving.defect, 2));
    result.prior_mismatch =
        whole_change < kMomentumPriorWholeChange / 3.0
        || whole_change > kMomentumPriorWholeChange * 3.0
        || defect < kMomentumPriorDefect / 3.0
        || defect > kMomentumPriorDefect * 3.0;
    for (int component = 0; component < 3; ++component)
      for (int localization = 0; localization < 2; ++localization) {
        const auto& outer = last.moving.localization[localization][component]
                                .region[kMomentumOuterSlot];
        if (std::abs(outer.eta - 1.0) > kMomentumBandHalfWidth
            || std::abs(outer.transfer) > kMomentumBandHalfWidth)
          result.far_field_active = true;
      }
  }

  for (int component = 0; component < 3; ++component) {
    auto& verdict = result.verdict[component];
    verdict.component = component;

    // Sec 6.5 G4 radius adequacy.  tau = 0 carries no accumulated tick, so the
    // enclosure and cumulative-identity statements start at the first readout.
    bool enclosure = true;
    for (const auto& checkpoint : result.checkpoints) {
      if (checkpoint.tau == 0) continue;
      for (int localization = 0; localization < 2; ++localization)
        for (const int slot : kMomentumPhysicsSlots)
          enclosure = enclosure
              && checkpoint.moving.localization[localization][component]
                     .region[slot].gate_enclosure
              && checkpoint.moving.localization[localization][component]
                     .region[slot].gate_identity;
    }
    const int source_half_width = std::max(
        result.moving_gates.maximum_source_half_width,
        result.rest_gates.maximum_source_half_width);
    for (const int slot : kMomentumPhysicsSlots)
      enclosure = enclosure
          && (kMomentumRadii[slot] - 1) > (source_half_width + 2);
    verdict.instrument_limited = !enclosure;

    // Sec 6.7 G6 significance.
    std::vector<int> qualifying;
    for (std::size_t index = 0; index < result.checkpoints.size(); ++index) {
      const auto& checkpoint = result.checkpoints[index];
      const double moving_change =
          std::abs(axis_value(checkpoint.moving.local_change, component));
      const double rest_change =
          std::abs(axis_value(checkpoint.rest.local_change, component));
      const double defect =
          std::abs(axis_value(checkpoint.moving.defect, component));
      if (moving_change >= kMomentumSignificanceFloor
          && defect >= kMomentumSignificanceFloor
          && moving_change >= kMomentumArmSeparation * rest_change)
        qualifying.push_back(static_cast<int>(index));
    }
    verdict.qualifying_checkpoints = static_cast<int>(qualifying.size());
    verdict.significant = !qualifying.empty();

    // Sec 6.8 G7 rest-arm control.
    bool rest_clean = true;
    for (const int index : qualifying) {
      const auto& checkpoint = result.checkpoints[static_cast<std::size_t>(index)];
      const double moving_whole =
          std::abs(axis_value(checkpoint.moving.local_change, component));
      for (int localization = 0; localization < 2; ++localization)
        for (const int slot : kMomentumPhysicsSlots) {
          const auto& rest =
              checkpoint.rest.localization[localization][component].region[slot];
          const auto& moving =
              checkpoint.moving.localization[localization][component]
                  .region[slot];
          const double rest_transfer = std::abs(rest.flux + rest.sweep);
          const double moving_transfer = std::abs(moving.flux + moving.sweep);
          rest_clean = rest_clean
              && rest_transfer <= kMomentumRestFactor * moving_transfer
              && std::abs(rest.content_change)
                  <= kMomentumRestFactor * moving_whole;
        }
    }
    verdict.rest_arm_clean = rest_clean;

    // Sec 2.10 / Sec 7 orthogonal exchange-sign flag.
    // tau = 0 carries no change and no accumulated source, so it can neither
    // pass nor fail a sign test; the Sec 2.10 window starts at the first
    // readout.
    bool inverted = !qualifying.empty();
    for (std::size_t index = 1; index < result.checkpoints.size(); ++index) {
      if (!sign_floor_satisfied(result.checkpoints, index, component)) continue;
      const double matter = axis_value(
          result.checkpoints[index].moving.matter_change, component);
      const double source = result.checkpoints[index].moving
                                .localization[0][component]
                                .region[kMomentumOuterSlot].source;
      if (matter == 0.0 || source == 0.0) { inverted = false; break; }
      if ((matter > 0.0) == (source > 0.0)) { inverted = false; break; }
    }
    verdict.exchange_sign_inverted = inverted;

    if (verdict.instrument_limited) {
      verdict.verdict = "MOMENTUM_LEDGER_INSTRUMENT_LIMITED";
      continue;
    }
    if (!verdict.significant) {
      verdict.verdict = "MOMENTUM_LEDGER_NULL_NO_DEFECT";
      continue;
    }
    if (!verdict.rest_arm_clean) {
      verdict.verdict = "MOMENTUM_LEDGER_REST_ARM_CONTAMINATED";
      continue;
    }
    verdict.bucket[0] =
        classify_localization(result.checkpoints, 0, component, qualifying);
    verdict.bucket[1] =
        classify_localization(result.checkpoints, 1, component, qualifying);
    if (verdict.bucket[0] != verdict.bucket[1]) {
      verdict.verdict = "MOMENTUM_LEDGER_LOCALIZATION_AMBIGUOUS";
      continue;
    }
    verdict.verdict = "MOMENTUM_LEDGER_" + verdict.bucket[0];
  }
  // Sec 7 requires the triple (x,y,z) to be reported with per-axis
  // disagreement explicit and forbids a majority-vote collapse.  The headline
  // is therefore the z axis -- the only axis Sec 6.7 expects to qualify -- and
  // every axis verdict is written out separately.
  result.outcome = result.verdict[2].verdict;
}

// ---------------------------------------------------------------------------
// Sec 8 firewall (L=17, two ticks, non-evidential, writes nothing)
// ---------------------------------------------------------------------------

struct FirewallReport {
  bool pass = false;
  bool ledger_available = false;
  bool state_prepared = false;
  bool scalar_only = true;
  bool radius_degenerate = false;
  bool probe_exercised_bonds = false;
  int ticks = 0;
  double maximum_tick_identity_ratio = 0.0;
  double maximum_reynolds_ratio = 0.0;
  double maximum_complementarity = 0.0;
  double maximum_host_parity_residual = 0.0;
  double maximum_probe_host_parity_residual = 0.0;
  bool results_absent = true;
  std::string error;
};

std::filesystem::path momentum_results_directory() {
  return std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / kMomentumResultSlug;
}

FirewallReport run_firewall() {
  FirewallReport report;
  report.ledger_available = cuda_momentum_transport_ledger_available();
  if (!report.ledger_available) {
    report.error = "no CUDA device";
    return report;
  }
  ForensicDirection direction_record;
  if (!select_direction("face", direction_record)) {
    report.error = "direction unavailable";
    return report;
  }
  const auto normalization = measure_face_flux_normalization();
  if (!normalization.valid) {
    report.error = "normalization unavailable";
    return report;
  }
  auto action = forensic_options();
  const int L = kMomentumFirewallVolume;
  auto preparation = prepare_finite_support_derived_compact_pair(
      make_parent_geometry(L, direction_record), action,
      kMomentumSupportHalfWidth, 1e-13, 4096);
  report.state_prepared = preparation.valid && preparation.density_contained
      && preparation.compact_support && preparation.zero_boundary_crossing;
  if (!report.state_prepared) {
    report.error = "firewall state preparation failed";
    return report;
  }

  MomentumLedgerCudaStepper stepper(std::move(preparation.state), action,
                                    normalization.mapped_field_work_coefficient);
  // Sec 8 scope is wiring only ("runner schema, CUDA calls, masked-kernel
  // interface, per-tick accumulator wiring"), not physics validity of a
  // deliberately minimal, zero-formation-tick L=17 probe state -- see the
  // root-cause note on set_require_physics_gates.  Sec 6.3 G2 remains
  // unconditional for the registered --run campaign's own stepper.
  stepper.set_require_physics_gates(false);
  if (!stepper.valid()) {
    report.error = std::string("stepper invalid: ") + stepper.ledger_error();
    return report;
  }
  stepper.enable_host_parity(true);
  // At L=17 every registered radius reaches the periodic half-diameter, so the
  // registered masks degenerate to the whole domain.  That is recorded, not
  // repaired: Sec 8 forbids changing the radius set.  The masked-kernel
  // interface is exercised separately by the labelled probe below.
  report.radius_degenerate = kMomentumRadii[0] >= L / 2;

  for (int tick = 1; tick <= kMomentumFirewallTicks; ++tick) {
    stepper.capture_host_before_state();
    const auto step = stepper.advance(false);
    if (!step.valid || !step.ledger_valid) {
      report.error = "firewall tick failed";
      return report;
    }
    report.ticks = tick;
    report.maximum_tick_identity_ratio = std::max(
        report.maximum_tick_identity_ratio, step.maximum_tick_identity_ratio);
    report.maximum_reynolds_ratio =
        std::max(report.maximum_reynolds_ratio, step.maximum_reynolds_ratio);
    report.scalar_only = report.scalar_only
        && step.telemetry.complete_field_downloads == 0;
  }
  report.maximum_host_parity_residual = stepper.maximum_host_parity_residual();

  const auto& totals = stepper.accumulators();
  for (int localization = 0; localization < 2; ++localization)
    for (int component = 0; component < 3; ++component)
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot)
        report.maximum_complementarity = std::max(
            report.maximum_complementarity,
            std::abs(totals.ledger[localization][component][slot]
                         .complementarity_residual()));

  // Labelled, non-evidential masked-kernel interface probe.  It uses small
  // radii so that straddling bonds actually exist at L=17; it feeds no gate,
  // no accumulator of the registered run, and no verdict.
  {
    auto probe_preparation = prepare_finite_support_derived_compact_pair(
        make_parent_geometry(L, direction_record), action,
        kMomentumSupportHalfWidth, 1e-13, 4096);
    if (probe_preparation.valid) {
      MomentumLedgerCudaStepper probe(
          std::move(probe_preparation.state), action,
          normalization.mapped_field_work_coefficient);
      probe.set_require_physics_gates(false);
      if (probe.valid()) {
        probe.set_radius(kMomentumFirewallProbeRadii);
        probe.enable_host_parity(true);
        probe.capture_host_before_state();
        const auto step = probe.advance(false);
        if (step.valid && step.ledger_valid) {
          report.maximum_probe_host_parity_residual =
              probe.maximum_host_parity_residual();
          const auto& probe_totals = probe.accumulators();
          for (int localization = 0; localization < 2; ++localization)
            for (int component = 0; component < 3; ++component)
              for (int slot = 0; slot < 2; ++slot)
                if (probe_totals.ledger[localization][component][slot]
                        .flux_total() != 0.0)
                  report.probe_exercised_bonds = true;
        }
      }
    }
  }

  report.results_absent = !std::filesystem::exists(momentum_results_directory());
  report.pass = report.state_prepared && report.ticks == kMomentumFirewallTicks
      && report.scalar_only && report.results_absent
      && report.maximum_tick_identity_ratio <= kMomentumTickGate
      && report.maximum_reynolds_ratio <= kMomentumReynoldsGate
      && report.maximum_complementarity <= kMomentumComplementarityGate
      && report.maximum_host_parity_residual <= 1e-10
      && report.maximum_probe_host_parity_residual <= 1e-10
      && report.probe_exercised_bonds;
  return report;
}

// ---------------------------------------------------------------------------
// Campaign driver (Sec 5)
// ---------------------------------------------------------------------------

MomentumCampaign run_momentum_campaign(const ExactnessReport& precheck,
                                       const FirewallReport& firewall) {
  MomentumCampaign result;
  result.precheck = precheck;
  result.precheck_pass = precheck.pass;
  result.firewall_pass = firewall.pass;

  ForensicDirection direction_record;
  if (!select_direction("face", direction_record)) return result;
  const Vec3 direction = direction_unit(direction_record);
  const auto normalization = measure_face_flux_normalization();
  if (!normalization.valid) return result;
  result.interaction_scale = normalization.mapped_field_work_coefficient;
  auto action = forensic_options();
  result.lambda = action.wave_speed * action.dt;

  auto parent = build_parent(kMomentumVolume, direction_record, action,
                             result.interaction_scale);
  result.parent_valid = parent.valid;
  if (!parent.valid) return result;

  auto aged_state = std::move(parent.state);
  {
    MorphologyCudaStepper aging(std::move(aged_state), action,
                                result.interaction_scale);
    if (!aging.valid()) return result;
    result.aging_valid = true;
    for (int tick = 1; tick <= kMomentumAge; ++tick) {
      const auto step = aging.advance(tick == kMomentumAge);
      result.aging_valid = result.aging_valid && step.valid && step.common
          && step.common_residual <= kMomentumInheritedGate
          && step.energy_residual <= kMomentumInheritedGate
          && step.speed_excess <= kMomentumInheritedGate
          && (tick != kMomentumAge
              || (step.regularity_measured
                  && step.sigma_min >= kMomentumMinimumSigma
                  && step.condition <= kMomentumMaximumCondition
                  && step.inverse_valid
                  && step.inverse_residual <= kMomentumInheritedGate));
      if (!step.valid) break;
    }
    aged_state = aging.release_state();
  }
  if (!result.aging_valid) return result;

  auto rest_initial = aged_state;
  auto moving_initial = aged_state;
  for (auto& point : moving_initial.constituents)
    point.momentum += direction * kMomentumBoost;
  const auto rest_core = observe_support_invariant_matter(rest_initial, action);
  const auto moving_core =
      observe_support_invariant_matter(moving_initial, action);
  result.rest_initialized = rest_core.valid && rest_core.member
      && rest_core.graph_margin >= kMomentumMinimumCoreMargin
      && rest_core.energy_margin >= kMomentumMinimumCoreMargin;
  result.moving_initialized = moving_core.valid && moving_core.member
      && moving_core.graph_margin >= kMomentumMinimumCoreMargin
      && moving_core.energy_margin >= kMomentumMinimumCoreMargin;
  if (!result.rest_initialized || !result.moving_initialized) return result;

  result.laboratory_center = object_center(moving_initial);
  result.moving_initial_hash = state_hash(moving_initial);
  const double causal_reach =
      (kMomentumFormation + kMomentumAge + kMomentumTicks) * result.lambda;
  result.boundary_margin = 0.5 * kMomentumVolume - 4.0 - causal_reach;
  result.boundary_clear = result.boundary_margin > 0.0;

  const Vec3 rest_matter_initial = matter_momentum(rest_initial);
  const Vec3 moving_matter_initial = matter_momentum(moving_initial);
  const Vec3 rest_local_initial = matched_local_translation_momentum(
      rest_initial.electric, rest_initial.magnetic_half)
      * result.interaction_scale;
  const Vec3 moving_local_initial = matched_local_translation_momentum(
      moving_initial.electric, moving_initial.magnetic_half)
      * result.interaction_scale;

  ConnectedMooreBlockState rest_final, moving_final;
  {
    auto rest = std::make_unique<MomentumLedgerCudaStepper>(
        std::move(rest_initial), action, result.interaction_scale);
    auto moving = std::make_unique<MomentumLedgerCudaStepper>(
        moving_initial, action, result.interaction_scale);
    if (!rest->valid() || !moving->valid()) return result;
    result.forward_valid = true;

    MomentumCheckpoint zero;
    zero.tau = 0;
    zero.valid = true;
    zero.rest = make_arm_checkpoint(0, rest->state(), *rest, result.rest_gates,
                                    result.laboratory_center, direction,
                                    rest_matter_initial, rest_local_initial,
                                    rest_local_initial);
    zero.moving = make_arm_checkpoint(0, moving->state(), *moving,
                                      result.moving_gates,
                                      result.laboratory_center, direction,
                                      moving_matter_initial,
                                      moving_local_initial,
                                      moving_local_initial);
    // Sec 3 G_U is vacuous at tau = 0: no tick has been accumulated, so every
    // accumulator is still empty and Pi_i(R,0) is not yet known.  Pi_i(R,0) is
    // recovered from the first tick's chi_t-masked content (Sec 2.7) and
    // appears as initial_content from tau = 64 onward.
    zero.rest.units_pass = true;
    zero.moving.units_pass = true;
    zero.rest.maximum_unit_residual = 0.0;
    zero.moving.maximum_unit_residual = 0.0;
    result.checkpoints.push_back(std::move(zero));

    for (int tau = 1; tau <= kMomentumTicks; ++tau) {
      const bool checkpoint = tau % kMomentumStride == 0;
      const auto rest_step = rest->advance(checkpoint);
      const auto moving_step = moving->advance(checkpoint);
      accumulate_gates(result.rest_gates, rest_step, tau);
      accumulate_gates(result.moving_gates, moving_step, tau);
      result.maximum_rest_displacement = std::max(
          result.maximum_rest_displacement,
          (object_center(rest->state()) - result.laboratory_center).mag());
      result.forward_valid = result.forward_valid && result.rest_gates.inherited
          && result.moving_gates.inherited
          && result.maximum_rest_displacement <= kMomentumInheritedGate;
      if (!rest_step.valid || !moving_step.valid) break;
      if (!checkpoint) continue;
      MomentumCheckpoint record;
      record.tau = tau;
      record.rest = make_arm_checkpoint(
          tau, rest->state(), *rest, result.rest_gates,
          result.laboratory_center, direction, rest_matter_initial,
          rest_local_initial, rest_step.local_after);
      record.moving = make_arm_checkpoint(
          tau, moving->state(), *moving, result.moving_gates,
          result.laboratory_center, direction, moving_matter_initial,
          moving_local_initial, moving_step.local_after);
      record.valid = record.rest.inherited_pass && record.moving.inherited_pass
          && (record.rest.center - result.laboratory_center).mag()
              <= kMomentumInheritedGate;
      result.forward_valid = result.forward_valid && record.valid;
      result.checkpoints.push_back(std::move(record));
      std::cout << std::setprecision(17) << kMomentumFtdId
                << " forward tau=" << tau
                << " d=" << result.checkpoints.back().moving.displacement
                << " units=" << std::boolalpha
                << result.checkpoints.back().moving.units_pass
                << " valid=" << result.forward_valid << '\n';
    }
    rest_final = rest->release_state();
    moving_final = moving->release_state();
  }
  result.forward_valid = result.forward_valid
      && result.checkpoints.size()
          == static_cast<std::size_t>(kMomentumCheckpoints);
  result.moving_forward_final_hash = state_hash(moving_final);

  if (result.forward_valid) {
    auto reverse_state = std::move(moving_final);
    auto reverse_options = action;
    reverse_options.defer_volume_diagnostics = false;
    reverse_options.measure_final_root_regularity = false;
    ConnectedMooreBlockSolveCache reverse_cache;
    result.reverse_valid = true;
    for (int index = 1; index <= kMomentumTicks; ++index) {
      const auto reverse = solve_connected_moore_block_reverse(
          reverse_state, reverse_options, &reverse_cache);
      const double common = common_residual_0764(reverse);
      result.reverse_maximum_common =
          std::max(result.reverse_maximum_common, common);
      result.reverse_valid = result.reverse_valid && reverse.valid
          && reverse.common_action_gates_pass
          && common <= kMomentumInheritedGate;
      if (!reverse.valid) break;
      reverse_state = std::move(reverse.earlier);
      result.reverse_steps = index;
      if (index % kMomentumStride == 0)
        std::cout << kMomentumFtdId << " reverse steps=" << index
                  << " valid=" << std::boolalpha << result.reverse_valid << '\n';
    }
    result.reverse_recovery = connected_moore_block_state_max_difference(
        moving_initial, reverse_state);
    result.reverse_discrete_exact =
        discrete_state_equal(moving_initial, reverse_state);
    result.moving_reversed_hash = state_hash(reverse_state);
    result.reverse_valid = result.reverse_valid
        && result.reverse_steps == kMomentumTicks
        && result.reverse_discrete_exact
        && result.reverse_recovery <= kMomentumReverseGate;
  }
  classify_campaign(result);
  return result;
}

// ---------------------------------------------------------------------------
// Reporting
// ---------------------------------------------------------------------------

void write_momentum_region(std::ostream& out,
                           const MomentumRegionRecord& value) {
  out << "{\"radius\": " << value.radius
      << ", \"used\": " << value.used
      << ", \"clearance_marginal\": " << value.clearance_marginal
      << ", \"content\": " << json_number(value.content)
      << ", \"initial_content\": " << json_number(value.initial_content)
      << ", \"content_change\": " << json_number(value.content_change)
      << ", \"flux\": " << json_number(value.flux)
      << ", \"sweep\": " << json_number(value.sweep)
      << ", \"source\": " << json_number(value.source)
      << ", \"flux_complement\": " << json_number(value.flux_complement)
      << ", \"sweep_complement\": " << json_number(value.sweep_complement)
      << ", \"flux_quadrature\": " << json_number(value.flux_quadrature)
      << ", \"sweep_quadrature\": " << json_number(value.sweep_quadrature)
      << ", \"ledger_residual\": " << json_number(value.ledger_residual)
      << ", \"window_maximum_tick_residual\": "
      << json_number(value.window_maximum_tick_residual)
      << ", \"maximum_chain_residual\": "
      << json_number(value.maximum_chain_residual)
      << ", \"eta\": " << json_number(value.eta)
      << ", \"transfer\": " << json_number(value.transfer)
      << ", \"rho\": " << json_number(value.rho)
      << ", \"rho_ceiling\": " << json_number(value.rho_ceiling)
      << ", \"kappa\": " << json_number(value.kappa)
      << ", \"retention_identity_residual\": "
      << json_number(value.retention_identity_residual)
      << ", \"gate_identity\": " << value.gate_identity
      << ", \"gate_complementarity\": " << value.gate_complementarity
      << ", \"gate_enclosure\": " << value.gate_enclosure << '}';
}

void write_momentum_arm(std::ostream& out,
                        const MomentumArmCheckpoint& value) {
  out << "{\"tau\": " << value.tau << ", \"center\": ";
  write_vec(out, value.center);
  out << ", \"mask_center\": [" << value.mask_center[0] << ", "
      << value.mask_center[1] << ", " << value.mask_center[2] << ']'
      << ", \"displacement\": " << json_number(value.displacement)
      << ", \"state_hash\": \"" << value.state_hash << "\""
      << ", \"source_half_width\": " << value.source_half_width
      << ", \"units_pass\": " << value.units_pass
      << ", \"maximum_unit_residual\": "
      << json_number(value.maximum_unit_residual)
      << ", \"inherited_pass\": " << value.inherited_pass
      << ", \"matter_momentum\": ";
  write_vec(out, value.matter_momentum);
  out << ", \"local_field_momentum\": ";
  write_vec(out, value.local_field_momentum);
  out << ", \"matter_momentum_change\": ";
  write_vec(out, value.matter_change);
  out << ", \"local_field_momentum_change\": ";
  write_vec(out, value.local_change);
  out << ", \"momentum_cumulative_defect\": ";
  write_vec(out, value.defect);
  out << ", \"localizations\": [";
  for (int localization = 0; localization < 2; ++localization) {
    if (localization) out << ',';
    out << "{\"name\": \"" << localization_name(localization)
        << "\", \"components\": [";
    for (int component = 0; component < 3; ++component) {
      if (component) out << ',';
      out << "{\"component\": \"" << component_name(component)
          << "\", \"regions\": [";
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
        if (slot) out << ',';
        write_momentum_region(out,
            value.localization[localization][component].region[slot]);
      }
      out << "]}";
    }
    out << "]}";
  }
  out << "]}";
}

void write_momentum_gates(std::ostream& out,
                          const MomentumArmRunningGates& value) {
  out << "{\"inherited\": " << value.inherited
      << ", \"maximum_common_residual\": "
      << json_number(value.maximum_common_residual)
      << ", \"maximum_energy_residual\": "
      << json_number(value.maximum_energy_residual)
      << ", \"maximum_speed_excess\": "
      << json_number(value.maximum_speed_excess)
      << ", \"minimum_sigma\": " << json_number(value.minimum_sigma)
      << ", \"maximum_condition\": " << json_number(value.maximum_condition)
      << ", \"maximum_inverse_residual\": "
      << json_number(value.maximum_inverse_residual)
      << ", \"minimum_graph_margin\": "
      << json_number(value.minimum_graph_margin)
      << ", \"minimum_energy_margin\": "
      << json_number(value.minimum_energy_margin)
      << ", \"maximum_tick_identity_ratio\": "
      << json_number(value.maximum_tick_identity_ratio)
      << ", \"maximum_reynolds_ratio\": "
      << json_number(value.maximum_reynolds_ratio)
      << ", \"maximum_source_half_width\": " << value.maximum_source_half_width
      << ", \"site_hops\": " << value.site_hops
      << ", \"mask_centre_changes\": " << value.centre_changes
      << ", \"sweep_events\": [";
  for (std::size_t i = 0; i < value.sweep_events.size(); ++i) {
    if (i) out << ',';
    out << '[' << value.sweep_events[i][0] << ", " << value.sweep_events[i][1]
        << ", " << value.sweep_events[i][2] << ", "
        << value.sweep_events[i][3] << ']';
  }
  out << "]}";
}

void write_precheck(std::ostream& out, const ExactnessReport& value) {
  out << "{\"pass\": " << value.pass << ", \"volume\": " << value.L
      << ", \"lambda\": " << json_number(value.lambda)
      << ", \"site_channel_under_site_mask\": "
      << json_number(value.site_channel_under_site_mask)
      << ", \"site_channel_under_component_mask\": "
      << json_number(value.site_channel_under_component_mask)
      << ", \"site_channel_exercised\": " << value.site_channel_exercised
      << ", \"census\": [";
  for (std::size_t i = 0; i < value.census.size(); ++i) {
    if (i) out << ',';
    const auto& record = value.census[i];
    out << "{\"operator\": \"" << record.operator_name
        << "\", \"component\": \"" << component_name(record.component)
        << "\", \"displacements\": " << record.displacement_count
        << ", \"entries\": " << record.entry_count
        << ", \"classes\": " << record.class_count
        << ", \"bond_generators\": " << record.bond_generator_count
        << ", \"site_generators\": " << record.site_generator_count
        << ", \"maximum_l1\": " << record.maximum_l1
        << ", \"maximum_linf\": " << record.maximum_linf
        << ", \"skewness_residual\": "
        << json_number(record.skewness_residual) << '}';
  }
  out << "], \"checks\": [";
  for (std::size_t i = 0; i < value.checks.size(); ++i) {
    if (i) out << ',';
    out << "{\"name\": \"" << value.checks[i].name << "\", \"residual\": "
        << json_number(value.checks[i].residual) << ", \"tolerance\": "
        << json_number(value.checks[i].tolerance) << ", \"pass\": "
        << value.checks[i].pass << '}';
  }
  out << "]}";
}

void write_firewall(std::ostream& out, const FirewallReport& value) {
  out << "{\"pass\": " << value.pass
      << ", \"volume\": " << kMomentumFirewallVolume
      << ", \"ticks\": " << value.ticks
      << ", \"ledger_available\": " << value.ledger_available
      << ", \"state_prepared\": " << value.state_prepared
      << ", \"scalar_only\": " << value.scalar_only
      << ", \"registered_radii_degenerate_at_firewall_volume\": "
      << value.radius_degenerate
      << ", \"interface_probe_exercised_bonds\": " << value.probe_exercised_bonds
      << ", \"maximum_tick_identity_ratio\": "
      << json_number(value.maximum_tick_identity_ratio)
      << ", \"maximum_reynolds_ratio\": "
      << json_number(value.maximum_reynolds_ratio)
      << ", \"maximum_complementarity\": "
      << json_number(value.maximum_complementarity)
      << ", \"maximum_host_parity_residual\": "
      << json_number(value.maximum_host_parity_residual)
      << ", \"maximum_interface_probe_host_parity_residual\": "
      << json_number(value.maximum_probe_host_parity_residual)
      << ", \"results_absent\": " << value.results_absent
      << ", \"error\": \"" << value.error << "\"}";
}

void write_campaign_json(const MomentumCampaign& value,
                         const FirewallReport& firewall) {
  const auto directory = momentum_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream out(directory
      / (std::string(kMomentumResultSlug)
         + "_total_momentum_stress_ledger_v1.json"));
  out << std::boolalpha << std::setprecision(17)
      << "{\n  \"ftd_id\": \"" << kMomentumFtdId << "\",\n"
      << "  \"protocol_sha256\": \"" << kMomentumProtocolSha256 << "\",\n"
      << "  \"run_record_schema\": \"" << kMomentumResultSlug
      << "_total_momentum_stress_ledger_v1\",\n"
      << "  \"observer_mode\": "
         "\"unit_bond_stress_ledger_moving_control_volume_two_localizations\","
         "\n"
      << "  \"volume\": " << kMomentumVolume << ",\n"
      << "  \"formation_ticks\": " << kMomentumFormation << ",\n"
      << "  \"preparation_age\": " << kMomentumAge << ",\n"
      << "  \"discovery_ticks\": " << kMomentumTicks << ",\n"
      << "  \"checkpoint_stride\": " << kMomentumStride << ",\n"
      << "  \"boost\": " << json_number(kMomentumBoost) << ",\n"
      << "  \"direction\": [0, 0, 1],\n"
      << "  \"lambda\": " << json_number(value.lambda) << ",\n"
      << "  \"interaction_scale\": " << json_number(value.interaction_scale)
      << ",\n  \"radii\": [";
  for (int slot = 0; slot < kCudaMomentumMaximumRadii; ++slot) {
    if (slot) out << ", ";
    out << kMomentumRadii[slot];
  }
  out << "],\n  \"physics_radii\": [" << kMomentumRadii[kMomentumPhysicsSlots[0]]
      << ", " << kMomentumRadii[kMomentumPhysicsSlots[1]] << ", "
      << kMomentumRadii[kMomentumPhysicsSlots[2]] << "],\n"
      << "  \"outer_radius\": " << kMomentumRadii[kMomentumOuterSlot] << ",\n"
      << "  \"tolerances\": {\"per_tick_identity\": "
      << json_number(kMomentumTickGate) << ", \"reynolds\": "
      << json_number(kMomentumReynoldsGate) << ", \"complementarity\": "
      << json_number(kMomentumComplementarityGate) << ", \"units\": "
      << json_number(kMomentumUnitGate) << ", \"enclosure\": "
      << json_number(kMomentumKappaGate) << ", \"inherited\": "
      << json_number(kMomentumInheritedGate) << ", \"reverse\": "
      << json_number(kMomentumReverseGate) << "},\n"
      << "  \"bands\": {\"half_width\": "
      << json_number(kMomentumBandHalfWidth) << ", \"flatness\": "
      << json_number(kMomentumFlatness) << ", \"accumulation\": "
      << json_number(kMomentumAccumulation) << "},\n"
      << "  \"exactness_precheck\": ";
  write_precheck(out, value.precheck);
  out << ",\n  \"firewall\": ";
  write_firewall(out, firewall);
  out << ",\n  \"laboratory_center\": ";
  write_vec(out, value.laboratory_center);
  out << ",\n  \"parent_valid\": " << value.parent_valid
      << ",\n  \"aging_valid\": " << value.aging_valid
      << ",\n  \"rest_initialized\": " << value.rest_initialized
      << ",\n  \"moving_initialized\": " << value.moving_initialized
      << ",\n  \"forward_valid\": " << value.forward_valid
      << ",\n  \"boundary_clear\": " << value.boundary_clear
      << ",\n  \"boundary_margin\": " << json_number(value.boundary_margin)
      << ",\n  \"maximum_rest_displacement\": "
      << json_number(value.maximum_rest_displacement)
      << ",\n  \"moving_initial_hash\": \"" << value.moving_initial_hash
      << "\",\n  \"moving_forward_final_hash\": \""
      << value.moving_forward_final_hash
      << "\",\n  \"moving_reversed_hash\": \"" << value.moving_reversed_hash
      << "\",\n  \"reverse_valid\": " << value.reverse_valid
      << ",\n  \"reverse_discrete_exact\": " << value.reverse_discrete_exact
      << ",\n  \"reverse_recovery\": " << json_number(value.reverse_recovery)
      << ",\n  \"reverse_maximum_common\": "
      << json_number(value.reverse_maximum_common)
      << ",\n  \"reverse_steps\": " << value.reverse_steps
      << ",\n  \"prior_mismatch\": " << value.prior_mismatch
      << ",\n  \"far_field_active\": " << value.far_field_active
      << ",\n  \"clearance_marginal_radius\": "
      << kMomentumRadii[kMomentumMarginalSlot]
      << ",\n  \"rest_gates\": ";
  write_momentum_gates(out, value.rest_gates);
  out << ",\n  \"moving_gates\": ";
  write_momentum_gates(out, value.moving_gates);
  out << ",\n  \"verdicts\": [";
  for (int component = 0; component < 3; ++component) {
    if (component) out << ',';
    const auto& verdict = value.verdict[component];
    out << "{\"component\": \"" << component_name(component)
        << "\", \"verdict\": \"" << verdict.verdict
        << "\", \"L1_bucket\": \"" << verdict.bucket[0]
        << "\", \"L2_bucket\": \"" << verdict.bucket[1]
        << "\", \"significant\": " << verdict.significant
        << ", \"rest_arm_clean\": " << verdict.rest_arm_clean
        << ", \"instrument_limited\": " << verdict.instrument_limited
        << ", \"exchange_sign_inverted\": " << verdict.exchange_sign_inverted
        << ", \"qualifying_checkpoints\": " << verdict.qualifying_checkpoints
        << '}';
  }
  out << "],\n  \"outcome\": \"" << value.outcome << "\",\n"
      << "  \"checkpoints\": [";
  for (std::size_t i = 0; i < value.checkpoints.size(); ++i) {
    if (i) out << ',';
    out << "\n{\"tau\": " << value.checkpoints[i].tau
        << ", \"valid\": " << value.checkpoints[i].valid << ", \"rest\": ";
    write_momentum_arm(out, value.checkpoints[i].rest);
    out << ", \"moving\": ";
    write_momentum_arm(out, value.checkpoints[i].moving);
    out << '}';
  }
  out << "\n  ],\n  \"production_changed\": false,\n"
      << "  \"dynamics_changed\": false,\n"
      << "  \"new_primitive_added\": false\n}\n";
}

void write_campaign_csv(const MomentumCampaign& value) {
  const auto directory = momentum_results_directory();
  std::filesystem::create_directories(directory);
  std::ofstream out(directory
      / (std::string(kMomentumResultSlug)
         + "_total_momentum_stress_ledger_v1_body.csv"));
  out << std::setprecision(17)
      << "arm,tau,localization,component,radius,content,initial_content,"
         "content_change,flux,sweep,source,flux_complement,sweep_complement,"
         "flux_quadrature,sweep_quadrature,ledger_residual,"
         "window_maximum_tick_residual,maximum_chain_residual,eta,transfer,"
         "rho,rho_ceiling,kappa,retention_identity_residual,gate_identity,"
         "gate_complementarity,gate_enclosure\n";
  const auto emit = [&](const char* arm, const MomentumArmCheckpoint& record) {
    for (int localization = 0; localization < 2; ++localization)
      for (int component = 0; component < 3; ++component)
        for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
          const auto& region =
              record.localization[localization][component].region[slot];
          out << arm << ',' << record.tau << ','
              << localization_name(localization) << ','
              << component_name(component) << ',' << region.radius << ','
              << region.content << ',' << region.initial_content << ','
              << region.content_change << ',' << region.flux << ','
              << region.sweep << ',' << region.source << ','
              << region.flux_complement << ',' << region.sweep_complement << ','
              << region.flux_quadrature << ',' << region.sweep_quadrature << ','
              << region.ledger_residual << ','
              << region.window_maximum_tick_residual << ','
              << region.maximum_chain_residual << ',' << region.eta << ','
              << region.transfer << ',' << region.rho << ','
              << region.rho_ceiling << ',' << region.kappa << ','
              << region.retention_identity_residual << ','
              << (region.gate_identity ? 1 : 0) << ','
              << (region.gate_complementarity ? 1 : 0) << ','
              << (region.gate_enclosure ? 1 : 0) << '\n';
        }
  };
  for (const auto& checkpoint : value.checkpoints) {
    emit("rest", checkpoint.rest);
    emit("moving", checkpoint.moving);
  }
}

void print_precheck(const ExactnessReport& report) {
  std::cout << std::setprecision(17) << std::boolalpha
            << "exactness pre-check (L=" << report.L << "): pass="
            << report.pass << '\n'
            << "  site channel under site mask      = "
            << report.site_channel_under_site_mask << " (must be exactly 0)\n"
            << "  site channel under component mask = "
            << report.site_channel_under_component_mask
            << " (must be non-zero)\n";
  for (const auto& check : report.checks)
    if (!check.pass)
      std::cout << "  FAIL " << check.name << " residual=" << check.residual
                << " tolerance=" << check.tolerance << '\n';
  for (const auto& record : report.census)
    std::cout << "  census " << record.operator_name << '_'
              << component_name(record.component)
              << " displacements=" << record.displacement_count
              << " entries=" << record.entry_count
              << " classes=" << record.class_count
              << " bond_generators=" << record.bond_generator_count
              << " site_generators=" << record.site_generator_count
              << " l1=" << record.maximum_l1
              << " linf=" << record.maximum_linf << '\n';
}

void print_firewall(const FirewallReport& report) {
  std::cout << std::setprecision(17) << std::boolalpha
            << "firewall (L=" << kMomentumFirewallVolume << "): pass="
            << report.pass << " ticks=" << report.ticks
            << " scalar_only=" << report.scalar_only
            << " identity=" << report.maximum_tick_identity_ratio
            << " reynolds=" << report.maximum_reynolds_ratio
            << " parity=" << report.maximum_host_parity_residual
            << " probe_parity=" << report.maximum_probe_host_parity_residual
            << " probe_bonds=" << report.probe_exercised_bonds
            << " registered_radii_degenerate=" << report.radius_degenerate
            << " results_absent=" << report.results_absent;
  if (!report.error.empty()) std::cout << " error=" << report.error;
  std::cout << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  const std::string mode = argc == 2 ? argv[1] : std::string();
  if (mode == "--exactness-precheck") {
    const auto report = run_exactness_precheck();
    print_precheck(report);
    return report.pass ? 0 : 1;
  }
  if (mode == "--firewall") {
    const auto report = run_firewall();
    print_firewall(report);
    return report.pass ? 0 : 1;
  }
  if (mode != "--run") {
    std::cout << "total momentum stress ledger runner: "
                 "--exactness-precheck | --firewall | --run\n";
    return argc == 1 ? 0 : 2;
  }
  // Sec 12: locking order.  The instrumentation, the pre-check and the
  // firewall come first; the id and the protocol hash come last.  Until they
  // are substituted no registered artifact may be written.
  if (std::string(kMomentumProtocolSha256) == "UNLOCKED") {
    std::cerr << "protocol hash not locked; --run refused (see Sec 12)\n";
    return 3;
  }
  if (std::filesystem::exists(momentum_results_directory())) {
    std::cerr << "result directory already exists\n";
    return 4;
  }
  const auto precheck = run_exactness_precheck();
  print_precheck(precheck);
  if (!precheck.pass) {
    std::cerr << "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED "
                 "(Sec 6.4 exactness pre-check)\n";
    return 5;
  }
  const auto firewall = run_firewall();
  print_firewall(firewall);
  if (!firewall.pass) {
    std::cerr << "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED "
                 "(Sec 8 firewall)\n";
    return 6;
  }
  const auto result = run_momentum_campaign(precheck, firewall);
  write_campaign_json(result, firewall);
  write_campaign_csv(result);
  std::cout << std::boolalpha << kMomentumFtdId
            << " forward=" << result.forward_valid
            << " reverse=" << result.reverse_valid
            << " outcome=" << result.outcome
            << " x=" << result.verdict[0].verdict
            << " y=" << result.verdict[1].verdict
            << " z=" << result.verdict[2].verdict << '\n';
  return result.outcome == "MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED"
      || result.outcome == "MOMENTUM_LEDGER_BASELINE_INVALID" ? 1 : 0;
}

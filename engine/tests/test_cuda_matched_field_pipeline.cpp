// CUDA qualification for the matched face/edge field and regional observer.

#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/cuda_matched_field_pipeline.h"
#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/matched_face_momentum_transaction.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/quadratic_coat_face_current.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

template <typename Field>
double difference(const Field& left, const Field& right) {
  double result = 0.0;
  for (std::size_t i = 0; i < left.x.size(); ++i) {
    result = std::max({result, std::abs(left.x[i]-right.x[i]),
        std::abs(left.y[i]-right.y[i]), std::abs(left.z[i]-right.z[i])});
  }
  return result;
}

void add_current(ftd::eft::MatchedFaceFlux& field,
                 const ftd::eft::QuadraticCoatFaceCurrent& segment,
                 double scale) {
  for (const auto& entry : segment.sparse_current) {
    const auto index = static_cast<std::size_t>(segment.index(
        entry.face.x, entry.face.y, entry.face.z));
    auto& component = entry.axis == 0 ? field.x
        : (entry.axis == 1 ? field.y : field.z);
    component[index] += scale*entry.value;
  }
}

double profile_difference(
    const ftd::eft::BatchedRegionalEnergyProfile& left,
    const ftd::eft::BatchedRegionalEnergyProfile& right) {
  if (left.regions.size() != right.regions.size()) return INFINITY;
  double result = std::abs(left.maximum_scalar_equivalence_residual
      -right.maximum_scalar_equivalence_residual);
  result = std::max({result,
      std::abs(left.energy_before-right.energy_before),
      std::abs(left.energy_pre_current-right.energy_pre_current),
      std::abs(left.energy_after-right.energy_after)});
  for (std::size_t i = 0; i < left.regions.size(); ++i) {
    const auto& a = left.regions[i];
    const auto& b = right.regions[i];
    result = std::max({result,
        std::abs(a.energy_before-b.energy_before),
        std::abs(a.energy_pre_current-b.energy_pre_current),
        std::abs(a.energy_after-b.energy_after),
        std::abs(a.boundary_transport_into-b.boundary_transport_into),
        std::abs(a.source_exchange_into_field-b.source_exchange_into_field),
        std::abs(a.energy_change-b.energy_change),
        std::abs(a.global_source_free_residual-b.global_source_free_residual),
        std::abs(a.partition_residual-b.partition_residual),
        std::abs(a.regional_ledger_residual-b.regional_ledger_residual)});
  }
  return result;
}

std::vector<double> density(
    int L, const std::vector<ftd::eft::QuadraticCoatFaceCurrent>& segments,
    bool later, double scale) {
  std::vector<double> result(static_cast<std::size_t>(L)*L*L,0.0);
  for(const auto& segment:segments) {
    const auto& coat=later?segment.end_coat:segment.start_coat;
    for(std::size_t item=0;item<coat.weight_count;++item) {
      const auto& entry=coat.weights[item];
      result[static_cast<std::size_t>(segment.index(
          entry.site.x,entry.site.y,entry.site.z))]+=scale*entry.weight;
    }
  }
  return result;
}

double vector_difference(const ftd::Vec3& left,const ftd::Vec3& right) {
  return std::max({std::abs(left.x-right.x),std::abs(left.y-right.y),
                   std::abs(left.z-right.z)});
}

}  // namespace

int run_large_volume_smoke() {
  using namespace ftd::eft;
  constexpr int L = 321;
  constexpr double lambda = 0.25*ftd::C_SPEED;
  MatchedFaceFlux electric(L);
  MatchedEdgeField magnetic(L);
  const auto segment = make_quadratic_coat_face_current(
      L, {160.20,160.10,160.30}, {160.28,160.06,160.34}, +1, false);
  if (!segment.valid) return 1;
  CudaMatchedFieldPipeline gpu(L);
  if (!gpu.valid() || !gpu.upload(electric, magnetic)
      || !gpu.prepare_forward(lambda)) {
    std::cerr << "large CUDA setup failed: " << gpu.error() << '\n';
    return 1;
  }
  MatchedEdgeField magnetic_after;
  MatchedFaceFlux electric_pre;
  if (!gpu.download_prepared(magnetic_after, electric_pre)
      || !gpu.apply_sparse_current({segment}, 1.0)) {
    std::cerr << "large CUDA transaction failed: " << gpu.error() << '\n';
    return 1;
  }
  const auto profile = gpu.observe(lambda, {160.0,160.0,160.0},
      {8,12,16,24,32,48}, 1e-10);
  const auto diagnostics=gpu.diagnose_common_action(
      {segment},1.0,1.0,ftd::C_SPEED,0.25,1e-10);
  MatchedFaceFlux electric_after;
  MatchedEdgeField magnetic_after_copy;
  if (!profile.valid || !diagnostics.valid || !gpu.download_after(
      electric_after, magnetic_after_copy)) {
    std::cerr << "large CUDA observation failed: " << gpu.error() << '\n';
    return 1;
  }
  const auto& timing = gpu.timings();
  std::cout << std::setprecision(17)
      << "L=321 upload_ms=" << timing.upload_ms
      << " prepare_ms=" << timing.prepare_ms
      << " current_ms=" << timing.current_ms
      << " observe_ms=" << timing.observe_ms
      << " download_ms=" << timing.download_ms
      << " energy_after=" << profile.energy_after
      << " gauss_after=" << diagnostics.gauss_after_residual
      << " spline_after=" << diagnostics.spline_field_momentum_after.mag()
      << '\n';
  return 0;
}

int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--large")
    return run_large_volume_smoke();
  if (argc != 1) return 2;
  using namespace ftd::eft;
  constexpr int L = 17;
  constexpr double lambda = 0.25*ftd::C_SPEED;
  constexpr double polarity_scale = 0.875;
  MatchedFaceFlux electric(L);
  MatchedEdgeField magnetic(L);
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(electric.index(x,y,z));
        electric.x[i] = 1e-3*std::sin(0.31*x+0.17*y-0.13*z);
        electric.y[i] = 1e-3*std::cos(0.19*x-0.23*y+0.29*z);
        electric.z[i] = 1e-3*std::sin(0.11*x+0.37*y+0.07*z);
        magnetic.x[i] = 7e-4*std::cos(0.13*x+0.21*y-0.31*z);
        magnetic.y[i] = 7e-4*std::sin(0.27*x-0.09*y+0.33*z);
        magnetic.z[i] = 7e-4*std::cos(0.15*x+0.25*y+0.05*z);
      }

  auto cpu_magnetic_after = magnetic;
  const auto electric_curl = matched_curl_adjoint(electric);
  for (std::size_t i = 0; i < electric.x.size(); ++i) {
    cpu_magnetic_after.x[i] -= lambda*electric_curl.x[i];
    cpu_magnetic_after.y[i] -= lambda*electric_curl.y[i];
    cpu_magnetic_after.z[i] -= lambda*electric_curl.z[i];
  }
  auto cpu_pre = electric;
  const auto magnetic_curl = matched_curl(cpu_magnetic_after);
  for (std::size_t i = 0; i < electric.x.size(); ++i) {
    cpu_pre.x[i] += lambda*magnetic_curl.x[i];
    cpu_pre.y[i] += lambda*magnetic_curl.y[i];
    cpu_pre.z[i] += lambda*magnetic_curl.z[i];
  }
  const auto positive = make_quadratic_coat_face_current(
      L, {8.20,8.10,8.30}, {8.28,8.06,8.34}, +1, false);
  const auto negative = make_quadratic_coat_face_current(
      L, {9.10,8.35,7.90}, {9.04,8.41,7.87}, -1, false);
  const std::vector<QuadraticCoatFaceCurrent> segments{positive,negative};
  if (!positive.valid || !negative.valid) return 1;
  auto cpu_after = cpu_pre;
  for (const auto& segment : segments)
    add_current(cpu_after, segment, -polarity_scale);
  const std::vector<int> radii{2,4,6};
  const ftd::Vec3 center{8.0,8.0,8.0};
  const auto cpu_profile = evaluate_batched_regional_energy_profile(
      electric, magnetic, cpu_pre, cpu_magnetic_after, cpu_after,
      lambda, center, radii, 1e-10);

  CudaMatchedFieldPipeline gpu(L);
  if (!gpu.valid()) {
    std::cerr << "CUDA matched pipeline unavailable: " << gpu.error() << '\n';
    return 1;
  }
  if (!gpu.upload(electric, magnetic) || !gpu.prepare_forward(lambda)) {
    std::cerr << "CUDA preparation failed: " << gpu.error() << '\n';
    return 1;
  }
  MatchedEdgeField gpu_magnetic_after;
  MatchedFaceFlux gpu_pre;
  if (!gpu.download_prepared(gpu_magnetic_after, gpu_pre)
      || !gpu.apply_sparse_current(segments, polarity_scale)) {
    std::cerr << "CUDA prepared/current stage failed: " << gpu.error() << '\n';
    return 1;
  }
  const auto gpu_profile = gpu.observe(lambda, center, radii, 1e-10);
  constexpr double interaction_scale=1.25;
  const auto gpu_diagnostics=gpu.diagnose_common_action(
      segments,polarity_scale,interaction_scale,ftd::C_SPEED,0.25,1e-10);
  MatchedFaceFlux gpu_after;
  MatchedEdgeField gpu_after_magnetic;
  if (!gpu.download_after(gpu_after, gpu_after_magnetic)) {
    std::cerr << "CUDA download failed: " << gpu.error() << '\n';
    return 1;
  }
  CudaMatchedFieldPipeline repeat_gpu(L);
  MatchedFaceFlux repeat_after;
  MatchedEdgeField repeat_magnetic;
  if(!repeat_gpu.valid()||!repeat_gpu.upload(electric,magnetic)
      ||!repeat_gpu.prepare_forward(lambda)
      ||!repeat_gpu.apply_sparse_current(segments,polarity_scale)) return 1;
  const auto repeat_profile=repeat_gpu.observe(lambda,center,radii,1e-10);
  const auto repeat_diagnostics=repeat_gpu.diagnose_common_action(
      segments,polarity_scale,interaction_scale,ftd::C_SPEED,0.25,1e-10);
  if(!repeat_gpu.download_after(repeat_after,repeat_magnetic)) return 1;

  const double magnetic_difference = difference(
      cpu_magnetic_after, gpu_magnetic_after);
  const double pre_difference = difference(cpu_pre, gpu_pre);
  const double after_difference = difference(cpu_after, gpu_after);
  const double profile_residual = profile_difference(cpu_profile, gpu_profile);
  const double determinism_residual=std::max({
      difference(gpu_after,repeat_after),
      difference(gpu_after_magnetic,repeat_magnetic),
      profile_difference(gpu_profile,repeat_profile),
      std::abs(gpu_diagnostics.gauss_before_residual
          -repeat_diagnostics.gauss_before_residual),
      std::abs(gpu_diagnostics.gauss_after_residual
          -repeat_diagnostics.gauss_after_residual),
      vector_difference(gpu_diagnostics.local_field_momentum_before,
          repeat_diagnostics.local_field_momentum_before),
      vector_difference(gpu_diagnostics.local_field_momentum_after,
          repeat_diagnostics.local_field_momentum_after),
      vector_difference(gpu_diagnostics.spline_field_momentum_before,
          repeat_diagnostics.spline_field_momentum_before),
      vector_difference(gpu_diagnostics.spline_field_momentum_after,
          repeat_diagnostics.spline_field_momentum_after)});
  ConnectedMooreBlockVolumeDiagnostics cpu_diagnostics;
  cpu_diagnostics.valid=true;
  cpu_diagnostics.gauss_before_residual=max_fractional_gauss_residual(
      electric,density(L,segments,false,polarity_scale));
  cpu_diagnostics.gauss_after_residual=max_fractional_gauss_residual(
      cpu_after,density(L,segments,true,polarity_scale));
  cpu_diagnostics.field_energy_before=interaction_scale*cpu_profile.energy_before;
  cpu_diagnostics.field_energy_after=interaction_scale*cpu_profile.energy_after;
  cpu_diagnostics.local_field_momentum_before=
      matched_local_translation_momentum(electric,magnetic)*interaction_scale;
  cpu_diagnostics.local_field_momentum_after=
      matched_local_translation_momentum(cpu_after,cpu_magnetic_after)
      *interaction_scale;
  cpu_diagnostics.spline_field_momentum_before=measure_spline_poynting_momentum(
      electric,magnetic,ftd::C_SPEED,0.25,interaction_scale).momentum;
  cpu_diagnostics.spline_field_momentum_after=measure_spline_poynting_momentum(
      cpu_after,cpu_magnetic_after,ftd::C_SPEED,0.25,interaction_scale).momentum;
  const double diagnostic_residual=std::max({
      std::abs(cpu_diagnostics.gauss_before_residual
          -gpu_diagnostics.gauss_before_residual),
      std::abs(cpu_diagnostics.gauss_after_residual
          -gpu_diagnostics.gauss_after_residual),
      std::abs(cpu_diagnostics.field_energy_before
          -gpu_diagnostics.field_energy_before),
      std::abs(cpu_diagnostics.field_energy_after
          -gpu_diagnostics.field_energy_after),
      vector_difference(cpu_diagnostics.local_field_momentum_before,
          gpu_diagnostics.local_field_momentum_before),
      vector_difference(cpu_diagnostics.local_field_momentum_after,
          gpu_diagnostics.local_field_momentum_after),
      vector_difference(cpu_diagnostics.spline_field_momentum_before,
          gpu_diagnostics.spline_field_momentum_before),
      vector_difference(cpu_diagnostics.spline_field_momentum_after,
          gpu_diagnostics.spline_field_momentum_after)});

  const auto initialized=initialize_connected_moore_block(
      L,2,0,0,0.125,1e-13,4096);
  if(!initialized.valid) return 1;
  auto transaction_initial=initialized.state;
  for(auto& point:transaction_initial.constituents)
    point.momentum={0.002,-0.001,0.0015};
  ConnectedMooreBlockOptions cpu_options;
  cpu_options.allow_shared_anchor_chart=true;
  cpu_options.use_sparse_local_current=true;
  cpu_options.use_local_residual_evaluation=true;
  const auto cpu_step=solve_connected_moore_block_forward(
      transaction_initial,cpu_options);
  auto accelerated_options=cpu_options;
  accelerated_options.defer_volume_diagnostics=true;
  CudaMatchedFieldPipeline transaction_gpu(L);
  if(!transaction_gpu.valid()
      ||!transaction_gpu.upload(transaction_initial.electric,
          transaction_initial.magnetic_half)
      ||!transaction_gpu.prepare_forward(ftd::C_SPEED)) return 1;
  MatchedEdgeField transaction_magnetic;
  MatchedFaceFlux transaction_pre;
  if(!transaction_gpu.download_prepared(transaction_magnetic,transaction_pre))
    return 1;
  auto accelerated_step=solve_connected_moore_block_forward_prepared(
      transaction_initial,std::move(transaction_magnetic),
      std::move(transaction_pre),
      accelerated_options);
  if(!accelerated_step.volume_diagnostics_pending
      ||!transaction_gpu.apply_sparse_current(
          accelerated_step.segments,accelerated_options.polarity_scale))
    return 1;
  const auto transaction_profile=transaction_gpu.observe(
      ftd::C_SPEED,{8.0,8.0,8.0},{2,4,6},1e-10);
  const auto transaction_diagnostics=transaction_gpu.diagnose_common_action(
      accelerated_step.segments,accelerated_options.polarity_scale,
      accelerated_step.interaction_scale,accelerated_options.wave_speed,
      accelerated_options.dt,1e-10);
  accelerated_step=complete_connected_moore_block_volume_diagnostics(
      std::move(accelerated_step),transaction_diagnostics,
      accelerated_options);
  const double transaction_state_difference=
      connected_moore_block_state_max_difference(
          cpu_step.later,accelerated_step.later);
  const double transaction_diagnostic_difference=std::max({
      std::abs(cpu_step.gauss_before_residual
          -accelerated_step.gauss_before_residual),
      std::abs(cpu_step.gauss_after_residual
          -accelerated_step.gauss_after_residual),
      std::abs(cpu_step.field_energy_before
          -accelerated_step.field_energy_before),
      std::abs(cpu_step.field_energy_after
          -accelerated_step.field_energy_after),
      std::abs(cpu_step.total_energy_residual
          -accelerated_step.total_energy_residual),
      vector_difference(cpu_step.local_field_momentum_before,
          accelerated_step.local_field_momentum_before),
      vector_difference(cpu_step.local_field_momentum_after,
          accelerated_step.local_field_momentum_after),
      vector_difference(cpu_step.spline_field_momentum_before,
          accelerated_step.spline_field_momentum_before),
      vector_difference(cpu_step.spline_field_momentum_after,
          accelerated_step.spline_field_momentum_after)});
  const bool pass = cpu_profile.valid && gpu_profile.valid
      && gpu_diagnostics.valid
      && repeat_profile.valid && repeat_diagnostics.valid
      && cpu_step.valid && cpu_step.common_action_gates_pass
      && transaction_profile.valid && transaction_diagnostics.valid
      && accelerated_step.valid && accelerated_step.common_action_gates_pass
      && magnetic_difference <= 2e-15 && pre_difference <= 2e-15
      && after_difference <= 2e-15 && profile_residual <= 2e-10
      && diagnostic_residual <= 2e-10
      && determinism_residual <= 1e-14
      && transaction_state_difference <= 2e-10
      && transaction_diagnostic_difference <= 2e-10;
  const auto& timing = gpu.timings();
  std::cout << std::setprecision(17)
      << "magnetic_difference=" << magnetic_difference
      << " pre_difference=" << pre_difference
      << " after_difference=" << after_difference
      << " profile_difference=" << profile_residual
      << " diagnostic_difference=" << diagnostic_residual
      << " determinism_difference=" << determinism_residual
      << " transaction_state_difference=" << transaction_state_difference
      << " transaction_diagnostic_difference="
      << transaction_diagnostic_difference
      << " upload_ms=" << timing.upload_ms
      << " prepare_ms=" << timing.prepare_ms
      << " current_ms=" << timing.current_ms
      << " observe_ms=" << timing.observe_ms
      << " download_ms=" << timing.download_ms << '\n';
  return pass ? 0 : 1;
}

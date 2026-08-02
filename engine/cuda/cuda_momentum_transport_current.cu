#include "ftd/eft/cuda_momentum_transport_current.h"

#include <cuda_runtime.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

// Ten reduced quantities per (localization, component, radius slot).  The
// ordering is frozen with the kernel and mirrored by drain_slot() below.
constexpr int kQuantityPhiPlain = 0;
constexpr int kQuantityPhiBinding = 1;
constexpr int kQuantityPhiPlainComplement = 2;
constexpr int kQuantityPhiBindingComplement = 3;
constexpr int kQuantitySweep = 4;
constexpr int kQuantitySweepComplement = 5;
constexpr int kQuantitySource = 6;
constexpr int kQuantityContentAfter = 7;
constexpr int kQuantityContentBefore = 8;
constexpr int kQuantityContentOld = 9;
constexpr int kQuantities = 10;
constexpr int kReducedQuantities = kQuantities * kCudaMomentumSlots;  // 60

constexpr int kThreads = 128;
constexpr int kMaximumBlocks = 512;

// Chord-field selectors.  The plain-D_i flux rides a derived field (u = C B'
// for L1, w = C^T E for L2); the binding flux rides a resident field
// (E for L1, B' for L2).
constexpr int kChordFieldElectricBefore = 0;   // E
constexpr int kChordFieldMagneticAfter = 1;    // B'
constexpr int kChordFieldCurlMagneticAfter = 2;      // u = C B'
constexpr int kChordFieldCurlAdjointElectric = 3;    // w = C^T E

struct DeviceTriplet {
  const double* x = nullptr;
  const double* y = nullptr;
  const double* z = nullptr;
};

/// One generator of T^(i)_{a,d}(v) (Sec 2.3), device layout.
struct DeviceBond {
  int axis;
  int base_x, base_y, base_z;
  int r_x, r_y, r_z;
  int a, b;
  double weight;
};

struct DeviceBondRange {
  int begin[3]{0, 0, 0};
  int end[3]{0, 0, 0};
};

struct DeviceRadii {
  int value[kCudaMomentumMaximumRadii]{-1, -1, -1, -1, -1};
};

double milliseconds_since(const std::chrono::steady_clock::time_point& start) {
  return std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - start).count();
}

DeviceTriplet view(const CudaMatchedFieldDeviceView& field) {
  return {field.x, field.y, field.z};
}

__device__ int wrap_coordinate(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

__device__ std::size_t index_at(int x, int y, int z, int L) {
  return (static_cast<std::size_t>(wrap_coordinate(x, L)) * L
          + wrap_coordinate(y, L)) * L + wrap_coordinate(z, L);
}

__device__ double component_at(DeviceTriplet field, int axis,
                               int x, int y, int z, int L) {
  const auto index = index_at(x, y, z, L);
  return axis == 0 ? field.x[index]
                   : (axis == 1 ? field.y[index] : field.z[index]);
}

/// C: (C B)_a = eps_{abc} d_b^- B_c  (matched_gauss_transport.cpp:183-212).
__device__ double curl_component(DeviceTriplet edge, int axis,
                                 int x, int y, int z, int L) {
  const auto f = [&](int c, int xx, int yy, int zz) {
    return component_at(edge, c, xx, yy, zz, L);
  };
  if (axis == 0)
    return f(2, x, y, z) - f(2, x, y - 1, z) - f(1, x, y, z) + f(1, x, y, z - 1);
  if (axis == 1)
    return f(0, x, y, z) - f(0, x, y, z - 1) - f(2, x, y, z) + f(2, x - 1, y, z);
  return f(1, x, y, z) - f(1, x - 1, y, z) - f(0, x, y, z) + f(0, x, y - 1, z);
}

/// C^T: (C^T E)_a = eps_{abc} d_b^+ E_c  (matched_gauss_transport.cpp:214-243).
__device__ double curl_adjoint_component(DeviceTriplet face, int axis,
                                         int x, int y, int z, int L) {
  const auto f = [&](int c, int xx, int yy, int zz) {
    return component_at(face, c, xx, yy, zz, L);
  };
  if (axis == 0)
    return f(2, x, y + 1, z) - f(2, x, y, z) - f(1, x, y, z + 1) + f(1, x, y, z);
  if (axis == 1)
    return f(0, x, y, z + 1) - f(0, x, y, z) - f(2, x + 1, y, z) + f(2, x, y, z);
  return f(1, x + 1, y, z) - f(1, x, y, z) - f(0, x, y + 1, z) + f(0, x, y, z);
}

__device__ double chord_field(int kind, DeviceTriplet electric_before,
                              DeviceTriplet magnetic_after, int a,
                              int x, int y, int z, int L) {
  if (kind == kChordFieldElectricBefore)
    return component_at(electric_before, a, x, y, z, L);
  if (kind == kChordFieldMagneticAfter)
    return component_at(magnetic_after, a, x, y, z, L);
  if (kind == kChordFieldCurlMagneticAfter)
    return curl_component(magnetic_after, a, x, y, z, L);
  return curl_adjoint_component(electric_before, a, x, y, z, L);
}

/// Sum_a T^(i)_{a,d}(v) on the unit bond (v, v+e_d).  Valid to aggregate over
/// the component index precisely because the production mask is
/// component-independent (Sec 2.3 site-mask collapse).
__device__ double bond_current(const DeviceBond* bonds, DeviceBondRange range,
                               int kind, DeviceTriplet electric_before,
                               DeviceTriplet magnetic_after, int d,
                               int x, int y, int z, int L) {
  double total = 0.0;
  for (int entry = range.begin[d]; entry < range.end[d]; ++entry) {
    const DeviceBond bond = bonds[entry];
    const int bx = x - bond.base_x;
    const int by = y - bond.base_y;
    const int bz = z - bond.base_z;
    const double left = chord_field(kind, electric_before, magnetic_after,
                                    bond.a, bx, by, bz, L);
    const double right = chord_field(kind, electric_before, magnetic_after,
                                     bond.b, bx + bond.r_x, by + bond.r_y,
                                     bz + bond.r_z, L);
    total += bond.weight * left * right;
  }
  return total;
}

__device__ int absolute_offset(int value, int center, int L) {
  int delta = (value - center) % L;
  if (delta < 0) delta += L;
  if (delta > L / 2) delta -= L;
  return delta < 0 ? -delta : delta;
}

__device__ bool slot_inside(int slot, DeviceRadii radii, int cx, int cy,
                            int cz, int x, int y, int z, int L) {
  if (slot == kCudaMomentumWholeDomainSlot) return true;
  const int radius = radii.value[slot];
  if (radius < 0) return false;  // unused slot
  const int dx = absolute_offset(x, cx, L);
  const int dy = absolute_offset(y, cy, L);
  const int dz = absolute_offset(z, cz, L);
  const int largest = dx > dy ? (dx > dz ? dx : dz) : (dy > dz ? dy : dz);
  return largest <= radius;
}

__global__ void momentum_ledger_kernel(
    DeviceTriplet electric_before, DeviceTriplet magnetic_before,
    DeviceTriplet magnetic_after, DeviceTriplet electric_pre_current,
    DeviceTriplet electric_after, std::size_t count, int L, int component,
    int localization, int previous_x, int previous_y, int previous_z,
    int current_x, int current_y, int current_z, DeviceRadii radii,
    const DeviceBond* plain_bonds, DeviceBondRange plain_range,
    int plain_field_kind, const DeviceBond* binding_bonds,
    DeviceBondRange binding_range, int binding_field_kind, double* partial) {
  extern __shared__ double shared[];
  double values[kReducedQuantities];
  for (int q = 0; q < kReducedQuantities; ++q) values[q] = 0.0;

  const int step_x = component == 0 ? 1 : 0;
  const int step_y = component == 1 ? 1 : 0;
  const int step_z = component == 2 ? 1 : 0;
  const std::size_t plane = static_cast<std::size_t>(L) * L;
  const std::size_t first =
      static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
  const std::size_t stride =
      static_cast<std::size_t>(gridDim.x) * blockDim.x;
  for (std::size_t linear = first; linear < count; linear += stride) {
    const int x = static_cast<int>(linear / plane);
    const std::size_t remainder = linear - static_cast<std::size_t>(x) * plane;
    const int y = static_cast<int>(remainder / L);
    const int z = static_cast<int>(remainder
                                   - static_cast<std::size_t>(y) * L);

    // ---- densities pi^t, pi^{t+1} and the Q density at this site ----------
    double density_before = 0.0;
    double density_after = 0.0;
    double density_source = 0.0;
    if (localization == 0) {
      // L1: pi^(1) = E.(D_i C B) before, E''.(D_i C B') after,
      //     Q density = K.(D_i C B'), K = E'-E''.
      for (int a = 0; a < 3; ++a) {
        const double curl_before_plus = curl_component(
            magnetic_before, a, x + step_x, y + step_y, z + step_z, L);
        const double curl_before_minus = curl_component(
            magnetic_before, a, x - step_x, y - step_y, z - step_z, L);
        const double curl_after_plus = curl_component(
            magnetic_after, a, x + step_x, y + step_y, z + step_z, L);
        const double curl_after_minus = curl_component(
            magnetic_after, a, x - step_x, y - step_y, z - step_z, L);
        const double drive_before = 0.5 * (curl_before_plus - curl_before_minus);
        const double drive_after = 0.5 * (curl_after_plus - curl_after_minus);
        const double e_before = component_at(electric_before, a, x, y, z, L);
        const double e_after = component_at(electric_after, a, x, y, z, L);
        const double current =
            component_at(electric_pre_current, a, x, y, z, L) - e_after;
        density_before += e_before * drive_before;
        density_after += e_after * drive_after;
        density_source += current * drive_after;
      }
    } else {
      // L2: pi^(2) = -B.(D_i C^T E) before, -B'.(D_i C^T E'') after,
      //     Q density = -B'.(D_i C^T K).
      for (int a = 0; a < 3; ++a) {
        const double w_plus = curl_adjoint_component(
            electric_before, a, x + step_x, y + step_y, z + step_z, L);
        const double w_minus = curl_adjoint_component(
            electric_before, a, x - step_x, y - step_y, z - step_z, L);
        const double w_after_plus = curl_adjoint_component(
            electric_after, a, x + step_x, y + step_y, z + step_z, L);
        const double w_after_minus = curl_adjoint_component(
            electric_after, a, x - step_x, y - step_y, z - step_z, L);
        const double w_pre_plus = curl_adjoint_component(
            electric_pre_current, a, x + step_x, y + step_y, z + step_z, L);
        const double w_pre_minus = curl_adjoint_component(
            electric_pre_current, a, x - step_x, y - step_y, z - step_z, L);
        const double drive_before = 0.5 * (w_plus - w_minus);
        const double drive_after = 0.5 * (w_after_plus - w_after_minus);
        const double drive_source =
            0.5 * ((w_pre_plus - w_after_plus) - (w_pre_minus - w_after_minus));
        const double b_before = component_at(magnetic_before, a, x, y, z, L);
        const double b_after = component_at(magnetic_after, a, x, y, z, L);
        density_before -= b_before * drive_before;
        density_after -= b_after * drive_after;
        density_source -= b_after * drive_source;
      }
    }

    bool current_in[kCudaMomentumSlots];
    bool previous_in[kCudaMomentumSlots];
    for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
      current_in[slot] = slot_inside(slot, radii, current_x, current_y,
                                     current_z, x, y, z, L);
      previous_in[slot] = slot_inside(slot, radii, previous_x, previous_y,
                                      previous_z, x, y, z, L);
      const int base = slot * kQuantities;
      if (current_in[slot]) {
        values[base + kQuantityContentAfter] += density_after;
        values[base + kQuantityContentBefore] += density_before;
        values[base + kQuantitySource] += density_source;
      }
      if (previous_in[slot])
        values[base + kQuantityContentOld] += density_before;
      const int sweep = static_cast<int>(current_in[slot])
          - static_cast<int>(previous_in[slot]);
      if (sweep != 0)
        values[base + kQuantitySweep] += sweep * density_before;
      const int sweep_complement = static_cast<int>(!current_in[slot])
          - static_cast<int>(!previous_in[slot]);
      if (sweep_complement != 0)
        values[base + kQuantitySweepComplement] +=
            sweep_complement * density_before;
    }

    // ---- unit-bond fluxes on chords straddling dOmega (Sec 2.3) -----------
    for (int d = 0; d < 3; ++d) {
      const int nx = d == 0 ? x + 1 : x;
      const int ny = d == 1 ? y + 1 : y;
      const int nz = d == 2 ? z + 1 : z;
      bool neighbour_in[kCudaMomentumSlots];
      bool straddles = false;
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
        neighbour_in[slot] = slot_inside(slot, radii, current_x, current_y,
                                         current_z, nx, ny, nz, L);
        if (neighbour_in[slot] != current_in[slot]) straddles = true;
      }
      if (!straddles) continue;
      const double plain = bond_current(plain_bonds, plain_range,
          plain_field_kind, electric_before, magnetic_after, d, x, y, z, L);
      const double binding = bond_current(binding_bonds, binding_range,
          binding_field_kind, electric_before, magnetic_after, d, x, y, z, L);
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
        const int base = slot * kQuantities;
        const int factor = static_cast<int>(current_in[slot])
            - static_cast<int>(neighbour_in[slot]);
        if (factor != 0) {
          values[base + kQuantityPhiPlain] += plain * factor;
          values[base + kQuantityPhiBinding] += binding * factor;
        }
        const int complement = static_cast<int>(!current_in[slot])
            - static_cast<int>(!neighbour_in[slot]);
        if (complement != 0) {
          values[base + kQuantityPhiPlainComplement] += plain * complement;
          values[base + kQuantityPhiBindingComplement] += binding * complement;
        }
      }
    }
  }

  for (int q = 0; q < kReducedQuantities; ++q) {
    shared[threadIdx.x] = values[q];
    __syncthreads();
    for (int stride_size = blockDim.x / 2; stride_size > 0; stride_size >>= 1) {
      if (threadIdx.x < stride_size)
        shared[threadIdx.x] += shared[threadIdx.x + stride_size];
      __syncthreads();
    }
    if (threadIdx.x == 0)
      partial[static_cast<std::size_t>(q) * gridDim.x + blockIdx.x] = shared[0];
    __syncthreads();
  }
}

void fail(CudaMomentumTransportTelemetry& telemetry, const char* message) {
  telemetry.valid = false;
  telemetry.error = message;
}

}  // namespace

// ---------------------------------------------------------------------------

struct CudaMomentumTransportLedger::Impl {
  int L = 0;
  std::size_t count = 0;
  int blocks = 0;
  bool ready = false;
  std::string error;
  MomentumTransportCurrentTable plain[3];
  MomentumTransportCurrentTable binding[2][3];
  DeviceBondRange plain_range[3];
  DeviceBondRange binding_range[2][3];
  std::size_t plain_offset[3]{};
  std::size_t binding_offset[2][3]{};
  DeviceBond* bonds = nullptr;
  double* partial = nullptr;
  std::size_t uploaded_bytes = 0;

  ~Impl() {
    cudaFree(bonds);
    cudaFree(partial);
  }
};

namespace {

/// Sort one table's bond generators by axis and record the per-axis range.
void append_bonds(const MomentumTransportCurrentTable& table,
                  std::vector<DeviceBond>& out, DeviceBondRange& range,
                  std::size_t& offset) {
  offset = out.size();
  for (int axis = 0; axis < 3; ++axis) {
    range.begin[axis] = static_cast<int>(out.size());
    for (const auto& generator : table.bond) {
      if (generator.axis != axis) continue;
      DeviceBond bond{};
      bond.axis = generator.axis;
      bond.base_x = generator.base[0];
      bond.base_y = generator.base[1];
      bond.base_z = generator.base[2];
      bond.r_x = generator.r[0];
      bond.r_y = generator.r[1];
      bond.r_z = generator.r[2];
      bond.a = generator.a;
      bond.b = generator.b;
      bond.weight = generator.weight;
      out.push_back(bond);
    }
    range.end[axis] = static_cast<int>(out.size());
  }
}

}  // namespace

CudaMomentumTransportLedger::CudaMomentumTransportLedger(int L)
    : impl_(std::make_unique<Impl>()) {
  impl_->L = L;
  if (L <= 0) {
    impl_->error = "invalid volume";
    return;
  }
  impl_->count = static_cast<std::size_t>(L) * L * L;
  impl_->blocks = std::min<int>(kMaximumBlocks,
      static_cast<int>((impl_->count + kThreads - 1) / kThreads));
  if (impl_->blocks <= 0) {
    impl_->error = "invalid block count";
    return;
  }

  std::vector<DeviceBond> bonds;
  for (int component = 0; component < 3; ++component) {
    impl_->plain[component] = build_momentum_transport_current_table(
        MomentumOperatorKind::CentralDifference, component);
    impl_->binding[0][component] = build_momentum_transport_current_table(
        MomentumOperatorKind::FaceBinding, component);
    impl_->binding[1][component] = build_momentum_transport_current_table(
        MomentumOperatorKind::EdgeBinding, component);
    if (!impl_->plain[component].valid || !impl_->binding[0][component].valid
        || !impl_->binding[1][component].valid) {
      impl_->error = "chord table construction failed";
      return;
    }
  }
  for (int component = 0; component < 3; ++component)
    append_bonds(impl_->plain[component], bonds, impl_->plain_range[component],
                 impl_->plain_offset[component]);
  for (int localization = 0; localization < 2; ++localization)
    for (int component = 0; component < 3; ++component)
      append_bonds(impl_->binding[localization][component], bonds,
                   impl_->binding_range[localization][component],
                   impl_->binding_offset[localization][component]);

  const std::size_t bond_bytes = bonds.size() * sizeof(DeviceBond);
  const std::size_t partial_bytes =
      static_cast<std::size_t>(kReducedQuantities) * impl_->blocks
      * sizeof(double);
  if (cudaMalloc(&impl_->bonds, std::max<std::size_t>(bond_bytes, 64))
          != cudaSuccess
      || cudaMalloc(&impl_->partial, partial_bytes) != cudaSuccess) {
    impl_->error = "momentum ledger allocation failed";
    return;
  }
  if (!bonds.empty()
      && cudaMemcpy(impl_->bonds, bonds.data(), bond_bytes,
                    cudaMemcpyHostToDevice) != cudaSuccess) {
    impl_->error = "momentum chord table upload failed";
    return;
  }
  impl_->uploaded_bytes = bond_bytes;
  impl_->ready = true;
}

CudaMomentumTransportLedger::~CudaMomentumTransportLedger() = default;

bool CudaMomentumTransportLedger::valid() const {
  return impl_ && impl_->ready;
}

const char* CudaMomentumTransportLedger::error() const {
  return impl_ ? impl_->error.c_str() : "no implementation";
}

int CudaMomentumTransportLedger::size() const {
  return impl_ ? impl_->L : 0;
}

const MomentumTransportCurrentTable&
CudaMomentumTransportLedger::plain_table(int component) const {
  return impl_->plain[component];
}

const MomentumTransportCurrentTable&
CudaMomentumTransportLedger::binding_table(MomentumLocalization localization,
                                           int component) const {
  return impl_->binding[localization == MomentumLocalization::ECarries ? 0 : 1]
                       [component];
}

CudaMomentumLedgerTick CudaMomentumTransportLedger::observe(
    const CudaMatchedFieldResidentViews& views,
    const CudaMomentumLedgerOptions& options,
    CudaMomentumTransportTelemetry* telemetry_out) {
  CudaMomentumLedgerTick result;
  CudaMomentumTransportTelemetry telemetry;
  telemetry.host_to_device_bytes = impl_ ? impl_->uploaded_bytes : 0;
  if (!valid() || !views.prepared || !views.current_applied
      || !views.electric_before.valid() || !views.magnetic_before.valid()
      || !views.magnetic_prepared.valid()
      || !views.electric_pre_current.valid()
      || !views.electric_after.valid()
      || views.electric_before.L != impl_->L
      || views.magnetic_before.L != impl_->L
      || views.magnetic_prepared.L != impl_->L
      || views.electric_pre_current.L != impl_->L
      || views.electric_after.L != impl_->L
      || !(options.lambda > 0.0) || !std::isfinite(options.lambda)
      || !std::isfinite(options.interaction_scale)) {
    fail(telemetry, "invalid momentum-ledger input");
    if (telemetry_out) *telemetry_out = telemetry;
    return result;
  }

  DeviceRadii radii;
  for (int slot = 0; slot < kCudaMomentumMaximumRadii; ++slot)
    radii.value[slot] = options.radius[slot];

  const int blocks = impl_->blocks;
  std::vector<double> host(static_cast<std::size_t>(kReducedQuantities)
                           * blocks);
  const std::size_t download_bytes = host.size() * sizeof(double);
  const auto e0 = view(views.electric_before);
  const auto b0 = view(views.magnetic_before);
  const auto b1 = view(views.magnetic_prepared);
  const auto epre = view(views.electric_pre_current);
  const auto e1 = view(views.electric_after);

  for (int localization = 0; localization < 2; ++localization) {
    const int plain_kind = localization == 0 ? kChordFieldCurlMagneticAfter
                                             : kChordFieldCurlAdjointElectric;
    const int binding_kind = localization == 0 ? kChordFieldElectricBefore
                                               : kChordFieldMagneticAfter;
    for (int component = 0; component < 3; ++component) {
      const auto kernel_start = std::chrono::steady_clock::now();
      momentum_ledger_kernel<<<blocks, kThreads, kThreads * sizeof(double)>>>(
          e0, b0, b1, epre, e1, impl_->count, impl_->L, component,
          localization, options.previous_center[0], options.previous_center[1],
          options.previous_center[2], options.current_center[0],
          options.current_center[1], options.current_center[2], radii,
          // DeviceBondRange holds absolute indices into the single uploaded
          // generator array, so every launch receives the same base pointer.
          impl_->bonds, impl_->plain_range[component], plain_kind,
          impl_->bonds, impl_->binding_range[localization][component],
          binding_kind, impl_->partial);
      if (cudaGetLastError() != cudaSuccess
          || cudaDeviceSynchronize() != cudaSuccess) {
        fail(telemetry, "momentum-ledger kernel failed");
        if (telemetry_out) *telemetry_out = telemetry;
        return result;
      }
      telemetry.kernel_ms += milliseconds_since(kernel_start);
      ++telemetry.kernel_launches;
      if (cudaMemcpy(host.data(), impl_->partial, download_bytes,
                     cudaMemcpyDeviceToHost) != cudaSuccess) {
        fail(telemetry, "momentum-ledger reduction download failed");
        if (telemetry_out) *telemetry_out = telemetry;
        return result;
      }
      telemetry.device_to_host_bytes += download_bytes;

      const auto reduce = [&](int quantity) {
        long double total = 0.0L;
        const std::size_t base =
            static_cast<std::size_t>(quantity) * blocks;
        for (int block = 0; block < blocks; ++block)
          total += host[base + static_cast<std::size_t>(block)];
        return static_cast<double>(total);
      };
      for (int slot = 0; slot < kCudaMomentumSlots; ++slot) {
        auto& terms = result.terms[localization][component][slot];
        const int base = slot * kQuantities;
        terms.phi_plain = reduce(base + kQuantityPhiPlain);
        terms.phi_binding = reduce(base + kQuantityPhiBinding);
        terms.phi_plain_complement =
            reduce(base + kQuantityPhiPlainComplement);
        terms.phi_binding_complement =
            reduce(base + kQuantityPhiBindingComplement);
        terms.sweep = reduce(base + kQuantitySweep);
        terms.sweep_complement = reduce(base + kQuantitySweepComplement);
        terms.source = reduce(base + kQuantitySource);
        terms.content_after = reduce(base + kQuantityContentAfter);
        terms.content_before = reduce(base + kQuantityContentBefore);
        terms.content_old = reduce(base + kQuantityContentOld);
        // Sec 3: every momentum-sector quantity carries interaction_scale,
        // applied at the same point the existing local_momentum lambda does.
        scale_momentum_ledger_tick_terms(terms, options.interaction_scale);
        const bool unused = slot < kCudaMomentumMaximumRadii
            && options.radius[slot] < 0;
        terms.valid = !unused && std::isfinite(terms.phi_plain)
            && std::isfinite(terms.phi_binding)
            && std::isfinite(terms.phi_plain_complement)
            && std::isfinite(terms.phi_binding_complement)
            && std::isfinite(terms.sweep)
            && std::isfinite(terms.sweep_complement)
            && std::isfinite(terms.source)
            && std::isfinite(terms.content_after)
            && std::isfinite(terms.content_before)
            && std::isfinite(terms.content_old);
      }
    }
  }
  result.valid = true;
  telemetry.valid = true;
  if (telemetry_out) *telemetry_out = telemetry;
  return result;
}

bool cuda_momentum_transport_ledger_available() {
  int devices = 0;
  return cudaGetDeviceCount(&devices) == cudaSuccess && devices > 0;
}

}  // namespace ftd::eft

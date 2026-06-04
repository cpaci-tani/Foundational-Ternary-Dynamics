/**
 * @file benchmark_field_soa_cpu.cpp
 * @brief Non-physics timing probe for CPU FieldSoA read paths.
 *
 * Usage:
 *   benchmark_field_soa_cpu --L=64 --iters=5 --mode=all --stride=2
 */

#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct Args {
  int L = 64;
  int iters = 5;
  int stride = 2;
  std::string mode = "all";
};

Args parse(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    const char* s = argv[i];
    auto eat_int = [&](const char* key, int& dst) {
      const std::size_t n = std::strlen(key);
      if (std::strncmp(s, key, n) == 0 && s[n] == '=') {
        dst = std::atoi(s + n + 1);
        return true;
      }
      return false;
    };
    if (eat_int("--L", a.L)) continue;
    if (eat_int("--iters", a.iters)) continue;
    if (eat_int("--stride", a.stride)) continue;
    constexpr const char* mode_key = "--mode=";
    constexpr std::size_t mode_len = 7;
    if (std::strncmp(s, mode_key, mode_len) == 0) a.mode = s + mode_len;
  }
  if (a.L < 4) a.L = 4;
  if (a.iters < 1) a.iters = 1;
  if (a.stride < 1) a.stride = 1;
  return a;
}

void seed_fields(ftd::RenderBridge& rb) {
  rb.force_cpu();
  auto& voxels = rb.voxels();
  const int total = static_cast<int>(voxels.size());
  for (int i = 0; i < total; ++i) {
    const double a = static_cast<double>((i % 17) - 8);
    const double b = static_cast<double>((i % 13) - 6);
    const double c = static_cast<double>((i % 11) - 5);
    voxels[i].flux = {0.011 * a, -0.017 * b, 0.023 * c};
    voxels[i].wave_vel = {-0.007 * c, 0.005 * a, -0.003 * b};
  }
  (void)rb.fields();
}

template <typename Fn>
double seconds_for(Fn&& fn, double& checksum) {
  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();
  checksum = fn();
  const auto t1 = clock::now();
  return std::chrono::duration<double>(t1 - t0).count();
}

struct Row {
  std::string mode;
  double aos_seconds = 0.0;
  double soa_seconds = 0.0;
  double aos_checksum = 0.0;
  double soa_checksum = 0.0;
};

Row bench_flux_volume(ftd::RenderBridge& rb, int iters) {
  Row r;
  r.mode = "flux-volume";
  const auto& voxels = rb.voxels();
  const auto& fields = rb.fields();
  const int N = rb.lattice().size();
  r.aos_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
          for (int x = 0; x < N; ++x)
            sum += voxels[rb.lattice().index(x, y, z)].density();
    }
    return sum;
  }, r.aos_checksum);
  r.soa_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      for (int z = 0; z < N; ++z)
        for (int y = 0; y < N; ++y)
          for (int x = 0; x < N; ++x)
            sum += fields.density_at(static_cast<std::size_t>(rb.lattice().index(x, y, z)));
    }
    return sum;
  }, r.soa_checksum);
  return r;
}

Row bench_flux_slice(ftd::RenderBridge& rb, int iters) {
  Row r;
  r.mode = "flux-slice";
  const auto& voxels = rb.voxels();
  const auto& fields = rb.fields();
  const int N = rb.lattice().size();
  const int z = N / 2;
  r.aos_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
          sum += voxels[rb.lattice().index(x, y, z)].density();
    return sum;
  }, r.aos_checksum);
  r.soa_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
          sum += fields.density_at(static_cast<std::size_t>(rb.lattice().index(x, y, z)));
    return sum;
  }, r.soa_checksum);
  return r;
}

Row bench_flux_vector_sampled(ftd::RenderBridge& rb, int iters, int stride) {
  Row r;
  r.mode = "flux-vector-sampled";
  const auto& voxels = rb.voxels();
  const auto& fields = rb.fields();
  const int N = rb.lattice().size();
  r.aos_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int z = 0; z < N; z += stride)
        for (int y = 0; y < N; y += stride)
          for (int x = 0; x < N; x += stride) {
            const auto& f = voxels[rb.lattice().index(x, y, z)].flux;
            sum += f.x + f.y + f.z;
          }
    return sum;
  }, r.aos_checksum);
  r.soa_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int z = 0; z < N; z += stride)
        for (int y = 0; y < N; y += stride)
          for (int x = 0; x < N; x += stride) {
            const int idx = rb.lattice().index(x, y, z);
            sum += fields.flux_x[idx] + fields.flux_y[idx] + fields.flux_z[idx];
          }
    return sum;
  }, r.soa_checksum);
  return r;
}

Row bench_div_curl(ftd::RenderBridge& rb, int iters, int stride) {
  Row r;
  r.mode = "div-curl";
  const auto& voxels = rb.voxels();
  const auto& fields = rb.fields();
  const auto& lattice = rb.lattice();
  const int N = lattice.size();
  r.aos_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int z = 0; z < N; z += stride)
        for (int y = 0; y < N; y += stride)
          for (int x = 0; x < N; x += stride) {
            const int idx = lattice.index(x, y, z);
            const auto c = ::ftd::curl_flux_op(voxels, lattice, idx);
            sum += ::ftd::divergence_flux_op(voxels, lattice, idx) + c.x + c.y + c.z;
          }
    return sum;
  }, r.aos_checksum);
  r.soa_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it)
      for (int z = 0; z < N; z += stride)
        for (int y = 0; y < N; y += stride)
          for (int x = 0; x < N; x += stride) {
            const int idx = lattice.index(x, y, z);
            const auto c = ::ftd::curl_flux_op(fields, lattice, idx);
            sum += ::ftd::divergence_flux_op(fields, lattice, idx) + c.x + c.y + c.z;
          }
    return sum;
  }, r.soa_checksum);
  return r;
}

Row bench_sync(int L, int iters) {
  Row r;
  r.mode = "sync";
  ftd::RenderBridge rb(L);
  seed_fields(rb);
  r.soa_seconds = seconds_for([&] {
    double sum = 0.0;
    for (int it = 0; it < iters; ++it) {
      rb.voxels()[0].flux.x += 1e-12;
      sum += rb.fields().flux_x[0];
    }
    return sum;
  }, r.soa_checksum);
  return r;
}

void print_row(const Row& r, int L, int iters, int stride, bool comma) {
  const double speedup = (r.soa_seconds > 0.0 && r.aos_seconds > 0.0)
      ? (r.aos_seconds / r.soa_seconds)
      : 0.0;
  std::printf(
      "%s{\"mode\":\"%s\",\"L\":%d,\"iters\":%d,\"stride\":%d,"
      "\"aos_seconds\":%.9g,\"soa_seconds\":%.9g,\"speedup\":%.9g,"
      "\"aos_checksum\":%.17g,\"soa_checksum\":%.17g}",
      comma ? "," : "", r.mode.c_str(), L, iters, stride,
      r.aos_seconds, r.soa_seconds, speedup,
      r.aos_checksum, r.soa_checksum);
}

}  // namespace

int main(int argc, char** argv) {
  const Args args = parse(argc, argv);
  std::vector<Row> rows;

  if (args.mode == "all" || args.mode == "sync") {
    rows.push_back(bench_sync(args.L, args.iters));
  }

  if (args.mode != "sync") {
    ftd::RenderBridge rb(args.L);
    seed_fields(rb);
    if (args.mode == "all" || args.mode == "flux-volume")
      rows.push_back(bench_flux_volume(rb, args.iters));
    if (args.mode == "all" || args.mode == "flux-slice")
      rows.push_back(bench_flux_slice(rb, args.iters));
    if (args.mode == "all" || args.mode == "flux-vector-sampled")
      rows.push_back(bench_flux_vector_sampled(rb, args.iters, args.stride));
    if (args.mode == "all" || args.mode == "div-curl")
      rows.push_back(bench_div_curl(rb, args.iters, args.stride));
  }

  std::printf("[");
  for (std::size_t i = 0; i < rows.size(); ++i) {
    print_row(rows[i], args.L, args.iters, args.stride, i != 0);
  }
  std::printf("]\n");
  return rows.empty() ? 1 : 0;
}

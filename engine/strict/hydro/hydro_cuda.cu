// CUDA kernels for phi-hydro-staged-candidate-1 (research candidate only). Physical schedule
// is four CUDA launches, one launch per elapsed global microtick; host extraction only
// observes events from the actual before/after device states (decoded on the host) -- it
// never recomputes or repairs the next state.
//
// Deltas from engine/strict/staged_cuda.cu (the Phi-v2 template this was copied from):
//  - hydro's byte layout has no `ell` and splits admitted/gate into separate sc/fcc arrays;
//    the View offsets below are exactly task-9-brief.md's (units of n = L^3 sites).
//  - admission is gather-style from the inverse SOURCE_SC/SOURCE_FCC tables (one thread per
//    owner site scans its relations' source channels), not the forward TANGENT scan v2 uses,
//    because hydro's law admits into both SC and FCC relations (v2 only has SC absorption).
//  - the collision table is the full 2^24-entry inverse permutation (loaded from the same blob
//    hydro_runtime.cpp verifies), a plain device buffer uploaded once per advance() call --
//    not v2's compact per-`ell` constant-memory table (hydro has no `ell` collision layer).
//  - the small read-only lookup tables (velocities, A9 rotate/phase/eps/blank/absorbed,
//    inverse sources) go in real CUDA __constant__ memory (a few hundred bytes, well under
//    the 64 KiB budget) instead of a plain device buffer passed by pointer.
#include "hydro_cuda.h"
#include "hydro_runtime.h"
#include "hydro_tables.h"
#include "../frozen_tables.h"
#include <cuda_runtime.h>
#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ftd::hydro::gpu {
namespace frozen = ::ftd::strict::frozen;
namespace {

void check(cudaError_t result, const char* operation) {
    if (result != cudaSuccess)
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(result));
}

struct DeviceBuffer {
    void* pointer = nullptr;
    explicit DeviceBuffer(std::size_t size) { check(cudaMalloc(&pointer, size), "cudaMalloc"); }
    ~DeviceBuffer() { if (pointer) cudaFree(pointer); }
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
};

// Small read-only lookup tables in CUDA constant memory. The 2^24-entry collision table is a
// plain device buffer instead (64 MiB, far past the constant-memory budget) passed by pointer.
struct Tables {
    std::int8_t velocity[24][3];
    std::int8_t source_sc[3][4][4];     // [axis][source][{ox,oy,oz,v}]
    std::int8_t source_fcc[3][2][2][4]; // [plane][q][source][{ox,oy,oz,v}]
    std::uint8_t rotate[9];
    std::int8_t phase[9];
    std::int8_t eps[9];
    std::uint8_t blank;
    std::uint8_t absorbed[2];
};
__constant__ Tables g_tables;

Tables host_tables() {
    Tables t{};
    for (unsigned v = 0; v < 24; ++v)
        for (unsigned k = 0; k < 3; ++k) t.velocity[v][k] = tables::VELOCITY[v][k];
    for (unsigned a = 0; a < 3; ++a)
        for (unsigned s = 0; s < 4; ++s)
            for (unsigned k = 0; k < 4; ++k) t.source_sc[a][s][k] = tables::SOURCE_SC[a][s][k];
    for (unsigned p = 0; p < 3; ++p)
        for (unsigned q = 0; q < 2; ++q)
            for (unsigned s = 0; s < 2; ++s)
                for (unsigned k = 0; k < 4; ++k) t.source_fcc[p][q][s][k] = tables::SOURCE_FCC[p][q][s][k];
    std::copy_n(frozen::ROTATE, 9, t.rotate);
    std::copy_n(frozen::PHASE, 9, t.phase);
    std::copy_n(frozen::EPS, 9, t.eps);
    t.blank = frozen::BLANK;
    std::copy_n(frozen::ABSORBED_RESERVE, 2, t.absorbed);
    return t;
}

// Field-major byte layout (offsets in units of n = L^3 sites, per task-9-brief.md):
// s:0, bank:n, sc:193n, fcc:199n, admitted_sc:211n, admitted_fcc:214n, gate_sc:220n,
// gate_fcc:223n (229n total, matching BYTES_PER_SITE).
struct View {
    std::int8_t* s;
    std::uint8_t *bank, *sc, *fcc, *admitted_sc, *admitted_fcc, *gate_sc, *gate_fcc;
    __device__ View(std::uint8_t* bytes, std::size_t n)
        : s(reinterpret_cast<std::int8_t*>(bytes)), bank(bytes + n), sc(bytes + 193 * n),
          fcc(bytes + 199 * n), admitted_sc(bytes + 211 * n), admitted_fcc(bytes + 214 * n),
          gate_sc(bytes + 220 * n), gate_fcc(bytes + 223 * n) {}
};

__host__ __device__ std::size_t site_shift(std::size_t i, unsigned L, unsigned axis, int sign) {
    std::size_t stride = axis == 0 ? std::size_t(L) * L : (axis == 1 ? L : 1);
    unsigned coordinate = unsigned((i / stride) % L);
    if (sign > 0) return coordinate + 1 == L ? i - stride * (L - 1) : i + stride;
    return coordinate == 0 ? i + stride * (L - 1) : i - stride;
}
__device__ std::size_t offset_shift(std::size_t i, unsigned L, const std::int8_t offset[3]) {
    if (offset[0]) i = site_shift(i, L, 0, offset[0]);
    if (offset[1]) i = site_shift(i, L, 1, offset[1]);
    if (offset[2]) i = site_shift(i, L, 2, offset[2]);
    return i;
}
__device__ void plane(unsigned p, unsigned& a, unsigned& b) {
    a = p == 0 ? 1 : 0;
    b = p == 2 ? 1 : 2;
}
__device__ void fcc_endpoints(std::size_t owner, unsigned L, unsigned p, unsigned q,
                               std::size_t& first, std::size_t& second) {
    unsigned a, b; plane(p, a, b);
    if (q == 0) { first = owner; second = site_shift(site_shift(owner, L, a, 1), L, b, 1); }
    else { first = site_shift(owner, L, a, 1); second = site_shift(owner, L, b, 1); }
}
__device__ unsigned population(const std::uint8_t* bank, std::size_t site) {
    unsigned sum = 0;
    for (unsigned c = 0; c < 192; ++c) sum += bank[site * 192 + c];
    return sum;
}
__device__ void relation_device(std::uint8_t l, std::uint8_t r, std::uint8_t& out_l,
                                 std::uint8_t& out_r, bool gate) {
    const bool lo = l != g_tables.blank, ro = r != g_tables.blank;
    const bool phase_zero = lo != ro && g_tables.phase[lo ? l : r] == 0;
    const bool cross = phase_zero && gate;
    out_l = g_tables.rotate[cross ? r : l];
    out_r = g_tables.rotate[cross ? l : r];
}

// One thread per site. For each of the site's 3 SC and 6 FCC relations: compute this
// microtick's gate from population parity, then (if both relation slots are blank) count
// proposals by scanning the inverse source table across both polarities; admit iff exactly
// one. No two threads ever touch the same relation or the same source channel (each phase-2
// channel targets exactly one relation, owned by exactly one site), so no atomics are needed.
__global__ void admission_kernel(std::uint8_t* source, std::uint8_t* dest, std::size_t n, unsigned L) {
    const std::size_t i = std::size_t(blockIdx.x) * blockDim.x + threadIdx.x;
    if (i >= n) return;
    View in(source, n), out(dest, n);
    for (unsigned a = 0; a < 3; ++a) {
        const auto head = site_shift(i, L, a, 1);
        out.gate_sc[i * 3 + a] = std::uint8_t((population(in.bank, i) + population(in.bank, head)) % 2 == 0);
        const auto key = i * 3 + a;
        if (in.sc[key * 2] != g_tables.blank || in.sc[key * 2 + 1] != g_tables.blank) continue;
        unsigned count = 0, sel_c = 0; std::size_t sel_site = 0;
        for (unsigned s = 0; s < 4; ++s) {
            const auto* entry = g_tables.source_sc[a][s];
            const auto site = offset_shift(i, L, entry);
            const unsigned v = unsigned(entry[3]);
            for (unsigned pol = 0; pol < 2; ++pol) {
                const unsigned c = pol * 96 + 48 + v;  // k = 2 (phase-2 proposal channel)
                if (in.bank[site * 192 + c]) { ++count; sel_site = site; sel_c = c; }
            }
        }
        if (count == 1) {
            out.bank[sel_site * 192 + sel_c] = 0;
            const unsigned pol = sel_c / 96;
            out.sc[key * 2] = g_tables.blank;
            out.sc[key * 2 + 1] = g_tables.absorbed[pol];
            out.admitted_sc[key] = 1;
        }
    }
    for (unsigned p = 0; p < 3; ++p) {
        for (unsigned q = 0; q < 2; ++q) {
            std::size_t first, second;
            fcc_endpoints(i, L, p, q, first, second);
            out.gate_fcc[i * 6 + p * 2 + q] = std::uint8_t((population(in.bank, first) + population(in.bank, second)) % 2 == 0);
            const auto key = (i * 3 + p) * 2 + q;
            if (in.fcc[key * 2] != g_tables.blank || in.fcc[key * 2 + 1] != g_tables.blank) continue;
            unsigned count = 0, sel_c = 0; std::size_t sel_site = 0;
            for (unsigned s = 0; s < 2; ++s) {
                const auto* entry = g_tables.source_fcc[p][q][s];
                const auto site = offset_shift(i, L, entry);
                const unsigned v = unsigned(entry[3]);
                for (unsigned pol = 0; pol < 2; ++pol) {
                    const unsigned c = pol * 96 + 48 + v;
                    if (in.bank[site * 192 + c]) { ++count; sel_site = site; sel_c = c; }
                }
            }
            if (count == 1) {
                out.bank[sel_site * 192 + sel_c] = 0;
                const unsigned pol = sel_c / 96;
                out.fcc[key * 2] = g_tables.blank;
                out.fcc[key * 2 + 1] = g_tables.absorbed[pol];
                out.admitted_fcc[key] = 1;
            }
        }
    }
}

// One thread per site. Table collision per polarity (destination bank already zeroed by the
// host before this launch), then relation crossing/hold for every non-admitted SC/FCC slot.
__global__ void collision_kernel(std::uint8_t* source, std::uint8_t* dest, std::size_t n, const std::uint32_t* table) {
    const std::size_t i = std::size_t(blockIdx.x) * blockDim.x + threadIdx.x;
    if (i >= n) return;
    View in(source, n), out(dest, n);
    for (unsigned pol = 0; pol < 2; ++pol) {
        std::uint32_t mask = 0;
        unsigned labels[24], nl = 0;
        for (unsigned v = 0; v < 24; ++v)
            for (unsigned k = 0; k < 4; ++k)
                if (in.bank[i * 192 + pol * 96 + k * 24 + v]) { mask |= (1u << v); labels[nl++] = k; }
        if (!mask) continue;
        const std::uint32_t new_mask = table[mask];
        for (unsigned a = 1; a < nl; ++a) {
            const unsigned key = labels[a]; unsigned b = a;
            while (b > 0 && labels[b - 1] > key) { labels[b] = labels[b - 1]; --b; }
            labels[b] = key;
        }
        unsigned j = 0;
        for (unsigned v = 0; v < 24; ++v)
            if ((new_mask >> v) & 1u) out.bank[i * 192 + pol * 96 + labels[j++] * 24 + v] = 1;
    }
    for (unsigned a = 0; a < 3; ++a) {
        const auto key = i * 3 + a;
        if (in.admitted_sc[key]) continue;
        relation_device(in.sc[key * 2], in.sc[key * 2 + 1], out.sc[key * 2], out.sc[key * 2 + 1], in.gate_sc[key] != 0);
    }
    for (unsigned p = 0; p < 3; ++p)
        for (unsigned q = 0; q < 2; ++q) {
            const auto key = (i * 3 + p) * 2 + q;
            if (in.admitted_fcc[key]) continue;
            relation_device(in.fcc[key * 2], in.fcc[key * 2 + 1], out.fcc[key * 2], out.fcc[key * 2 + 1], in.gate_fcc[key] != 0);
        }
}

// One thread per site. Scatter without atomics: the frozen streaming map is injective, so
// distinct source channels never share a destination channel (destination bank already
// zeroed by the host before this launch).
__global__ void streaming_kernel(std::uint8_t* source, std::uint8_t* dest, std::size_t n, unsigned L) {
    const std::size_t i = std::size_t(blockIdx.x) * blockDim.x + threadIdx.x;
    if (i >= n) return;
    View in(source, n), out(dest, n);
    for (unsigned c = 0; c < 192; ++c) {
        if (!in.bank[i * 192 + c]) continue;
        const unsigned pol = c / 96, k = (c % 96) / 24, v = c % 24;
        const auto y = offset_shift(i, L, g_tables.velocity[v]);
        unsigned k2 = (k + 1) % 4;
        if (in.s[i] != 0) k2 = (k2 + 2) % 4;
        out.bank[y * 192 + pol * 96 + k2 * 24 + v] = 1;
    }
}

// One thread per site (gather form of the manifestation scatter, same algebraic identity
// engine/strict/staged_cuda.cu already uses for its own SC/FCC incidence sums), plus clearing
// admitted_sc, admitted_fcc, gate_sc AND gate_fcc (hydro splits these into separate pairs;
// v2's manifestation_kernel only has one combined admitted/gate field each).
__global__ void manifestation_kernel(std::uint8_t* source, std::uint8_t* dest, std::size_t n, unsigned L) {
    const std::size_t i = std::size_t(blockIdx.x) * blockDim.x + threadIdx.x;
    if (i >= n) return;
    View in(source, n), out(dest, n);
    int incidence = 0;
    for (unsigned a = 0; a < 3; ++a) {
        const auto m = site_shift(i, L, a, -1);
        incidence += g_tables.eps[in.sc[(i * 3 + a) * 2]] - g_tables.eps[in.sc[(m * 3 + a) * 2]];
    }
    for (unsigned p = 0; p < 3; ++p) {
        unsigned a, b; plane(p, a, b);
        const auto ma = site_shift(i, L, a, -1), mb = site_shift(i, L, b, -1);
        incidence += g_tables.eps[in.fcc[((i * 3 + p) * 2 + 0) * 2]]
                    - g_tables.eps[in.fcc[((site_shift(ma, L, b, -1) * 3 + p) * 2 + 0) * 2]];
        incidence += g_tables.eps[in.fcc[((ma * 3 + p) * 2 + 1) * 2]]
                    - g_tables.eps[in.fcc[((mb * 3 + p) * 2 + 1) * 2]];
    }
    out.s[i] = std::int8_t(((incidence + 1) % 3 + 3) % 3 - 1);
    for (unsigned a = 0; a < 3; ++a) out.admitted_sc[i * 3 + a] = 0;
    for (unsigned p = 0; p < 3; ++p) for (unsigned q = 0; q < 2; ++q) out.admitted_fcc[(i * 3 + p) * 2 + q] = 0;
    for (unsigned a = 0; a < 3; ++a) out.gate_sc[i * 3 + a] = 0;
    for (unsigned p = 0; p < 3; ++p) for (unsigned q = 0; q < 2; ++q) out.gate_fcc[(i * 3 + p) * 2 + q] = 0;
}

// Host-side inverse-offset shift (identical to hydro_runtime.cpp's anonymous-namespace
// `shift(L, site, offset[3])`, duplicated here since that overload is not part of the public
// hydro_runtime.h API), used only to recover an absorbed particle's owning relation for event
// bookkeeping below.
std::size_t host_shift(std::uint32_t L, std::size_t site, const std::int8_t offset[3]) {
    std::size_t xyz[3] = {site / (std::size_t(L) * L), (site / L) % L, site % L};
    for (unsigned k = 0; k < 3; ++k) {
        int v = int(xyz[k]) + int(offset[k]);
        v %= int(L);
        if (v < 0) v += int(L);
        xyz[k] = std::size_t(v);
    }
    return (xyz[0] * L + xyz[1]) * L + xyz[2];
}

// Deterministic forensic extraction from the actual before/after host-decoded device states.
// This never recomputes or repairs the next state; it only observes what already happened.
Events observed_events(const State& before, const State& after) {
    Events ev;
    using Byte = std::uint8_t;
    const auto n = before.s.size();
    if (before.phase() == 0) {
        for (std::size_t x = 0; x < n; ++x)
            for (unsigned c = 0; c < 192; ++c)
                if (before.bank[x * 192 + c] && !after.bank[x * 192 + c]) {
                    const unsigned v = c % 24;
                    const bool fcc = tables::TARGET_KIND[v] != 0;
                    const auto owner = host_shift(before.L, x, tables::TARGET_OFFSET[v]);
                    ev.absorptions.push_back({x, c, fcc, owner, unsigned(tables::TARGET_INDEX[v][0]), unsigned(tables::TARGET_INDEX[v][1])});
                }
    } else if (before.phase() == 1) {
        for (std::size_t x = 0; x < n; ++x) {
            for (unsigned pol = 0; pol < 2; ++pol) {
                std::uint32_t before_mask = 0, after_mask = 0;
                for (unsigned v = 0; v < 24; ++v)
                    for (unsigned k = 0; k < 4; ++k) {
                        if (before.bank[x * 192 + pol * 96 + k * 24 + v]) before_mask |= 1u << v;
                        if (after.bank[x * 192 + pol * 96 + k * 24 + v]) after_mask |= 1u << v;
                    }
                if (before_mask != after_mask)
                    ev.collisions.push_back({x, pol == 0 ? 1 : -1, before_mask, after_mask});
            }
            for (unsigned a = 0; a < 3; ++a) {
                const auto key = x * 3 + a;
                if (before.admitted_sc[key]) continue;
                const Byte l = before.sc[key * 2], r = before.sc[key * 2 + 1];
                const bool lo = l != frozen::BLANK, ro = r != frozen::BLANK;
                const bool newlo = after.sc[key * 2] != frozen::BLANK;
                const int direction = (lo && !newlo) ? 1 : ((!lo && newlo) ? -1 : 0);
                if (direction) ev.crossings.push_back({false, x, a, 0, direction});
                const bool gate = before.gate_sc[key] != 0;
                if (lo != ro && frozen::PHASE[lo ? l : r] == 0 && !gate)
                    ev.gate_holds.push_back({false, x, a, 0, 0});
            }
            for (unsigned p = 0; p < 3; ++p)
                for (unsigned q = 0; q < 2; ++q) {
                    const auto key = (x * 3 + p) * 2 + q;
                    if (before.admitted_fcc[key]) continue;
                    const Byte l = before.fcc[key * 2], r = before.fcc[key * 2 + 1];
                    const bool lo = l != frozen::BLANK, ro = r != frozen::BLANK;
                    const bool newlo = after.fcc[key * 2] != frozen::BLANK;
                    const int direction = (lo && !newlo) ? 1 : ((!lo && newlo) ? -1 : 0);
                    if (direction) ev.crossings.push_back({true, x, p, q, direction});
                    const bool gate = before.gate_fcc[key] != 0;
                    if (lo != ro && frozen::PHASE[lo ? l : r] == 0 && !gate)
                        ev.gate_holds.push_back({true, x, p, q, 0});
                }
        }
    }
    return ev;
}

}  // namespace

std::string device_json() {
    int device = 0; check(cudaGetDevice(&device), "cudaGetDevice");
    cudaDeviceProp properties{}; check(cudaGetDeviceProperties(&properties, device), "cudaGetDeviceProperties");
    std::ostringstream out;
    out << "{\"backend\":\"cuda_device_kernels\",\"name\":\"" << properties.name
        << "\",\"compute_major\":" << properties.major << ",\"compute_minor\":" << properties.minor
        << ",\"device\":" << device << ",\"canonical_adoption\":false}";
    return out.str();
}

Events advance(State& state, std::uint64_t microticks) {
    validate(state);
    if (microticks > std::numeric_limits<std::uint64_t>::max() - state.microtick)
        throw std::overflow_error("hydro CUDA microtick overflow");
    if (!microticks) return {};
    auto bytes = encode(state);
    const auto n = state.s.size();
    const auto size = n * BYTES_PER_SITE;
    constexpr unsigned threads = 128;
    const auto required_blocks = n / threads + (n % threads != 0);
    int device = 0;
    cudaDeviceProp properties{};
    check(cudaGetDevice(&device), "cudaGetDevice");
    check(cudaGetDeviceProperties(&properties, device), "cudaGetDeviceProperties");
    if (required_blocks > std::numeric_limits<unsigned>::max() || required_blocks > std::size_t(properties.maxGridSize[0]))
        throw std::overflow_error("CUDA hydro state exceeds supported finite launch grid");
    const auto blocks = static_cast<unsigned>(required_blocks);

    const auto& host_table = ftd::hydro::table();
    DeviceBuffer first(size), second(size), table_buffer(host_table.size() * sizeof(std::uint32_t));
    check(cudaMemcpy(table_buffer.pointer, host_table.data(), host_table.size() * sizeof(std::uint32_t), cudaMemcpyHostToDevice),
          "upload collision table");
    auto tables_host = host_tables();
    check(cudaMemcpyToSymbol(g_tables, &tables_host, sizeof(Tables)), "upload constant tables");
    check(cudaMemcpy(first.pointer, bytes.data() + HEADER_BYTES, size, cudaMemcpyHostToDevice), "upload state");

    auto input = static_cast<std::uint8_t*>(first.pointer), output = static_cast<std::uint8_t*>(second.pointer);
    const auto table_ptr = static_cast<const std::uint32_t*>(table_buffer.pointer);
    State current = state;
    const auto work = work_units(state);
    Events combined;
    for (std::uint64_t tick = 0; tick < microticks; ++tick) {
        check(cudaMemcpy(output, input, size, cudaMemcpyDeviceToDevice), "copy immutable pre-state");
        switch (current.phase()) {
        case 0:
            admission_kernel<<<blocks, threads>>>(input, output, n, state.L);
            break;
        case 1:
            check(cudaMemset(output + n, 0, 192 * n), "clear collision destination bank");
            collision_kernel<<<blocks, threads>>>(input, output, n, table_ptr);
            break;
        case 2:
            check(cudaMemset(output + n, 0, 192 * n), "clear streaming destination bank");
            streaming_kernel<<<blocks, threads>>>(input, output, n, state.L);
            break;
        default:
            manifestation_kernel<<<blocks, threads>>>(input, output, n, state.L);
            break;
        }
        check(cudaGetLastError(), "launch hydro kernel");
        check(cudaDeviceSynchronize(), "complete hydro microtick");
        check(cudaMemcpy(bytes.data() + HEADER_BYTES, output, size, cudaMemcpyDeviceToHost), "download state");
        const auto clock = current.microtick + 1;
        for (unsigned i = 0; i < 8; ++i) bytes[12 + i] = std::uint8_t(clock >> (8 * i));
        State next = decode(bytes);
        if (work_units(next) != work) throw std::runtime_error("CUDA hydro work conservation failure");
        auto ev = observed_events(current, next);
        combined.absorptions.insert(combined.absorptions.end(), ev.absorptions.begin(), ev.absorptions.end());
        combined.collisions.insert(combined.collisions.end(), ev.collisions.begin(), ev.collisions.end());
        combined.crossings.insert(combined.crossings.end(), ev.crossings.begin(), ev.crossings.end());
        combined.gate_holds.insert(combined.gate_holds.end(), ev.gate_holds.begin(), ev.gate_holds.end());
        current = std::move(next);
        std::swap(input, output);
    }
    state = std::move(current);  // Single commit after all device and validation work.
    return combined;
}

}  // namespace ftd::hydro::gpu

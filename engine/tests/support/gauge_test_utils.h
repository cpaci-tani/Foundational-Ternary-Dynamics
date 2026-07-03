#pragma once
// ============================================================================
// gauge_test_utils.h — shared helpers for the SU(2)/SU(3) gauge-sector tests
// (test_gauge_links.cpp, test_gauge_gpu_parity.cpp). Revision 0.9 option a.
//
// The perturbation is the SAME deterministic xorshift stream in every test so
// the CPU golden profile and the GPU parity run start from an identical link
// configuration. Do not change the seed or the draw order — the pinned
// GAUGE_GOLDEN_HASH in test_gauge_links.cpp depends on it.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/gauge_field.h"
#include "golden_hash.h"   // sibling header — resolves on gcc and MSVC alike

#include <complex>
#include <cstdint>
#include <vector>

namespace ftd { namespace test {

// Deterministic perturbation of the (identity-initialized) link fields —
// moves the relaxation off the identity fixed point (identity is exactly
// stationary under the staple update, so an unperturbed run would not
// exercise the sweep). Test-only const_cast: the engine's only writers are
// the relax sweeps themselves; tests seed the initial configuration through
// the const accessors (which lazily materialize the buffers, revision 4.1b).
inline void perturb_links(RenderBridge& rb, double eps) {
    auto& lx = const_cast<std::vector<SU2Link>&>(rb.su2_links_x());
    auto& ly = const_cast<std::vector<SU2Link>&>(rb.su2_links_y());
    auto& lz = const_cast<std::vector<SU2Link>&>(rb.su2_links_z());
    auto& mx = const_cast<std::vector<SU3Link>&>(rb.su3_links_x());
    std::uint64_t s = 0x9e3779b97f4a7c15ULL;
    auto next = [&s]() {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        return (double)(s >> 11) / (double)(1ULL << 53) - 0.5;
    };
    for (std::size_t i = 0; i < lx.size(); ++i) {
        for (auto* l : {&lx[i], &ly[i], &lz[i]}) {
            l->a += std::complex<double>(eps * next(), eps * next());
            l->b += std::complex<double>(eps * next(), eps * next());
            l->normalize();
        }
        // SU3: small off-diagonal perturbation, x-direction only (enough to
        // move the relaxation off the identity fixed point).
        mx[i].m[0][1] += std::complex<double>(eps * next(), eps * next());
        mx[i].m[1][0] -= std::conj(mx[i].m[0][1]);
    }
}

inline std::uint64_t hash_su2_links(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;
    for (const auto* v : {&rb.su2_links_x(), &rb.su2_links_y(), &rb.su2_links_z()}) {
        for (const auto& l : *v) {
            h = mix_double(h, l.a.real()); h = mix_double(h, l.a.imag());
            h = mix_double(h, l.b.real()); h = mix_double(h, l.b.imag());
        }
    }
    return h;
}

// SU(2) + SU(3) links, all directions, fixed traversal order.
inline std::uint64_t hash_all_links(const RenderBridge& rb) {
    std::uint64_t h = hash_su2_links(rb);
    for (const auto* v : {&rb.su3_links_x(), &rb.su3_links_y(), &rb.su3_links_z()}) {
        for (const auto& l : *v) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    h = mix_double(h, l.m[i][j].real());
                    h = mix_double(h, l.m[i][j].imag());
                }
            }
        }
    }
    return h;
}

}}  // namespace ftd::test

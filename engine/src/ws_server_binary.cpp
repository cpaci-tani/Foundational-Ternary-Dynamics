/**
 * @file ws_server_binary.cpp
 * @brief Bounded binary visualization payload encoders.
 */

#include "ws_server_internal.h"

#include "ftd/visual_sample_grid.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace ftd::ws_server_detail {

// ============================================================================
//  Particle data extraction (matches WASM get_particle_data)
// ============================================================================

std::vector<uint8_t> pack_particle_data(ftd::RenderBridge& rb) {
    constexpr uint32_t kParticleFrameMagic = 0x32505446u; // LE bytes: F T P 2
    std::vector<std::int8_t> states;
    rb.copy_visual_states(states);
    const auto& read_rb = std::as_const(rb);
    const int N = read_rb.lattice().size();
    const int total = N * N * N;

    // Count visible voxels (strictly manifested particles state != 0). A dense
    // L=181 state can contain almost six million particles; sending all of them
    // would allocate a ~166 MiB frame and a second WebGL geometry of comparable
    // size. Systematic deterministic sampling keeps the visualization bounded
    // without changing the engine state or diagnostic counts.
    std::size_t manifested_count = 0;
    for (int i = 0; i < total; i++) {
        if (states[static_cast<std::size_t>(i)] != 0)
            ++manifested_count;
    }
    const std::size_t count = (std::min)(manifested_count, kMaxVisualParticles);

    std::vector<int> selected_indices;
    selected_indices.reserve(count);
    std::uint64_t select_accumulator = 0;
    for (int i = 0; i < total; ++i) {
        if (states[static_cast<std::size_t>(i)] == 0) continue;
        if (manifested_count > count) {
            select_accumulator += count;
            if (select_accumulator < manifested_count) continue;
            select_accumulator -= manifested_count;
        }
        selected_indices.push_back(i);
    }

    // Compact GPU gather: five floats per selected site (remainder xyz, spin,
    // color). This preserves sub-voxel motion without downloading the full AoS
    // voxel mirror, which is multi-gigabyte at native large-lattice sizes.
    std::vector<float> attributes;
    rb.copy_visual_particle_attributes(selected_indices, attributes);

    // FTP2 layout: [u32 magic][u32 count][pos 3N][color 3N][size N]
    //              [spin N][colorCharge N], all payload values float32.
    size_t header_bytes = 8;
    size_t pos_bytes    = count * 3 * sizeof(float);
    size_t col_bytes    = count * 3 * sizeof(float);
    size_t size_bytes   = count * sizeof(float);
    size_t spin_bytes   = count * sizeof(float);
    size_t charge_bytes = count * sizeof(float);
    size_t total_bytes  = header_bytes + pos_bytes + col_bytes + size_bytes
                        + spin_bytes + charge_bytes;

    std::vector<uint8_t> buf(total_bytes);
    auto* ptr = buf.data();

    std::memcpy(ptr, &kParticleFrameMagic, sizeof(kParticleFrameMagic));
    ptr += sizeof(kParticleFrameMagic);
    uint32_t cnt = static_cast<uint32_t>(count);
    std::memcpy(ptr, &cnt, 4);
    ptr += 4;

    const std::size_t pos_offset = header_bytes;
    const std::size_t color_offset = pos_offset + pos_bytes;
    const std::size_t size_offset = color_offset + col_bytes;
    const std::size_t spin_offset = size_offset + size_bytes;
    const std::size_t charge_offset = spin_offset + spin_bytes;

    for (std::size_t idx = 0; idx < selected_indices.size(); ++idx) {
        const int i = selected_indices[idx];
        const std::int8_t state = states[static_cast<std::size_t>(i)];
        const auto c = read_rb.lattice().coord(i);
        const std::size_t a = idx * 5u;

        const float px = static_cast<float>(c.x) + 0.5f + attributes[a + 0u];
        const float py = static_cast<float>(c.y) + 0.5f + attributes[a + 1u];
        const float pz = static_cast<float>(c.z) + 0.5f + attributes[a + 2u];
        ftd::write_binary_value(buf, pos_offset + (idx * 3u + 0u) * sizeof(float), px);
        ftd::write_binary_value(buf, pos_offset + (idx * 3u + 1u) * sizeof(float), py);
        ftd::write_binary_value(buf, pos_offset + (idx * 3u + 2u) * sizeof(float), pz);

        // Color by state
        if (state == 1) {
            // Green (positive)
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 0u) * sizeof(float), 0.29f);
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 1u) * sizeof(float), 0.87f);
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 2u) * sizeof(float), 0.50f);
        } else { // state == -1
            // Red (negative)
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 0u) * sizeof(float), 0.97f);
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 1u) * sizeof(float), 0.44f);
            ftd::write_binary_value(buf, color_offset + (idx * 3u + 2u) * sizeof(float), 0.44f);
        }

        // Size matches WASM particle size
        ftd::write_binary_value(buf, size_offset + idx * sizeof(float), 6.0f);
        ftd::write_binary_value(buf, spin_offset + idx * sizeof(float), attributes[a + 3u]);
        ftd::write_binary_value(buf, charge_offset + idx * sizeof(float), attributes[a + 4u]);
    }

    return buf;
}

// Compact sampled visualization frame:
// ["FTV2"][u32 latticeSize][u32 effectiveStride][u32 origin][u32 axisCount]
// [float32 density[axisCount^3]], x-fastest.
//
// Origin is the centre-anchored first voxel of visual_sample_grid so the
// dashboard can place samples at the same physical coordinates FTS2 uses.
// The renderer draws at most 53^3 points. Sending the old dense FTV1 N^3
// lattice therefore downloaded and transferred ~23.7 MiB on every L=181
// refresh only to discard 97.5% of it in JavaScript. Reuse the sparse compact
// GPU field sampler and materialize only its bounded regular grid here.
std::vector<uint8_t> pack_flux_volume(ftd::RenderBridge& rb,
                                      int requested_axis_samples) {
    constexpr uint32_t kFluxVolumeMagic = 0x32565446u; // LE bytes: F T V 2
    const auto& read_rb = std::as_const(rb);
    const int n = read_rb.lattice().size();
    const int target_axis = std::clamp(requested_axis_samples, 1, 64);
    const int requested_stride = std::max(1, (n + target_axis - 1) / target_axis);
    ftd::VisualFieldSample sample;
    rb.copy_visual_field_sample(
        ftd::VisualFieldKind::FluxVector, requested_stride, sample);

    const int stride = std::max(1, sample.effective_stride);
    const auto grid = ftd::visual_sample_grid(
        n, stride, ftd::is_interior_field_kind(ftd::VisualFieldKind::FluxVector));
    const int origin = grid.origin;
    const int axis_count = grid.count;
    const std::size_t count = axis_count > 0
        ? static_cast<std::size_t>(axis_count) * axis_count * axis_count
        : 0;
    std::vector<uint8_t> buf(20u + count * sizeof(float), 0u);
    const uint32_t header[5] = {
        kFluxVolumeMagic,
        static_cast<uint32_t>(n),
        static_cast<uint32_t>(stride),
        static_cast<uint32_t>(origin),
        static_cast<uint32_t>(axis_count),
    };
    std::memcpy(buf.data(), header, sizeof(header));
    const std::size_t compact_count = sample.count();
    if (sample.components != 3u || sample.positions.size() != compact_count * 3u
        || sample.data.size() != compact_count * 3u) {
        throw std::runtime_error("invalid compact flux-vector sample layout");
    }
    for (std::size_t i = 0; i < compact_count; ++i) {
        const int x = static_cast<int>(sample.positions[i * 3u + 0u]);
        const int y = static_cast<int>(sample.positions[i * 3u + 1u]);
        const int z = static_cast<int>(sample.positions[i * 3u + 2u]);
        const int xi = (x - origin) / stride;
        const int yi = (y - origin) / stride;
        const int zi = (z - origin) / stride;
        if (xi < 0 || yi < 0 || zi < 0
            || xi >= axis_count || yi >= axis_count || zi >= axis_count) {
            continue;
        }
        const float jx = sample.data[i * 3u + 0u];
        const float jy = sample.data[i * 3u + 1u];
        const float jz = sample.data[i * 3u + 2u];
        const std::size_t q = (static_cast<std::size_t>(zi) * axis_count + yi)
                            * axis_count + xi;
        const float density = std::sqrt(jx * jx + jy * jy + jz * jz);
        ftd::write_binary_value(buf, sizeof(header) + q * sizeof(float), density);
    }
    return buf;
}

// FTS2 sampled-field frame. The server always sends the effective compacted
// sample (not the dense candidate grid), so large lattices stay bounded in
// both PCIe and WebSocket traffic. Origin + effective stride describe the
// represented regular grid independently of sparse zero-value omission; the
// dashboard needs them to select the correct physical slice plane.
std::vector<uint8_t> pack_field_sample(ftd::RenderBridge& rb,
                                       ftd::VisualFieldKind kind,
                                       int stride,
                                       std::uint32_t token,
                                       int planes_mid = -1) {
    constexpr std::uint32_t kFieldSampleMagic = 0x32535446u; // F T S 2
    ftd::VisualFieldSample sample;
    rb.copy_visual_field_sample(kind, stride, sample);

    // Slice-panel mode. The dashboard's flux-slice panel only ever draws the
    // three center mid-planes (x=mid, y=mid, z=mid), yet the full cube is
    // several MiB per field over the WebSocket. When planes_mid >= 0, drop every
    // sample NOT on one of those three planes before packing — identical FTS2
    // layout (the client already slices by plane), but ~axis× fewer points
    // (3·axis² instead of axis³). effective_stride/origin are left describing the
    // FULL regular grid so the client's resolveSamplePlane still snaps to the
    // right plane. The full-cube path (planes_mid < 0) is unchanged and still
    // feeds the 3D viewport overlay, which genuinely needs the whole volume.
    if (planes_mid >= 0 && sample.count() > 0) {
        // Snap the requested mid to the nearest SAMPLED plane, mirroring the
        // client's resolveSamplePlane (flux-slice-helpers.js) exactly, so both
        // sides agree on which plane is kept vs drawn. Needed because the grid
        // is center-anchored on (N-1)/2 at the effective stride: for an even N
        // (e.g. the L=32 default) or a coarse stride, the raw mid=N>>1 is often
        // NOT itself a sampled coordinate, and filtering at it would keep an
        // empty plane while the client slices at the snapped one.
        const int origin = (std::max)(0, sample.origin);
        const int estride = (std::max)(1, sample.effective_stride);
        const int lattice_n = static_cast<int>(rb.lattice().size());
        const int last_allowed = origin > 0 ? lattice_n - 2 : lattice_n - 1;
        const int last = origin + ((std::max)(0, last_allowed - origin) / estride) * estride;
        int plane = planes_mid;
        if (planes_mid >= origin) {  // Math.round((mid-origin)/estride), integer form
            plane = origin + ((2 * (planes_mid - origin) + estride) / (2 * estride)) * estride;
        }
        plane = (std::max)(origin, (std::min)(last, plane));

        const std::uint32_t comp = sample.components;
        std::vector<float> fpos;
        std::vector<float> fdata;
        fpos.reserve(sample.positions.size());
        fdata.reserve(sample.data.size());
        const std::size_t n = sample.count();
        for (std::size_t i = 0; i < n; ++i) {
            // Positions are voxel centres (x + 0.5); truncation recovers x.
            const int ix = static_cast<int>(sample.positions[i * 3u + 0u]);
            const int iy = static_cast<int>(sample.positions[i * 3u + 1u]);
            const int iz = static_cast<int>(sample.positions[i * 3u + 2u]);
            if (ix != plane && iy != plane && iz != plane) continue;
            fpos.push_back(sample.positions[i * 3u + 0u]);
            fpos.push_back(sample.positions[i * 3u + 1u]);
            fpos.push_back(sample.positions[i * 3u + 2u]);
            for (std::uint32_t c = 0; c < comp; ++c)
                fdata.push_back(sample.data[i * comp + c]);
        }
        sample.positions.swap(fpos);
        sample.data.swap(fdata);
    }

    const std::uint32_t kind_code = static_cast<std::uint32_t>(kind);
    const std::uint32_t components = sample.components;
    const std::uint32_t count = static_cast<std::uint32_t>(sample.count());
    const std::size_t header_bytes = 7u * sizeof(std::uint32_t);
    const std::size_t position_bytes = static_cast<std::size_t>(count) * 3u * sizeof(float);
    const std::size_t data_bytes = static_cast<std::size_t>(count) * components * sizeof(float);
    std::vector<std::uint8_t> frame(header_bytes + position_bytes + data_bytes);
    std::uint32_t header[7] = {
        kFieldSampleMagic, token, kind_code, components, count,
        static_cast<std::uint32_t>(std::max(1, sample.effective_stride)),
        static_cast<std::uint32_t>(std::max(0, sample.origin)),
    };
    std::memcpy(frame.data(), header, header_bytes);
    if (position_bytes != 0)
        std::memcpy(frame.data() + header_bytes, sample.positions.data(), position_bytes);
    if (data_bytes != 0)
        std::memcpy(frame.data() + header_bytes + position_bytes,
                    sample.data.data(), data_bytes);
    return frame;
}

}  // namespace ftd::ws_server_detail

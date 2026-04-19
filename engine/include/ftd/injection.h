#pragma once
/**
 * Injection — state-mutating primitives for seeding the lattice.
 *
 * Extracted from render_bridge.cpp in the 2026-04-18 R5 refactor. These
 * functions mutate RenderBridge state (voxels, host_mutated_, gpu_dirty_,
 * next_particle_id_, next_pair_id_). The RenderBridge:: methods stay as
 * thin forwarders so the public API signature is unchanged.
 */

#include "ftd/voxel.h"

namespace ftd {

class RenderBridge;
struct AggregateProfile;

void inject_flux_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val);
void inject_flux_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val);
void inject_wave_vel_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& wv_val);
void inject_particle_cpu(RenderBridge& rb, int x, int y, int z, int8_t state,
                         const Vec3& flux_val, int8_t spin, int8_t color);
void inject_wavepacket_cpu(RenderBridge& rb, int cx, int cy, int cz, int8_t state,
                           double sigma, double amplitude);
void create_entangled_pair_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val);
AggregateProfile compute_aggregate_profile(const RenderBridge& rb, int center_idx, double threshold);

}  // namespace ftd

#pragma once
/**
 * Diagnostics — read-only reductions over voxel state.
 *
 * Extracted from render_bridge.cpp in the 2026-04-18 R4 refactor. All
 * helpers take `const RenderBridge&` because they are purely read-only:
 * they observe voxels, phi_coulomb, and toggles and return aggregate
 * structs (Diagnostics, EnergyAudit) or per-site EM fields.
 */

#include "ftd/voxel.h"

namespace ftd {

class RenderBridge;
struct Diagnostics;
struct EnergyAudit;
struct EMFieldDiag;

Diagnostics  compute_diagnostics(const RenderBridge& rb);
EnergyAudit  compute_energy_audit(const RenderBridge& rb);
EMFieldDiag  compute_em_field_at(const RenderBridge& rb, int idx);
Vec3         compute_poynting_vector(const RenderBridge& rb, int idx);
double       compute_entropy_cpu(const RenderBridge& rb);

}  // namespace ftd

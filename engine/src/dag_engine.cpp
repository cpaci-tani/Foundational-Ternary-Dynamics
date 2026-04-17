#include "ftd/dag_engine.h"
#include <iostream>

namespace ftd {

DagEngine::DagEngine(int lattice_size) {
    dag_ = std::make_unique<SparseVoxelDAG>(lattice_size);
    
    // Core engine rules defaulting to ON
    toggles_.enable_all();
    
    // Disable extensions
    toggles_.larmor_radiation = false;
    toggles_.latency_field = false;
    toggles_.color_forces = false;
    toggles_.strong_force = false;
    toggles_.triad_binding = false;
    toggles_.pair_production = false;
    toggles_.exchange_force = false;

    // Buffer allocations (flat for now until we build a sparse delta list mapper)
    int total = lattice_size * lattice_size * lattice_size;
    delta_j_.resize(total, {0.0, 0.0, 0.0});
}

void DagEngine::clear() {
    int sz = dag_->size();
    dag_ = std::make_unique<SparseVoxelDAG>(sz);
    tick_ = 0;
}

void DagEngine::tick() {
    // 1. Compute delta_J (Laplacian wave equation, Biort-Savart coupling)
    if (toggles_.wave_propagation || toggles_.coupling) {
        phase_read();
    }
    
    // 2. Leapfrog wave update and threshold manifestation constraints
    phase_write();

    // 3. Exact U(1) charge conservation projection 
    if (toggles_.gauss_projection) {
        gauss_project();
    }

    // 4. Compute deterministic field-based forces
    if (toggles_.forces) {
        phase_forces();
    }

    // 5. Integration of velocities and strict movement collisions
    if (toggles_.movement) {
        phase_movement();
    }
    
    tick_++;
}

// -----------------------------------------------------------------------------
// Discrete Physics Operators
// -----------------------------------------------------------------------------

Vec3 DagEngine::laplacian_flux(int x, int y, int z) const {
    Vec3 lap;
    // 6 faces (1/3 weight)
    lap += dag_->get_voxel(x+1, y, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x-1, y, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y+1, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y-1, z).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y, z+1).flux * (1.0/3.0);
    lap += dag_->get_voxel(x, y, z-1).flux * (1.0/3.0);

    // 12 edges (1/6 weight)
    lap += dag_->get_voxel(x+1, y+1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y-1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y+1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y-1, z).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x+1, y, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x-1, y, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y+1, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y+1, z-1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y-1, z+1).flux * (1.0/6.0);
    lap += dag_->get_voxel(x, y-1, z-1).flux * (1.0/6.0);

    lap -= dag_->get_voxel(x, y, z).flux * 4.0;
    return lap;
}

Vec3 DagEngine::gradient_state(int x, int y, int z) const {
    Vec3 grad;
    grad.x = (dag_->get_voxel(x+1, y, z).state - dag_->get_voxel(x-1, y, z).state) * 0.5;
    grad.y = (dag_->get_voxel(x, y+1, z).state - dag_->get_voxel(x, y-1, z).state) * 0.5;
    grad.z = (dag_->get_voxel(x, y, z+1).state - dag_->get_voxel(x, y, z-1).state) * 0.5;
    return grad;
}

Vec3 DagEngine::curl_state_velocity(int x, int y, int z) const {
    auto jcur = [&](int px, int py, int pz) -> Vec3 {
        auto v = dag_->get_voxel(px, py, pz);
        return v.velocity * static_cast<double>(v.state);
    };
    Vec3 curl;
    curl.x = (jcur(x, y + 1, z).z - jcur(x, y - 1, z).z) * 0.5 -
             (jcur(x, y, z + 1).y - jcur(x, y, z - 1).y) * 0.5;
    curl.y = (jcur(x, y, z + 1).x - jcur(x, y, z - 1).x) * 0.5 -
             (jcur(x + 1, y, z).z - jcur(x - 1, y, z).z) * 0.5;
    curl.z = (jcur(x + 1, y, z).y - jcur(x - 1, y, z).y) * 0.5 -
             (jcur(x, y + 1, z).x - jcur(x, y - 1, z).x) * 0.5;
    return curl;
}

// -----------------------------------------------------------------------------
// Recursive Tree Traversals
// -----------------------------------------------------------------------------

void DagEngine::recursive_read(int x, int y, int z, int current_size) {
    // Structural leaf mapping reached
    if (current_size == 1) {
        // Check local activity bounds (skip deep pure void optimizations here later)
        int idx = z*(dag_->size()*dag_->size()) + y*dag_->size() + x;
        delta_j_[idx] = {};
        
        if (toggles_.wave_propagation) {
            delta_j_[idx] = laplacian_flux(x, y, z) * (C_WAVE * C_WAVE);
        }
        if (toggles_.coupling) {
            delta_j_[idx] += gradient_state(x, y, z) * G_C;
            delta_j_[idx] += curl_state_velocity(x, y, z) * G_C;
        }
        return;
    }

    // Recurse heavily into 8 Octants 
    int half = current_size >> 1;
    recursive_read(x,          y,          z,          half);
    recursive_read(x,          y,          z + half,   half);
    recursive_read(x,          y + half,   z,          half);
    recursive_read(x,          y + half,   z + half,   half);
    recursive_read(x + half,   y,          z,          half);
    recursive_read(x + half,   y,          z + half,   half);
    recursive_read(x + half,   y + half,   z,          half);
    recursive_read(x + half,   y + half,   z + half,   half);
}

void DagEngine::recursive_write(int x, int y, int z, int current_size) {
    if (current_size == 1) {
        int idx = z*(dag_->size()*dag_->size()) + y*dag_->size() + x;
        Voxel v = dag_->get_voxel(x, y, z);
        Vec3 dj = delta_j_[idx];
        
        bool needs_update = (dj.mag2() > 0.0 || v.wave_vel.mag2() > 0.0 || v.flux.mag2() > 0.0 || v.state != 0);
        
        if (needs_update) {
            v.wave_vel += dj;
            v.flux += v.wave_vel;
            
            if (toggles_.damping) {
                double eff_damping = 1.0 - DAMPING;
                v.flux *= eff_damping;
                v.wave_vel *= eff_damping;
            }
            
            // Dynamic allocation through structural COW
            dag_->set_voxel(x, y, z, v);
        }
        return;
    }
    
    // Recurse into 8 Octants
    int half = current_size >> 1;
    recursive_write(x,          y,          z,          half);
    recursive_write(x,          y,          z + half,   half);
    recursive_write(x,          y + half,   z,          half);
    recursive_write(x,          y + half,   z + half,   half);
    recursive_write(x + half,   y,          z,          half);
    recursive_write(x + half,   y,          z + half,   half);
    recursive_write(x + half,   y + half,   z,          half);
    recursive_write(x + half,   y + half,   z + half,   half);
}

// -----------------------------------------------------------------------------
// Core Engine Phases
// -----------------------------------------------------------------------------

void DagEngine::phase_read() {
    recursive_read(0, 0, 0, dag_->size());
}

void DagEngine::phase_write() {
    recursive_write(0, 0, 0, dag_->size());
}

void DagEngine::gauss_project() {
    // [OPEN] Implement recursive SOR solver skipping active manifested indices.
}

void DagEngine::phase_forces() {
    // [OPEN] Implement recursive Poisson and Lorentz force summations.
}

void DagEngine::phase_movement() {
    // [OPEN] Integrate fractional remainder accumulation handling for precise tracking.
}

// -----------------------------------------------------------------------------
// Test APIs & Infrastructure Implementations
// -----------------------------------------------------------------------------

void DagEngine::inject_flux(int x, int y, int z, double fx, double fy, double fz) {
    Voxel v = dag_->get_voxel(x, y, z);
    v.flux.x += fx;
    v.flux.y += fy;
    v.flux.z += fz;
    dag_->set_voxel(x, y, z, v);
}

bool DagEngine::get_toggle(const std::string& name) const { return false; }
void DagEngine::set_toggle(const std::string& name, bool value) {}
ScaleBaseDiagnostics DagEngine::base_diagnostics() const { return {}; }

} // namespace ftd

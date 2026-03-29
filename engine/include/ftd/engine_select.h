#pragma once
/**
 * Engine selection helper.
 *
 * When FTD_ENABLE_CUDA is defined, provides a GPU-backed simulation engine
 * that wraps GpuEngine with the same API as RenderBridge. Otherwise, falls
 * back to RenderBridge (CPU).
 *
 * Usage in campaigns:
 *   #include "ftd/engine_select.h"
 *   ftd::SimEngine engine(64);          // Uses GPU if available, CPU otherwise
 *   engine.inject_flux(x, y, z, flux);
 *   engine.run(200);
 *   auto voxels = engine.get_voxels();  // Always returns host-side data
 */

#include "render_bridge.h"
#include <memory>

#ifdef FTD_ENABLE_CUDA
#include "gpu_engine.h"
#endif

namespace ftd {

class SimEngine {
public:
    explicit SimEngine(int lattice_size)
        : size_(lattice_size), N_(lattice_size * lattice_size * lattice_size)
    {
#ifdef FTD_ENABLE_CUDA
        gpu_ = std::make_unique<gpu::GpuEngine>(lattice_size);
        use_gpu_ = true;
#endif
        if (!use_gpu_) {
            cpu_ = std::make_unique<RenderBridge>(lattice_size);
        }
    }

    // Toggle access
    TermToggles& toggles() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) return gpu_->toggles;
#endif
        return cpu_->toggles;
    }

    void inject_flux(int x, int y, int z, const Vec3& flux_val) {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) { gpu_->inject_flux(x, y, z, flux_val); return; }
#endif
        cpu_->inject_flux(x, y, z, flux_val);
    }

    void inject_particle(int x, int y, int z, int8_t state,
                         const Vec3& flux_val,
                         int8_t spin = 0, int8_t color = 0) {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) { gpu_->inject_particle(x, y, z, state, flux_val, spin, color); return; }
#endif
        cpu_->inject_particle(x, y, z, state, flux_val, spin, color);
    }

    void inject_wavepacket(int cx, int cy, int cz, int8_t state,
                           double sigma = 3.0, double amplitude = K_B) {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) { gpu_->inject_wavepacket(cx, cy, cz, state, sigma, amplitude); return; }
#endif
        cpu_->inject_wavepacket(cx, cy, cz, state, sigma, amplitude);
    }

    void tick() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) { gpu_->tick(); return; }
#endif
        cpu_->tick();
    }

    void run(int num_ticks) {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) { gpu_->run(num_ticks); return; }
#endif
        cpu_->run(num_ticks);
    }

    int current_tick() const {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) return gpu_->current_tick();
#endif
        return cpu_->current_tick();
    }

    int lattice_size() const { return size_; }
    int total_sites() const { return N_; }

    // Get voxels to host for inspection
    const std::vector<Voxel>& get_voxels() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) {
            gpu_->sync_to_host(host_cache_);
            return host_cache_;
        }
#endif
        return cpu_->voxels();
    }

    // Access a single voxel (syncs from GPU if needed)
    const Voxel& voxel_at(int x, int y, int z) {
        int idx = ((x % size_ + size_) % size_) * size_ * size_
                + ((y % size_ + size_) % size_) * size_
                + ((z % size_ + size_) % size_);
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) {
            gpu_->sync_to_host(host_cache_);
            return host_cache_[idx];
        }
#endif
        return cpu_->voxels()[idx];
    }

    Diagnostics diagnostics() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) return gpu_->diagnostics();
#endif
        return cpu_->diagnostics();
    }

    EnergyAudit energy_audit() {
#ifdef FTD_ENABLE_CUDA
        if (use_gpu_) return gpu_->energy_audit();
#endif
        return cpu_->energy_audit();
    }

    bool using_gpu() const { return use_gpu_; }

private:
#ifdef FTD_ENABLE_CUDA
    std::unique_ptr<gpu::GpuEngine> gpu_;
#endif
    std::unique_ptr<RenderBridge> cpu_;
    int size_;
    int N_;
    bool use_gpu_;
    std::vector<Voxel> host_cache_;
};

}  // namespace ftd

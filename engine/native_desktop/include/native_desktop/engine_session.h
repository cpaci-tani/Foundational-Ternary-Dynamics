#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ftd {
class RenderBridge;
}

namespace ftd::native_desktop {

struct NativeParticle {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float size = 0.45f;
};

struct NativeFrame {
    int tick = 0;
    int lattice_size = 0;
    int flux_boundary = 2;
    std::uint32_t total_manifested = 0;
    std::string scenario;
    std::string backend;
    std::string status;
    std::vector<NativeParticle> particles;
    std::vector<NativeParticle> flux;
};

struct NativeEngineOptions {
    int lattice_size = 32;
    std::string scenario = "s0-seed-hydrogen";
    bool force_cpu = true;
    // Matches the web Scale-0 toolbar default (Dispersal).
    int flux_boundary = 2;
};

class NativeEngineSession {
public:
    explicit NativeEngineSession(NativeEngineOptions options);
    ~NativeEngineSession();

    NativeEngineSession(const NativeEngineSession&) = delete;
    NativeEngineSession& operator=(const NativeEngineSession&) = delete;

    void tick();
    NativeFrame capture();
    NativeFrame capture_particles() { return capture(); }

    void apply_options(NativeEngineOptions options);
    void load_scenario(std::string name);
    void set_lattice_size(int lattice_size);
    void set_flux_boundary(int flux_boundary);
    void reset_current();

    int lattice_size() const;
    int current_tick() const;
    int flux_boundary() const { return options_.flux_boundary; }
    const char* backend_name() const;
    const std::string& scenario() const { return options_.scenario; }
    const std::string& status() const { return status_; }
    const NativeEngineOptions& options() const { return options_; }

private:
    void boot();
    void apply_boundary();
    void fill_frame_meta(NativeFrame& frame) const;

    NativeEngineOptions options_;
    std::unique_ptr<RenderBridge> bridge_;
    std::string status_;
};

}  // namespace ftd::native_desktop

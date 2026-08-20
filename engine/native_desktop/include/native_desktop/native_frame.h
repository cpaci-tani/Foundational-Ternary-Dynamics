#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

}  // namespace ftd::native_desktop

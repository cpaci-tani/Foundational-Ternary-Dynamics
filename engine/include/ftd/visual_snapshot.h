#pragma once
/**
 * @file visual_snapshot.h
 * @brief Versioned asynchronous capture contract for native visual frames.
 *
 * Visual capture is intentionally separate from scalar telemetry.  A native
 * renderer asks the backend to stage one bounded frame, then polls the fence
 * without forcing a canonical voxel mirror.  The request epoch is owned by
 * the caller (normally the native visual lane) and is echoed in the completed
 * snapshot so late frames can be discarded after a scenario/reset boundary.
 *
 * This header is CUDA-independent.  The first capture kind is particles; the
 * common request/result lifecycle is deliberately extensible to compact field
 * slices and volume frames without adding another synchronous readback API.
 */

#include <cstdint>
#include <type_traits>
#include <vector>

namespace ftd {

enum class VisualCaptureKind : std::uint32_t {
    Particles = 1,
};

// This is the existing native FTP2 visual cap.  It bounds both persistent GPU
// staging and one visual transport quantum; callers may request a smaller cap
// but never a larger one.
inline constexpr std::uint32_t kMaxVisualParticleCapture = 100000u;

/// Scheduler-owned capture request.  A zero max_particles selects the
/// protocol cap above.  `epoch` is opaque to the engine and is stamped into
/// the result at submission time.
struct VisualSnapshotRequest {
    VisualCaptureKind kind = VisualCaptureKind::Particles;
    std::uint64_t epoch = 0;
    std::uint32_t max_particles = kMaxVisualParticleCapture;
    double physical_time = 0.0;
    double dt = 1.0;
    int lattice_size = 0;
    // Interior occlusion cull ("video-game hack"): drop manifested sites buried
    // deeper than this many layers inside the clump from the VISUAL gather — a
    // site is culled when, along all 6 axis directions, the next N voxels are all
    // manifested (no void or lattice edge within N steps), so only a shell ~N
    // layers thick is captured. 0 = disabled (every manifested site is eligible,
    // bit-identical to the pre-cull behaviour — this is the default for all
    // non-native callers). Purely visual: physics, telemetry diagnostics, and the
    // true manifested count are unaffected.
    std::uint16_t interior_cull_layers = 0;
};

/// Immutable source provenance captured when begin_visual_snapshot() is
/// accepted.  `state_version == 0` is the CPU compatibility convention; its
/// caller-owned epoch and tick remain authoritative in that case.
struct VisualSnapshotMeta {
    std::uint64_t epoch = 0;
    std::uint64_t state_version = 0;
    int tick = 0;
    double physical_time = 0.0;
    double dt = 1.0;
    int lattice_size = 0;
};

/// One selected manifested lattice site.  This POD is also the fixed device /
/// pinned-host staging layout, so keep it trivially copyable and do not add
/// host-owned containers here.  The renderer derives its world position from
/// `index` plus the sub-voxel remainder.
struct VisualParticleRecord {
    std::int32_t index = -1;
    std::int8_t state = 0;
    std::int8_t spin = 0;
    std::int8_t color = 0;
    std::int8_t reserved = 0;
    float remainder_x = 0.0f;
    float remainder_y = 0.0f;
    float remainder_z = 0.0f;
};
static_assert(std::is_trivially_copyable<VisualParticleRecord>::value,
              "visual particle staging must remain POD");

/// Fixed staging header written by the device before its bounded record
/// gather.  It is public solely because GpuBuffers owns the typed pinned/device
/// allocation; normal callers receive VisualParticleCapture instead.
struct VisualParticleStagingHeader {
    std::uint32_t total_manifested = 0;
    std::uint32_t captured_count = 0;
};
static_assert(std::is_trivially_copyable<VisualParticleStagingHeader>::value,
              "visual particle staging header must remain POD");

/// Particle-specific payload.  `total_manifested` is the pre-sampling count;
/// `records` is deterministically sampled in ascending lattice-index order.
struct VisualParticleCapture {
    std::uint32_t total_manifested = 0;
    std::vector<VisualParticleRecord> records;
};

/// Completed capture.  `kind` identifies which payload is populated.  A
/// successful poll consumes the backend's one in-flight staging slot.
struct VisualSnapshot {
    VisualCaptureKind kind = VisualCaptureKind::Particles;
    VisualSnapshotMeta meta;
    VisualParticleCapture particles;
};

}  // namespace ftd

#pragma once
/**
 * @file interop_particle_record.h
 * @brief GPU-resident particle record shared between the CUDA gather kernel
 * and the D3D12 vertex shader that reads it via a StructuredBuffer.
 *
 * Unlike VisualParticleRecord (visual_snapshot.h), which carries a raw
 * lattice index and defers position decoding to the CPU, this struct carries
 * a fully-decoded world-space position and RGB color -- the interop gather
 * kernel runs on-device with no Lattice object available, so decoding must
 * happen there, not in HLSL or on the CPU.
 *
 * Layout MUST exactly match the HLSL StructuredBuffer element in
 * d3d12_presenter.cpp's interop vertex shader (kInteropParticleShader) --
 * float4-aligned, 32 bytes. If you change one side, change the other in the
 * same commit and re-run test_interop_particle_record_layout.
 */

#include <cstdint>
#include <type_traits>

namespace ftd {

struct InteropParticleRecord {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float size = 0.45f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
    float _pad = 0.0f;  // pads to 32 bytes (two float4s) -- HLSL cbuffer/
                        // structured-buffer packing rules want 16-byte
                        // alignment for the whole element when indexed by
                        // SV_InstanceID; 32 keeps it a clean float4 multiple.
};
static_assert(std::is_trivially_copyable<InteropParticleRecord>::value,
              "interop particle record must remain POD (written by a CUDA "
              "kernel, read by a D3D12 StructuredBuffer)");
static_assert(sizeof(InteropParticleRecord) == 32,
              "must match the HLSL struct's 32-byte stride exactly");

/// Fixed header the gather kernel writes before the record array, mirroring
/// VisualParticleStagingHeader (visual_snapshot.h) but trimmed to just the
/// one count the D3D12 draw call needs.
struct InteropParticleHeader {
    std::uint32_t captured_count = 0;
};
static_assert(std::is_trivially_copyable<InteropParticleHeader>::value,
              "interop header must remain POD");

}  // namespace ftd

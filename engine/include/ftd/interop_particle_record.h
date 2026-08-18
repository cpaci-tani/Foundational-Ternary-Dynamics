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
 * d3d12_presenter.cpp's interop vertex shader (kInteropParticleShader --
 * added in a later task) -- float4-aligned, 32 bytes. If you change one
 * side, change the other in the same commit and re-run
 * test_interop_particle_record_layout.
 */

#include <cstdint>
#include <type_traits>

namespace ftd {

/// One GPU-resident particle record: fully-decoded world-space position,
/// point size, and RGB color, plus trailing padding to a clean 32-byte
/// (two-float4) stride. Written by the CUDA gather kernel, read by the
/// D3D12 vertex shader via a StructuredBuffer -- see the file-level comment
/// above for the exact layout contract.
struct InteropParticleRecord {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float size = 0.45f;
    float r = 1.0f, g = 1.0f, b = 1.0f;
    float reserved = 0.0f;  // pads to 32 bytes (two float4s). cbuffer's
                             // 16-byte-boundary packing is an HLSL-enforced
                             // rule, but this struct lives in a
                             // StructuredBuffer, where D3D12 only requires the
                             // stride be a multiple of 4 bytes and match
                             // StructureByteStride exactly; 32 bytes here is a
                             // coalescing-friendly convention, not a
                             // language-enforced requirement.
};
// offsetof requires standard-layout; this type qualifies today (all-public,
// no bases, no virtuals). If a change breaks this (private members, a base
// class), the CUDA-kernel-writer / HLSL-reader offsetof contract in
// test_interop_particle_record_layout.cpp silently breaks -- catch it here
// first. Note this struct is NOT std::is_pod (the default member
// initializers make its default constructor non-trivial); the two
// properties that actually matter for this contract are trivially-copyable
// (safe to memcpy/write from a CUDA kernel) and standard-layout (licenses
// offsetof).
static_assert(std::is_trivially_copyable<InteropParticleRecord>::value,
              "interop particle record must stay trivially-copyable "
              "(written by a CUDA kernel, read by a D3D12 StructuredBuffer)");
static_assert(std::is_standard_layout<InteropParticleRecord>::value,
              "interop particle record must stay standard-layout "
              "(offsetof, used by the layout test, requires it)");
static_assert(sizeof(InteropParticleRecord) == 32,
              "must match the HLSL struct's 32-byte stride exactly");

/// Fixed header the gather kernel writes before the record array, mirroring
/// VisualParticleStagingHeader (visual_snapshot.h) but trimmed to just the
/// one count the D3D12 draw call needs.
struct InteropParticleHeader {
    std::uint32_t captured_count = 0;
};
static_assert(std::is_trivially_copyable<InteropParticleHeader>::value,
              "interop header must stay trivially-copyable");
static_assert(std::is_standard_layout<InteropParticleHeader>::value,
              "interop header must stay standard-layout");

}  // namespace ftd

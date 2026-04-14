// ============================================================================
// LatticeViewer_shaders.h — inline GLSL 330 core shader sources
// ============================================================================
//
// Two small programs used by LatticeViewer:
//
//   * voxelProgram — per-voxel points with state attribute (-1, 0, +1).
//                    The vertex shader picks a color from state and sizes
//                    points in screen space via gl_PointSize (distance
//                    attenuated). The fragment shader shapes each point as an
//                    antialiased disc (borrowed from engine/web/js/viewport.js
//                    particle rendering style; only the disc variant — the
//                    web dashboard's 7 shape variants are not required for
//                    Phase 4).
//   * lineProgram  — plain per-vertex color line rendering for the bounding
//                    box (and, eventually, RK4 field lines once the engine
//                    starts emitting vector field telemetry).
//
// Kept in a header so LatticeViewer.cpp is the only translation unit that
// pulls them in; no external .glsl asset files or .qrc entries needed.
// ----------------------------------------------------------------------------

#pragma once

namespace ftd::testrunner::shaders {

inline constexpr const char* kVoxelVertex = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in int  aState;

uniform mat4 uMVP;
uniform float uPointSize;

out vec3 vColor;
flat out int vState;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    // Distance-attenuated point size. Guard against near-zero w.
    float w = max(gl_Position.w, 0.001);
    gl_PointSize = uPointSize / w;

    if (aState > 0)      vColor = vec3(0.95, 0.25, 0.25); // + = red
    else if (aState < 0) vColor = vec3(0.25, 0.45, 0.95); // - = blue
    else                 vColor = vec3(0.40, 0.40, 0.40); // 0 = gray (skipped)

    vState = aState;
}
)GLSL";

inline constexpr const char* kVoxelFragment = R"GLSL(
#version 330 core
in vec3 vColor;
flat in int vState;
out vec4 FragColor;

void main() {
    // Antialiased disc — discard outside unit radius, smooth the edge.
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(d, d);
    if (r2 > 1.0) discard;
    float alpha = 1.0 - smoothstep(0.85, 1.0, r2);
    FragColor = vec4(vColor, alpha);
}
)GLSL";

inline constexpr const char* kLineVertex = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;
uniform mat4 uMVP;
out vec3 vColor;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vColor = aColor;
}
)GLSL";

inline constexpr const char* kLineFragment = R"GLSL(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() { FragColor = vec4(vColor, 1.0); }
)GLSL";

}  // namespace ftd::testrunner::shaders

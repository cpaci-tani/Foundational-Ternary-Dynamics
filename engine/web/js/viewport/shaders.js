/**
 * Centralized Shaders for FTD Web Frontend
 * ────────────────────────────────────────────────────────────────────
 *
 * Houses shared GLSL shader strings to ensure DRY compliance and
 * enable global shader optimizations.
 */

export const PARTICLE_FRAG = `
    uniform int shapeType;
    uniform float uOpacity;
    uniform float uGlow;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vec2 c = gl_PointCoord - vec2(0.5);
        float dist;

        if (shapeType == 1) {
            // Square
            dist = max(abs(c.x), abs(c.y));
            if (dist > 0.48) discard;
        } else if (shapeType == 2) {
            // Diamond
            dist = abs(c.x) + abs(c.y);
            if (dist > 0.5) discard;
        } else if (shapeType == 3) {
            // Star (5-pointed)
            float angle = atan(c.y, c.x);
            float r = length(c);
            float star = cos(5.0 * angle) * 0.15 + 0.35;
            if (r > star) discard;
            dist = r / star * 0.5;
        } else if (shapeType == 4) {
            // Triangle
            float x = c.x, y = c.y + 0.15;
            if (y > 0.35 || y < -0.35 + 0.7 * abs(x) / 0.4) discard;
            dist = length(c);
        } else if (shapeType == 5) {
            // Hexagon
            vec2 a = abs(c);
            dist = max(a.x * 0.866 + a.y * 0.5, a.y);
            if (dist > 0.45) discard;
            dist /= 0.45;
        } else if (shapeType == 6) {
            // Ring
            float r = length(c);
            if (r > 0.5 || r < 0.3) discard;
            dist = abs(r - 0.4) / 0.1;
        } else if (shapeType == 7) {
            // Cross
            float ax = abs(c.x), ay = abs(c.y);
            if (ax > 0.15 && ay > 0.15) discard;
            dist = max(ax, ay);
        } else {
            // Circle (default, shapeType == 0)
            dist = length(c);
            if (dist > 0.5) discard;
        }

        float alpha = 1.0 - smoothstep(0.15, 0.5, dist);
        float glow = exp(-dist * dist * 4.0) * uGlow;
        gl_FragColor = vec4(vColor + glow, alpha * alpha * uOpacity);
    }
`;

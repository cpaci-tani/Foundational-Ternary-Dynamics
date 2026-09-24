import { FRACTAL_VOLUME_GLSL } from './fractal-volumes.js';

/**
 * Bounded, distant-radiance fractals for the Observer shell.
 * These are decorative angular fields, not physical geometry or new ray paths.
 * Their only clock is the supplied session time; a zero rate is exactly static.
 * All styles cover the full sphere without allocating scene objects or textures.
 */
export const FRACTAL_GLSL = `
const float FRACTAL_PI = 3.141592653589793;

mat2 fractalTurn(float angle) {
    float c = cos(angle), s = sin(angle);
    return mat2(c, -s, s, c);
}

vec3 fractalPalette(float value, vec3 tint) {
    vec3 spectrum = .52 + .48 * cos(6.28318530718 * (value + vec3(0., .23, .52)));
    return mix(spectrum, max(tint, vec3(.015)), .38);
}

// A quadratic Julia set with smooth escape shading and an orbit-trap interior.
// Longitude enters periodically, so there is no equirectangular seam behind us.
vec3 fractalJulia(vec3 direction, float phase, float seedPhase, vec3 tint, float density) {
    float longitude = atan(direction.z, direction.x);
    float latitude = asin(clamp(direction.y, -.999999, .999999));
    vec2 z = vec2(sin(longitude * 2. + seedPhase), latitude * .9);
    z *= mix(1.15, 1.85, clamp(density / 4., 0., 1.));
    z += vec2(.12 * cos(phase * .13), .1 * sin(phase * .17));
    vec2 parameter = vec2(-.745 + .027 * sin(phase * .19 + seedPhase),
                          .186 + .038 * cos(phase * .13 - seedPhase));
    float trap = 1e3, escape = 32.;
    float radiusSquared = 0.;
    for (int iteration = 0; iteration < 32; iteration++) {
        z = vec2(z.x * z.x - z.y * z.y, 2. * z.x * z.y) + parameter;
        radiusSquared = dot(z, z);
        trap = min(trap, abs(radiusSquared - .36));
        if (radiusSquared > 64.) {
            escape = float(iteration) + 1. - log2(max(1., .5 * log(radiusSquared)));
            break;
        }
    }
    float orbitGlow = exp(-trap * 18.);
    float interior = step(31.5, escape);
    float filament = .5 + .5 * cos(escape * 2.1 + seedPhase);
    vec3 exterior = fractalPalette(escape * .037 + phase * .008, tint);
    exterior *= .055 + .48 * pow(clamp(escape / 32., 0., 1.), .62) + .1 * filament;
    vec3 inner = fractalPalette(.24 + orbitGlow * .22 + phase * .008, tint);
    inner *= .025 + orbitGlow * .65;
    return mix(exterior, inner, interior) + tint * pow(orbitGlow, 6.) * .25;
}

// A five-level Menger sponge sampled on a slowly moving cubical angular shell.
// Removing points with two middle ternary digits is the actual sponge rule.
// Derivative-sized boundaries keep its smallest windows legible while moving.
vec3 fractalMenger(vec3 direction, float phase, float seedPhase, vec3 tint, float density) {
    float major = max(max(abs(direction.x), abs(direction.y)), abs(direction.z));
    vec3 q = direction / max(major, .0001) * .485 + .5;
    q += .065 * vec3(sin(phase * .17 + seedPhase), cos(phase * .13), sin(phase * .11));
    q *= mix(.8, 1.55, clamp(density / 4., 0., 1.));
    float occupied = 1., edgeGlow = 0., depth = 0., weight = 1.;
    for (int iteration = 0; iteration < 5; iteration++) {
        vec3 cell = fract(q);
        vec3 edge = min(abs(cell - 1. / 3.), abs(cell - 2. / 3.));
        float nearestEdge = min(min(edge.x, edge.y), edge.z);
        float antialias = max(.0025, min(.12, fwidth(nearestEdge)));
        vec3 middle = smoothstep(vec3(1. / 3. - antialias), vec3(1. / 3. + antialias), cell)
                    * (1. - smoothstep(vec3(2. / 3. - antialias), vec3(2. / 3. + antialias), cell));
        float carved = max(max(middle.x * middle.y, middle.y * middle.z), middle.x * middle.z);
        edgeGlow += occupied * (1. - smoothstep(antialias, antialias * 2.5, nearestEdge)) * weight;
        depth += occupied * carved * float(iteration + 1);
        occupied *= 1. - carved;
        q = cell * 3.;
        weight *= .7;
    }
    vec3 openings = fractalPalette(.58 + depth * .065 + phase * .006, tint) * (.018 + depth * .015);
    vec3 lattice = fractalPalette(.12 + depth * .055, tint) * (.2 + occupied * .25);
    return mix(openings, lattice, occupied) + fractalPalette(.36 + seedPhase * .1, tint) * edgeGlow * .6;
}

// Seven bounded box/inversion folds form a kaleidoscopic iterated field.
// The clamp is part of the map: it avoids singular inversion and unbounded work.
vec3 fractalKaleidoscope(vec3 direction, float phase, float seedPhase, vec3 tint, float density) {
    float longitude = atan(direction.z, direction.x);
    vec2 z = vec2(sin(longitude * 3. + seedPhase), direction.y * 1.7);
    z *= mix(1.2, 2.4, clamp(density / 4., 0., 1.));
    vec2 translation = vec2(1.08 + .13 * sin(phase * .21), .82 + .12 * cos(phase * .17));
    mat2 turn = fractalTurn(.43 + .16 * sin(phase * .09 + seedPhase));
    float trap = 1e3, accumulated = 0., weight = 1.;
    for (int iteration = 0; iteration < 7; iteration++) {
        z = abs(z);
        if (z.x < z.y) z = z.yx;
        z = turn * (z * 1.9 - translation);
        z /= clamp(dot(z, z), .32, 1.5);
        float distanceToFold = min(abs(z.x), abs(z.y));
        trap = min(trap, distanceToFold);
        accumulated += exp(-distanceToFold * 7.) * weight;
        weight *= .72;
    }
    float lace = exp(-trap * 30.);
    vec3 color = fractalPalette(accumulated * .16 + phase * .009, tint);
    return color * (.035 + accumulated * .27) + fractalPalette(.7 + seedPhase * .06, tint) * lace * .42;
}

${FRACTAL_VOLUME_GLSL}

vec3 fractalEnvironment(vec3 direction, int style, float time, float rate, float seed, vec3 tint, float density, float detail) {
    // Seed affects preparation only; neither frame count nor wall-clock time enters.
    float seedPhase = mod(abs(seed), 65521.) * .00137;
    float phase = time * rate;
    direction.xz = fractalTurn(phase * .035 + seedPhase) * direction.xz;
    direction.yz = fractalTurn(.16 * sin(phase * .07 + seedPhase)) * direction.yz;
    if (style == 0) return fractalJulia(direction, phase, seedPhase, tint, density);
    if (style == 1) return fractalMenger(direction, phase, seedPhase, tint, density);
    if (style == 2) return fractalKaleidoscope(direction, phase, seedPhase, tint, density);
    return fractalVolumeEnvironment(direction, style, phase, seedPhase, tint, density, clamp(detail, 0., 1.));
}
`;

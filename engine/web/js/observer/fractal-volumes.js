/**
 * Bounded ray-marched distant radiance. These shapes are decorative presentations
 * of finite fractal iterations, never collision geometry or spacetime matter.
 * The repeat domain surrounds every viewing direction and has no camera seam.
 */
export const FRACTAL_VOLUME_GLSL = `
// Each estimator returns (presentation distance estimate, orbit trap).
// Explicit escape limits keep both coordinates and derivatives finite.
vec2 fractalBulbDistance(vec3 p, int levels) {
    vec3 z = p;
    float derivative = 1., radius = 0., trap = 4.;
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        radius = length(z);
        trap = min(trap, abs(radius - .8));
        if (radius > 3.) break;
        float safeRadius = max(radius, .000001);
        float theta = acos(clamp(z.z / safeRadius, -1., 1.));
        float phi = atan(z.y, z.x + .0000001);
        float power7 = pow(safeRadius, 7.);
        derivative = min(power7 * 8. * derivative + 1., 1e18);
        float power8 = power7 * safeRadius;
        theta *= 8.; phi *= 8.;
        z = power8 * vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)) + p;
    }
    radius = max(length(z), .000001);
    return vec2(max(0., .5 * log(radius) * radius / max(derivative, .000001)), trap);
}

vec2 fractalBoxDistance(vec3 p, int levels) {
    vec3 z = p;
    float derivative = 1., trap = 4.;
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        z = 2. * clamp(z, -1., 1.) - z;
        float radius2 = dot(z, z);
        trap = min(trap, min(abs(z.x), min(abs(z.y), abs(z.z))));
        float fold = clamp(1. / max(radius2, .25), 1., 4.);
        z *= fold;
        derivative *= fold;
        z = -1.8 * z + p;
        derivative = derivative * 1.8 + 1.;
        if (dot(z, z) > 1e12) break;
    }
    return vec2(length(z) / max(derivative, 1.), trap);
}

vec2 fractalTetraDistance(vec3 p, int levels) {
    vec3 z = p;
    float scale = 1., trap = 4.;
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        // Reflections in three diagonal planes generate tetrahedral symmetry.
        if (z.x + z.y < 0.) z.xy = -z.yx;
        if (z.x + z.z < 0.) z.xz = -z.zx;
        if (z.y + z.z < 0.) z.yz = -z.zy;
        trap = min(trap, min(abs(z.x + z.y), abs(z.y + z.z)) / scale);
        z = 2. * z - vec3(1.);
        scale *= 2.;
    }
    float planes = max(max(-z.x - z.y - z.z, -z.x + z.y + z.z),
                       max(z.x - z.y + z.z, z.x + z.y - z.z));
    return vec2(max(0., (planes - 1.) * .57735026919 / scale), trap);
}

vec2 fractalApollonianDistance(vec3 p, int levels, float phase) {
    vec3 z = p;
    float scale = 1., trap = 4.;
    float inversion = 1.35 + .06 * sin(phase * .09);
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        z = -1. + 2. * fract(.5 * z + .5);
        float radius2 = max(dot(z, z), .16);
        trap = min(trap, abs(radius2 - .6));
        float fold = inversion / radius2;
        z *= fold;
        scale *= fold;
    }
    return vec2(.22 * abs(z.y) / max(abs(scale), .000001), trap);
}

vec2 fractalQuaternionDistance(vec3 p, int levels, float phase) {
    vec4 z = vec4(p, .08 * sin(phase * .11));
    vec4 c = vec4(-.34, .62, .11 * sin(phase * .07), .08 * cos(phase * .09));
    float derivative = 1., trap = 4.;
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        float radius = length(z);
        trap = min(trap, length(z.zw));
        if (radius > 4.) break;
        derivative = min(2. * max(radius, .000001) * derivative, 1e18);
        z = vec4(z.x * z.x - dot(z.yzw, z.yzw), 2. * z.x * z.yzw) + c;
    }
    float radius = max(length(z), .000001);
    return vec2(max(0., .5 * radius * log(radius) / max(derivative, .000001)), trap);
}

// A box/sphere inversion construction inspired by Kleinian renderings. This is
// an artistic chamber field, not a claim to represent a particular limit set.
vec2 fractalChamberDistance(vec3 p, int levels, float phase) {
    vec3 z = p;
    float scale = 1., trap = 4.;
    vec3 chamber = vec3(1., .72 + .035 * sin(phase * .08), 1.);
    for (int i = 0; i < 10; i++) {
        if (i >= levels) break;
        z = 2. * clamp(z, -chamber, chamber) - z;
        float radius2 = max(dot(z, z), .18);
        trap = min(trap, abs(z.y) / max(scale, .000001));
        float fold = max(1. / radius2, 1.);
        z *= fold;
        scale *= fold;
        z.xz = fractalTurn(.16) * z.xz;
    }
    // A finite chamber pierced along all three axes. Using a bounded motif
    // avoids the unbounded slabs that otherwise expose the enclosing cut cap.
    vec3 outer = abs(z) - vec3(.76, .52, .76);
    float box = max(max(outer.x, outer.y), outer.z);
    float passages = min(length(z.xy), min(length(z.xz), length(z.yz))) - .34;
    float ribs = max(box, -passages);
    return vec2(max(0., ribs) * .65 / max(scale, .000001), trap);
}

vec2 fractalVolumeMap(vec3 point, int style, int levels, float phase, float density) {
    // The visitor lies in the clear space between neighboring cells. Repeating
    // fields are only sampled for this angular backdrop, never for scene hits.
    vec3 p = mod(point + 4., 8.) - 4.;
    float bound = length(p) - 2.35;
    if (style == 8) {
        // Intentional rounded architectural cells, not spherical slice caps.
        vec3 outer = abs(p) - vec3(1.95, 1.55, 1.95);
        bound = length(max(outer, vec3(0.))) + min(max(max(outer.x, outer.y), outer.z), 0.) - .08;
    }
    if (bound > .16) return vec2(bound, 1.);
    float size = mix(1.15, 1.65, clamp(density / 4., 0., 1.));
    vec3 q = p / size;
    vec2 sampleValue;
    if (style == 3) sampleValue = fractalBulbDistance(q, levels);
    // Fit the complete box-fold body inside its bound, including its corners.
    else if (style == 4) sampleValue = fractalBoxDistance(q * 2.6, levels) / vec2(2.6, 1.);
    else if (style == 5) sampleValue = fractalTetraDistance(q, levels);
    else if (style == 6) sampleValue = fractalApollonianDistance(q, levels, phase);
    else if (style == 7) sampleValue = fractalQuaternionDistance(q, levels, phase);
    else sampleValue = fractalChamberDistance(q, levels, phase);
    // The enclosing volume gives inversion fields a finite support and allows
    // cheap empty-space travel; min step and max travel bound the work further.
    sampleValue.x = max(bound, sampleValue.x * size);
    return vec2(clamp(sampleValue.x, 0., 8.), clamp(sampleValue.y, 0., 4.));
}

vec3 fractalVolumeNormal(vec3 p, int style, int levels, float phase, float density, float epsilon) {
    vec2 e = vec2(1., -1.) * .57735026919;
    vec3 normal = e.xyy * fractalVolumeMap(p + e.xyy * epsilon, style, levels, phase, density).x
                + e.yyx * fractalVolumeMap(p + e.yyx * epsilon, style, levels, phase, density).x
                + e.yxy * fractalVolumeMap(p + e.yxy * epsilon, style, levels, phase, density).x
                + e.xxx * fractalVolumeMap(p + e.xxx * epsilon, style, levels, phase, density).x;
    return normal / max(length(normal), .000001);
}

vec3 fractalVolumePigment(int style, float band, vec3 tint) {
    vec3 base, accent;
    if (style == 3) { base = vec3(.07, .16, .72); accent = vec3(1., .46, .055); }
    else if (style == 4) { base = vec3(.96, .32, .035); accent = vec3(.41, .08, .83); }
    else if (style == 5) { base = vec3(.025, .7, .86); accent = vec3(.91, .055, .42); }
    else if (style == 6) { base = vec3(.025, .47, .24); accent = vec3(1., .6, .055); }
    else if (style == 7) { base = vec3(.56, .055, .88); accent = vec3(.015, .78, 1.); }
    else { base = vec3(.67, .025, .105); accent = vec3(1., .58, .085); }
    // Preserve each motif's chromatic identity while retaining the user's tint.
    return mix(mix(base, accent, smoothstep(.28, .85, band)), max(tint, vec3(.005)), .14);
}

vec3 fractalVolumeEnvironment(vec3 direction, int style, float phase, float seedPhase,
                              vec3 tint, float density, float detail) {
    // Quality changes presentation sampling only: fixed limits and no clock or
    // scene-state dependencies. The legacy angular styles retain their budgets.
    int marchLimit = int(mix(28., 64., detail));
    int levels = int(mix(5., 9., detail));
    vec3 origin = vec3(.35, .22, 3.7);
    float travel = 0., glow = 0.;
    vec2 sampleValue = vec2(1.);
    bool hit = false;
    float epsilon = .003;
    for (int stepIndex = 0; stepIndex < 64; stepIndex++) {
        if (stepIndex >= marchLimit || travel > 34.) break;
        epsilon = mix(.006, .0022, detail) * (1. + travel * .06);
        sampleValue = fractalVolumeMap(origin + direction * travel, style, levels, phase, density);
        glow += exp(-sampleValue.x * 13.) * .007 * exp(-travel * .05);
        if (sampleValue.x < epsilon) { hit = true; break; }
        travel += clamp(sampleValue.x * .72, epsilon * .5, 3.);
    }
    float ribbon = .5 + .5 * sin(direction.y * 10. + direction.x * 4. + seedPhase);
    vec3 background = fractalVolumePigment(style, .15, tint) * (.0015 + .003 * ribbon);
    vec3 halo = fractalVolumePigment(style, .92, tint) * min(glow, .18);
    if (!hit) return background + halo * .45;
    vec3 position = origin + direction * travel;
    vec3 normal = fractalVolumeNormal(position, style, levels, phase, density, epsilon * 1.6);
    vec3 lightDirection = normalize(vec3(-.65, .8, -.2) - direction * .35);
    float diffuse = max(dot(normal, lightDirection), 0.);
    float fill = max(dot(normal, -lightDirection), 0.) * .055;
    float rim = pow(1. - max(dot(normal, -direction), 0.), 3.);
    vec3 halfVector = normalize(lightDirection - direction + vec3(.000001));
    float specular = pow(max(dot(normal, halfVector), 0.), 26.);
    float trap = sampleValue.y;
    float band = .5 + .5 * cos(trap * 19. + float(style));
    vec3 pigment = fractalVolumePigment(style, band, tint);
    float recess = .55 + .45 * smoothstep(0., .22, trap);
    vec3 surface = pigment * (.035 + diffuse * 1.25 + fill) * recess;
    surface += fractalVolumePigment(style, 1. - band, tint) * (rim * .23 + exp(-trap * 22.) * .075);
    surface += vec3(.8, .88, 1.) * specular * .5;
    float fog = exp(-travel * .045);
    return mix(background, surface, fog) + halo * .3;
}
`;

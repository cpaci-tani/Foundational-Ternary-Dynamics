/** Read-only volume presentation of native Scale-0 scalar samples.
 *
 * Samples are sparse regular voxel-centre anchors: absent anchors mean zero,
 * not missing telemetry. Explicit stride (and optional origin/grid count) is
 * required. The native non-interior grid is centre-anchored when origin is
 * omitted. No support is inferred from the nonzero sample positions.
 *
 * Half-float texture quantization and trilinear interpolation are visualization
 * only; this module neither evolves state nor supplies scientific reductions.
 * Optical density/color use a declared visual gamma (default 0.5). Source
 * values and the quantitative normalizer are never gamma-transformed.
 * One selected kind owns one volume. There is no animation loop or owner.
 */
import * as THREE from 'three';

const MAX_SIDE = 64;
const VERTEX = `
    varying vec3 vVolumePosition;
    void main() {
        vVolumePosition = position;
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
`;
const FRAGMENT = `
    precision highp float;
    precision highp sampler3D;
    uniform sampler3D uVolume;
    uniform sampler2D uRamp;
    uniform vec3 uCamera;
    uniform vec3 uDirection;
    uniform bool uOrthographic;
    uniform float uLatticeSize;
    uniform float uOrigin;
    uniform float uStride;
    uniform float uSide;
    uniform float uValueScale;
    uniform float uNormalizer;
    uniform float uOpacity;
    uniform float uThreshold;
    uniform float uGamma;
    uniform bool uSigned;
    varying vec3 vVolumePosition;

    void main() {
        vec3 ray = uOrthographic ? normalize(uDirection)
            : normalize(vVolumePosition - uCamera);
        vec3 start = uOrthographic ? vVolumePosition - ray * 3.0 : uCamera;
        // Preserve the sign for slab intersections and avoid 0 * infinity.
        vec3 safeRay = mix(vec3(-1.0), vec3(1.0), step(vec3(0.0), ray))
            * max(abs(ray), vec3(1e-7));
        vec3 a = -start / safeRay;
        vec3 b = (vec3(1.0) - start) / safeRay;
        vec3 lo = min(a, b), hi = max(a, b);
        float entry = max(max(lo.x, lo.y), lo.z);
        float exitAt = min(min(hi.x, hi.y), hi.z);
        entry = max(entry, 0.0); // Cameras inside the box start here.
        if (exitAt <= entry) discard;

        float delta = (exitAt - entry) / 96.0;
        vec4 result = vec4(0.0);
        for (int i = 0; i < 96; ++i) {
            vec3 p = start + ray * (entry + (float(i) + 0.5) * delta);
            vec3 uvw = ((p * uLatticeSize - vec3(uOrigin + 0.5))
                / uStride + vec3(0.5)) / uSide;
            // Interior samplers do not observe the outer shell. ClampToEdge
            // must not extend their last observed value into that shell.
            if (any(lessThan(uvw, vec3(0.0))) || any(greaterThan(uvw, vec3(1.0)))) continue;
            float value = texture(uVolume, uvw).r * uValueScale / uNormalizer;
            float magnitude = pow(clamp(abs(value), 0.0, 1.0), uGamma);
            if (magnitude <= uThreshold) continue;
            float rampAt = uSigned ? 0.5 + 0.5 * sign(value) * magnitude : magnitude;
            vec3 color = texture2D(uRamp, vec2(rampAt, 0.5)).rgb;
            float alpha = 1.0 - exp(-12.0 * uOpacity * magnitude * delta);
            result.rgb += (1.0 - result.a) * color * alpha;
            result.a += (1.0 - result.a) * alpha;
            if (result.a > 0.995) break;
        }
        if (result.a <= 0.0001) discard;
        // NormalBlending expects unpremultiplied RGB.
        gl_FragColor = vec4(result.rgb / result.a, result.a);
    }
`;

function gridFor(frame, L) {
    const supplied = frame?.sampleGrid;
    const stride = frame?.effectiveStride ?? supplied?.stride;
    if (!Number.isInteger(L) || L < 1 || !Number.isInteger(stride)
        || stride < 1 || stride > L) throw new TypeError('Explicit native sample stride required');
    const origin = frame.origin ?? supplied?.origin ?? Math.floor((L - 1) / 2) % stride;
    if (!Number.isInteger(origin) || origin < 0 || origin >= L) throw new TypeError('Invalid sample origin');
    if (supplied?.stride != null && supplied.stride !== stride
        || supplied?.origin != null && supplied.origin !== origin) throw new TypeError('Conflicting sample grid');
    const side = supplied?.count ?? Math.floor((L - 1 - origin) / stride) + 1;
    if (!Number.isInteger(side) || side < 1 || side > MAX_SIDE
        || origin + (side - 1) * stride >= L) throw new TypeError('Volume grid exceeds finite support');
    if (!Number.isInteger(frame.count) || frame.count < 0 || frame.count > side ** 3
        || !frame.positions || !frame.values || frame.positions.length < frame.count * 3
        || frame.values.length < frame.count) throw new TypeError('Invalid scalar sample lengths');
    return { L, stride, origin, side };
}

function defaultRamp(t, out) {
    const v = Math.max(0, Math.min(1, t));
    out[0] = Math.min(1, 2.0 * v);
    out[1] = Math.max(0.08, Math.min(1, 1.7 * v - 0.35));
    out[2] = Math.max(0.05, 0.45 - 0.35 * v);
}

/**
 * show(kind,true) selects the sole active kind; updates for other kinds are
 * ignored. update returns false and clears stale pixels for invalid input.
 * options: {ramp(t,out,offset), signed, opacity:[0,1], threshold:[0,1],
 *           gamma:(0,4], normalizer}. Default gamma=0.5; threshold is applied
 * after gamma (default 0.01 corresponds to 0.0001 of the raw normalizer).
 * Opacity persists between updates; setOpacity() only updates a uniform.
 * A signed ramp receives [-1,1]. A magnitude ramp receives [0,1].
 */
export function createScalarVolumeRenderer(scene, getLatticeSize) {
    const group = new THREE.Group();
    group.name = 'scale0-scalar-volume';
    group.visible = false;
    scene.add(group);
    const geometry = new THREE.BoxGeometry(1, 1, 1).translate(0.5, 0.5, 0.5);
    const rampData = new Uint8Array(256 * 4);
    const rampTexture = new THREE.DataTexture(rampData, 256, 1, THREE.RGBAFormat);
    rampTexture.minFilter = rampTexture.magFilter = THREE.LinearFilter;
    rampTexture.generateMipmaps = false;
    const uniforms = {
        uVolume: { value: null }, uRamp: { value: rampTexture },
        uCamera: { value: new THREE.Vector3() }, uDirection: { value: new THREE.Vector3() },
        uOrthographic: { value: false }, uLatticeSize: { value: 1 },
        uOrigin: { value: 0 }, uStride: { value: 1 }, uSide: { value: 1 },
        uValueScale: { value: 1 }, uNormalizer: { value: 1 },
        uOpacity: { value: 0.75 }, uThreshold: { value: 0.01 },
        uGamma: { value: 0.5 }, uSigned: { value: false },
    };
    const material = new THREE.ShaderMaterial({
        vertexShader: VERTEX, fragmentShader: FRAGMENT, uniforms,
        side: THREE.BackSide, transparent: true, depthWrite: false,
        // The exit face is behind contained particles. Testing its depth would
        // discard the entire ray, including volume in front of those particles.
        depthTest: false, blending: THREE.NormalBlending,
    });
    const mesh = new THREE.Mesh(geometry, material);
    mesh.name = 'scale0-scalar-volume-raymarch';
    mesh.frustumCulled = false;
    mesh.renderOrder = 3;
    group.add(mesh);
    const inverse = new THREE.Matrix4();
    mesh.onBeforeRender = (_renderer, _scene, camera) => {
        inverse.copy(mesh.matrixWorld).invert();
        uniforms.uCamera.value.setFromMatrixPosition(camera.matrixWorld).applyMatrix4(inverse);
        camera.getWorldDirection(uniforms.uDirection.value).transformDirection(inverse);
        uniforms.uOrthographic.value = !!camera.isOrthographicCamera;
    };
    let texture = null, dense = null, occupied = null;
    let activeKind = null, dataKind = null, ready = false, disposed = false;
    let previousRamp = null, previousSigned = null, uploadCount = 0;
    let frameSize = null;

    function hideData(reason = 'empty') {
        ready = false; dataKind = null; group.visible = false;
        group.userData.status = reason;
    }

    function update(kind, frame, options = {}) {
        if (disposed || kind !== activeKind) return false;
        try {
            const grid = gridFor(frame, Number(getLatticeSize()));
            const { L, side, origin, stride } = grid, size = side ** 3;
            if (!dense || dense.length !== size) {
                dense = new Float32Array(size); occupied = new Uint8Array(size);
            } else { dense.fill(0); occupied.fill(0); }
            let max = 0;
            const signed = options.signed ?? frame.signed ?? false;
            for (let i = 0; i < frame.count; ++i) {
                const value = Number(frame.values[i]);
                if (!Number.isFinite(value) || Math.abs(value) > 3.4028234663852886e38
                    || (!signed && value < 0)) throw new TypeError('Invalid scalar value');
                let index = 0, multiplier = 1;
                for (let axis = 0; axis < 3; ++axis) {
                    const coordinate = (Number(frame.positions[i * 3 + axis]) - origin - 0.5) / stride;
                    const integer = Math.round(coordinate);
                    if (!Number.isFinite(coordinate) || Math.abs(coordinate - integer) > 1e-5
                        || integer < 0 || integer >= side) throw new TypeError('Scalar position is off the native grid');
                    index += integer * multiplier; multiplier *= side;
                }
                if (occupied[index]) throw new TypeError('Duplicate native sample anchor');
                occupied[index] = 1; dense[index] = value; max = Math.max(max, Math.abs(value));
            }
            let fresh = false;
            if (!texture || texture.image.width !== side) {
                texture?.dispose();
                texture = new THREE.Data3DTexture(new Uint16Array(size), side, side, side);
                texture.format = THREE.RedFormat;
                texture.type = THREE.HalfFloatType;
                texture.minFilter = texture.magFilter = THREE.LinearFilter;
                texture.wrapS = texture.wrapT = texture.wrapR = THREE.ClampToEdgeWrapping;
                texture.unpackAlignment = 1; texture.generateMipmaps = false;
                uniforms.uVolume.value = texture; fresh = true;
            }
            const scale = max || 1, bytes = texture.image.data;
            for (let i = 0; i < size; ++i) {
                const encoded = THREE.DataUtils.toHalfFloat(dense[i] / scale);
                if (bytes[i] !== encoded) { bytes[i] = encoded; fresh = true; }
            }
            if (fresh) { texture.needsUpdate = true; uploadCount++; }
            const ramp = typeof options.ramp === 'function' ? options.ramp : defaultRamp;
            if (ramp !== previousRamp || signed !== previousSigned) {
                const rgb = [0, 0, 0];
                for (let i = 0; i < 256; ++i) {
                    const t = i / 255;
                    if (signed && ramp === defaultRamp) {
                        rgb[0] = t; rgb[1] = 0.15 + 0.5 * (1 - Math.abs(2 * t - 1)); rgb[2] = 1 - t;
                    } else ramp(signed ? 2 * t - 1 : t, rgb, 0);
                    for (let axis = 0; axis < 3; ++axis) {
                        if (!Number.isFinite(rgb[axis])) throw new TypeError('Invalid volume ramp');
                        rampData[i * 4 + axis] = Math.round(255 * Math.max(0, Math.min(1, rgb[axis])));
                    }
                    rampData[i * 4 + 3] = 255;
                }
                rampTexture.needsUpdate = true; previousRamp = ramp; previousSigned = signed;
            }
            const normalizer = options.normalizer ?? frame.normalizer ?? scale;
            if (!Number.isFinite(normalizer) || normalizer < 0) throw new TypeError('Invalid volume normalizer');
            const opacity = options.opacity ?? uniforms.uOpacity.value;
            const threshold = options.threshold ?? 0.01, gamma = options.gamma ?? 0.5;
            if (!Number.isFinite(opacity) || opacity < 0 || opacity > 1
                || !Number.isFinite(threshold) || threshold < 0 || threshold > 1
                || !Number.isFinite(gamma) || gamma <= 0 || gamma > 4) throw new TypeError('Invalid volume transfer options');
            uniforms.uLatticeSize.value = L; uniforms.uOrigin.value = origin;
            uniforms.uStride.value = stride; uniforms.uSide.value = side;
            uniforms.uValueScale.value = scale;
            uniforms.uNormalizer.value = normalizer > 0 ? normalizer : scale;
            uniforms.uOpacity.value = opacity; uniforms.uThreshold.value = threshold;
            uniforms.uGamma.value = gamma;
            uniforms.uSigned.value = !!signed;
            mesh.scale.setScalar(L); frameSize = L;
            ready = max > 0; dataKind = kind;
            group.visible = ready && opacity > 0;
            delete group.userData.reason;
            Object.assign(group.userData, { status: ready ? 'available' : 'empty', kind, grid, sampleCount: frame.count,
                transfer: { gamma, opacity, threshold, extinction: 12, visualOnly: true } });
            return true;
        } catch (error) {
            hideData('invalid'); group.userData.reason = error.message; return false;
        }
    }

    function show(kind, on) {
        if (disposed) return;
        if (on) {
            activeKind = kind;
            group.visible = ready && dataKind === kind && frameSize === Number(getLatticeSize())
                && uniforms.uOpacity.value > 0;
        } else if (activeKind === kind) { activeKind = null; group.visible = false; }
    }
    function setOpacity(value) {
        if (disposed || !Number.isFinite(value) || value < 0 || value > 1) return false;
        uniforms.uOpacity.value = value;
        if (group.userData.transfer) group.userData.transfer.opacity = value;
        group.visible = ready && dataKind === activeKind && activeKind != null
            && frameSize === Number(getLatticeSize()) && value > 0;
        return true;
    }
    function clear() {
        hideData(); frameSize = null;
        delete group.userData.kind; delete group.userData.grid;
        delete group.userData.sampleCount; delete group.userData.reason;
    }
    function dispose() {
        if (disposed) return;
        disposed = true; clear(); activeKind = null;
        texture?.dispose(); rampTexture.dispose(); material.dispose(); geometry.dispose();
        scene.remove(group); mesh.onBeforeRender = () => {};
        dense = null; occupied = null;
    }
    return { update, show, setOpacity, clear, dispose, group,
        get uploadCount() { return uploadCount; },
        get activeKind() { return activeKind; } };
}

/**
 * Live Scale 4 physics overlays.
 *
 * All coordinates remain in the renderer's undistorted simulation gauge:
 * AU/M_sun/yr for astronomical scenarios and natural units for Figure-8.
 * Vector lengths and point colors are explicitly presentation-normalized;
 * directions, field samples, Hill radii, Roche limits, and collision radii are
 * computed from the live state.
 */

import * as THREE from 'three';
import { fluidRocheLimitAu, hillRadiusAu } from './config/solar-system-physics.js?v=4';

const FIELD_RESOLUTION = 33;
const FIELD_POINT_COUNT = FIELD_RESOLUTION * FIELD_RESOLUTION;
const FIELD_REFRESH_MS = 160;
const EPSILON = 1e-18;

function simToWorld(x, y, z, target = new THREE.Vector3()) {
    return target.set(x, z, y);
}

function finiteVector(x, y, z) {
    return Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z);
}

function bodyPosition(body, target = new THREE.Vector3()) {
    return simToWorld(Number(body?.x) || 0, Number(body?.y) || 0, Number(body?.z) || 0, target);
}

function centerAndExtent(bodies) {
    if (!bodies.length) return { x: 0, y: 0, z: 0, extent: 1 };
    let totalMass = 0;
    let x = 0;
    let y = 0;
    let z = 0;
    for (const body of bodies) {
        const mass = Math.max(0, Number(body.mass) || 0);
        totalMass += mass;
        x += (Number(body.x) || 0) * mass;
        y += (Number(body.y) || 0) * mass;
        z += (Number(body.z) || 0) * mass;
    }
    if (totalMass > 0) {
        x /= totalMass;
        y /= totalMass;
        z /= totalMass;
    } else {
        x = bodies.reduce((sum, body) => sum + (Number(body.x) || 0), 0) / bodies.length;
        y = bodies.reduce((sum, body) => sum + (Number(body.y) || 0), 0) / bodies.length;
        z = bodies.reduce((sum, body) => sum + (Number(body.z) || 0), 0) / bodies.length;
    }
    let extent = 0;
    for (const body of bodies) {
        extent = Math.max(extent, Math.hypot(
            (Number(body.x) || 0) - x,
            (Number(body.y) || 0) - y,
            (Number(body.z) || 0) - z,
        ));
    }
    return { x, y, z, extent: Math.max(extent * 1.12, 0.25) };
}

function vectorMagnitude(body, prefix) {
    return Math.hypot(
        Number(body?.[`${prefix}x`]) || 0,
        Number(body?.[`${prefix}y`]) || 0,
        Number(body?.[`${prefix}z`]) || 0,
    );
}

function disposeArrowBatch(batch) {
    batch.geometry.dispose();
    batch.material.dispose();
}

export class PlanetaryPhysicsOverlays {
    constructor(parentGroup) {
        this.group = new THREE.Group();
        this.group.name = 'scale4-physics-overlays';
        parentGroup.add(this.group);

        this.enabled = {
            gravityField: true,
            accelerationVectors: false,
            velocityVectors: false,
            hillSpheres: false,
            rocheLimits: false,
            collisionShells: false,
        };
        this._lastData = null;
        this._selectedId = -1;
        this._lastFieldRefresh = -Infinity;
        this._fieldStatus = { samples: 0, min: 0, max: 0, sourceCount: 0 };

        this._buildField();
        this._hillGroup = new THREE.Group();
        this._hillGroup.name = 'scale4-hill-domains';
        this._rocheGroup = new THREE.Group();
        this._rocheGroup.name = 'scale4-roche-limits';
        this._collisionGroup = new THREE.Group();
        this._collisionGroup.name = 'scale4-collision-shells';
        this.group.add(this._hillGroup, this._rocheGroup, this._collisionGroup);

        const sphere = new THREE.SphereGeometry(1, 20, 12);
        this._wireSphereGeometry = new THREE.WireframeGeometry(sphere);
        sphere.dispose();
        this._hillMaterial = new THREE.LineBasicMaterial({ color: 0x60a5fa, transparent: true, opacity: 0.30, depthWrite: false });
        this._rocheMaterial = new THREE.LineBasicMaterial({ color: 0xfb923c, transparent: true, opacity: 0.42, depthWrite: false });
        this._collisionMaterial = new THREE.LineBasicMaterial({ color: 0xf472b6, transparent: true, opacity: 0.68, depthWrite: false, depthTest: false });
        this._hillMeshes = new Map();
        this._rocheMeshes = new Map();
        this._collisionMeshes = new Map();

        this._accelerationBatch = this._makeArrowBatch('scale4-net-acceleration-vectors', 0xfbbf24);
        this._velocityBatch = this._makeArrowBatch('scale4-velocity-vectors', 0x22d3ee);
        this.group.add(this._accelerationBatch, this._velocityBatch);
    }

    _buildField() {
        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(FIELD_POINT_COUNT * 3), 3));
        geometry.setAttribute('color', new THREE.BufferAttribute(new Float32Array(FIELD_POINT_COUNT * 3), 3));
        geometry.setDrawRange(0, 0);
        const material = new THREE.PointsMaterial({
            size: 0.06,
            vertexColors: true,
            transparent: true,
            opacity: 0.58,
            sizeAttenuation: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });
        this._field = new THREE.Points(geometry, material);
        this._field.name = 'scale4-gravity-field-strength';
        this._field.frustumCulled = false;
        this._field.renderOrder = -1;
        this._field.userData = {
            epistemicStatus: 'PARAMETRIC',
            meaning: 'Sampled Newtonian acceleration magnitude on the system reference plane; gravity has no finite cutoff.',
            displayMapping: 'logarithmic color and point-size normalization',
        };
        this.group.add(this._field);
    }

    _makeArrowBatch(name, color) {
        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(0), 3));
        const material = new THREE.LineBasicMaterial({ color, transparent: true, opacity: 0.88, depthWrite: false, depthTest: false });
        const lines = new THREE.LineSegments(geometry, material);
        lines.name = name;
        lines.renderOrder = 8;
        lines.userData = {
            epistemicStatus: 'PARAMETRIC',
            displayMapping: 'direction is physical; length is logarithmically normalized for visibility',
        };
        return lines;
    }

    setEnabled(key, visible) {
        if (!(key in this.enabled)) return false;
        this.enabled[key] = !!visible;
        this.update(this._lastData, this._selectedId, true);
        return true;
    }

    setSelectedBody(id) {
        this._selectedId = Number.isFinite(Number(id)) ? Number(id) : -1;
        this.update(this._lastData, this._selectedId, true);
    }

    update(data, selectedId = this._selectedId, force = false) {
        if (!data) return;
        this._lastData = data;
        this._selectedId = Number.isFinite(Number(selectedId)) ? Number(selectedId) : -1;
        const bodies = (data.bodies || []).filter((body) => Number(body.mass) > 0);
        const byId = new Map(bodies.map((body) => [Number(body.id), body]));
        const selected = byId.get(this._selectedId) || null;
        const visibleBodies = selected ? [selected] : bodies;
        const systemGauge = centerAndExtent(bodies);
        let gauge = systemGauge;
        if (selected) {
            const parent = byId.get(Number(selected.parentId));
            const hillRadius = parent && Number(selected.mass) / Number(parent.mass) <= 0.1
                ? hillRadiusAu(selected, parent)
                : 0;
            const localExtent = Math.max(Number(selected.r) * 12 || 0, hillRadius * 1.18);
            if (localExtent > 0) {
                gauge = {
                    x: Number(selected.x) || 0,
                    y: Number(selected.y) || 0,
                    z: Number(selected.z) || 0,
                    extent: localExtent,
                };
            }
        }
        const gravityActive = data.physics?.newtonianGravity !== false;

        const now = typeof performance !== 'undefined' ? performance.now() : Date.now();
        if (this.enabled.gravityField && gravityActive && (force || now - this._lastFieldRefresh >= FIELD_REFRESH_MS)) {
            this._updateGravityField(selected ? [selected] : bodies, gauge, Number(data.gravityConstant) || 1);
            this._lastFieldRefresh = now;
        }
        this._field.visible = this.enabled.gravityField && gravityActive && bodies.length > 0;

        this._updateArrowBatch(this._accelerationBatch, visibleBodies, 'a', gauge.extent, this.enabled.accelerationVectors);
        this._updateArrowBatch(this._velocityBatch, visibleBodies, 'v', gauge.extent, this.enabled.velocityVectors);
        this._updateHillSpheres(visibleBodies, byId);
        this._updateRocheLimits(
            visibleBodies,
            byId,
            data.scenario !== 'planetary-threebody' && data.physics?.rocheDisruption !== false,
        );
        this._updateCollisionShells(visibleBodies, data.physics?.finiteBodyCollisions !== false);
    }

    _updateGravityField(sourceBodies, gauge, G) {
        const position = this._field.geometry.getAttribute('position');
        const color = this._field.geometry.getAttribute('color');
        const magnitudes = new Float64Array(FIELD_POINT_COUNT);
        const step = (gauge.extent * 2) / (FIELD_RESOLUTION - 1);
        let min = Infinity;
        let max = 0;
        let index = 0;
        for (let row = 0; row < FIELD_RESOLUTION; row++) {
            const simY = gauge.y - gauge.extent + row * step;
            for (let column = 0; column < FIELD_RESOLUTION; column++, index++) {
                const simX = gauge.x - gauge.extent + column * step;
                let ax = 0;
                let ay = 0;
                let az = 0;
                for (const body of sourceBodies) {
                    const dx = (Number(body.x) || 0) - simX;
                    const dy = (Number(body.y) || 0) - simY;
                    const dz = (Number(body.z) || 0) - gauge.z;
                    const softening = Math.max(Number(body.r) || 0, step * 0.04, 1e-12);
                    const r2 = Math.max(dx * dx + dy * dy + dz * dz, softening * softening);
                    const scale = G * (Number(body.mass) || 0) / (r2 * Math.sqrt(r2));
                    ax += dx * scale;
                    ay += dy * scale;
                    az += dz * scale;
                }
                const magnitude = Math.max(Math.hypot(ax, ay, az), EPSILON);
                magnitudes[index] = magnitude;
                min = Math.min(min, magnitude);
                max = Math.max(max, magnitude);
                position.setXYZ(index, simX, gauge.z, simY);
            }
        }
        const logMin = Math.log10(Math.max(min, EPSILON));
        const logSpan = Math.max(Math.log10(Math.max(max, EPSILON)) - logMin, 1e-9);
        const cool = new THREE.Color(0x163b78);
        const mid = new THREE.Color(0x22d3ee);
        const hot = new THREE.Color(0xfbbf24);
        const work = new THREE.Color();
        for (let i = 0; i < FIELD_POINT_COUNT; i++) {
            const t = THREE.MathUtils.clamp((Math.log10(magnitudes[i]) - logMin) / logSpan, 0, 1);
            if (t < 0.58) work.copy(cool).lerp(mid, t / 0.58);
            else work.copy(mid).lerp(hot, (t - 0.58) / 0.42);
            color.setXYZ(i, work.r, work.g, work.b);
        }
        position.needsUpdate = true;
        color.needsUpdate = true;
        this._field.geometry.setDrawRange(0, FIELD_POINT_COUNT);
        this._field.material.size = THREE.MathUtils.clamp(gauge.extent * 0.008, 0.015, 0.20);
        this._fieldStatus = { samples: FIELD_POINT_COUNT, min, max, sourceCount: sourceBodies.length };
    }

    _updateArrowBatch(batch, bodies, prefix, extent, visible) {
        if (!visible || bodies.length === 0) {
            batch.visible = false;
            batch.geometry.setDrawRange(0, 0);
            return;
        }
        const candidates = bodies.filter((body) => {
            const x = Number(body?.[`${prefix}x`]);
            const y = Number(body?.[`${prefix}y`]);
            const z = Number(body?.[`${prefix}z`]);
            return finiteVector(x, y, z) && Math.hypot(x, y, z) > EPSILON;
        });
        if (!candidates.length) {
            batch.visible = false;
            batch.geometry.setDrawRange(0, 0);
            return;
        }
        const maxMagnitude = Math.max(...candidates.map((body) => vectorMagnitude(body, prefix)), EPSILON);
        const positions = new Float32Array(candidates.length * 18);
        const origin = new THREE.Vector3();
        const direction = new THREE.Vector3();
        const end = new THREE.Vector3();
        const perpendicular = new THREE.Vector3();
        let offset = 0;
        for (const body of candidates) {
            bodyPosition(body, origin);
            simToWorld(
                Number(body[`${prefix}x`]),
                Number(body[`${prefix}y`]),
                Number(body[`${prefix}z`]),
                direction,
            ).normalize();
            const normalized = Math.log1p(9 * vectorMagnitude(body, prefix) / maxMagnitude) / Math.log(10);
            const bodyRadius = Math.max(Number(body.r) || 0, 1e-12);
            const length = this._selectedId >= 0
                ? bodyRadius * 4.5
                : extent * (0.018 + 0.055 * normalized);
            end.copy(origin).addScaledVector(direction, length);
            perpendicular.crossVectors(direction, Math.abs(direction.y) < 0.9
                ? new THREE.Vector3(0, 1, 0)
                : new THREE.Vector3(1, 0, 0)).normalize();
            const headLength = length * 0.24;
            const headWidth = length * 0.11;
            const base = end.clone().addScaledVector(direction, -headLength);
            const wingA = base.clone().addScaledVector(perpendicular, headWidth);
            const wingB = base.clone().addScaledVector(perpendicular, -headWidth);
            for (const point of [origin, end, end, wingA, end, wingB]) {
                positions[offset++] = point.x;
                positions[offset++] = point.y;
                positions[offset++] = point.z;
            }
        }
        batch.geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
        batch.geometry.computeBoundingSphere();
        batch.geometry.setDrawRange(0, candidates.length * 6);
        batch.visible = true;
        batch.userData.vectorCount = candidates.length;
        batch.userData.maxMagnitude = maxMagnitude;
    }

    _wireObject(map, id, group, material, name) {
        let object = map.get(id);
        if (!object) {
            object = new THREE.LineSegments(this._wireSphereGeometry, material);
            object.name = name;
            object.frustumCulled = false;
            group.add(object);
            map.set(id, object);
        }
        return object;
    }

    _hideAll(map) {
        for (const object of map.values()) object.visible = false;
    }

    _updateHillSpheres(bodies, byId) {
        this._hideAll(this._hillMeshes);
        if (!this.enabled.hillSpheres) return;
        for (const body of bodies) {
            const parent = byId.get(Number(body.parentId));
            if (!parent || Number(body.mass) / Number(parent.mass) > 0.1) continue;
            const radius = hillRadiusAu(body, parent);
            if (!(radius > 0) || !Number.isFinite(radius)) continue;
            const object = this._wireObject(this._hillMeshes, body.id, this._hillGroup, this._hillMaterial, `Hill domain · ${body.name}`);
            bodyPosition(body, object.position);
            object.scale.setScalar(radius);
            object.visible = true;
            object.userData = { bodyId: body.id, radius, epistemicStatus: 'PARAMETRIC', meaning: 'Instantaneous circular restricted-three-body Hill approximation; not a gravity cutoff.' };
        }
    }

    _updateRocheLimits(bodies, byId, physicalUnits) {
        this._hideAll(this._rocheMeshes);
        if (!this.enabled.rocheLimits || !physicalUnits) return;
        for (const child of bodies) {
            const parent = byId.get(Number(child.parentId));
            if (!parent || child.rocheEligible === false) continue;
            const radius = fluidRocheLimitAu(parent, child);
            if (!(radius > 0) || !Number.isFinite(radius)) continue;
            const object = this._wireObject(this._rocheMeshes, child.id, this._rocheGroup, this._rocheMaterial, `Roche limit · ${child.name}`);
            bodyPosition(parent, object.position);
            object.scale.setScalar(radius);
            object.visible = true;
            object.userData = { childId: child.id, parentId: parent.id, radius, epistemicStatus: 'IMPOSED', meaning: 'Fluid Roche-limit approximation for this child density.' };
        }
    }

    _updateCollisionShells(bodies, kernelActive) {
        this._hideAll(this._collisionMeshes);
        if (!this.enabled.collisionShells || !kernelActive) return;
        for (const body of bodies) {
            const radius = Number(body.r);
            if (!(radius > 0) || !Number.isFinite(radius)) continue;
            const object = this._wireObject(this._collisionMeshes, body.id, this._collisionGroup, this._collisionMaterial, `Collision surface · ${body.name}`);
            bodyPosition(body, object.position);
            object.scale.setScalar(radius);
            object.visible = true;
            object.userData = { bodyId: body.id, radius, epistemicStatus: 'IMPOSED', meaning: 'Exact finite-body contact radius used by the collision kernel.' };
        }
    }

    getStatus() {
        const countVisible = (map) => [...map.values()].filter((object) => object.visible).length;
        return {
            enabled: { ...this.enabled },
            selectedOnly: this._selectedId >= 0,
            gravityField: { ...this._fieldStatus, visible: this._field.visible },
            accelerationVectors: this._accelerationBatch.visible ? this._accelerationBatch.userData.vectorCount || 0 : 0,
            velocityVectors: this._velocityBatch.visible ? this._velocityBatch.userData.vectorCount || 0 : 0,
            hillSpheres: countVisible(this._hillMeshes),
            rocheLimits: countVisible(this._rocheMeshes),
            collisionShells: countVisible(this._collisionMeshes),
        };
    }

    dispose() {
        this.group.parent?.remove(this.group);
        this._field.geometry.dispose();
        this._field.material.dispose();
        disposeArrowBatch(this._accelerationBatch);
        disposeArrowBatch(this._velocityBatch);
        this._wireSphereGeometry.dispose();
        this._hillMaterial.dispose();
        this._rocheMaterial.dispose();
        this._collisionMaterial.dispose();
        this._hillMeshes.clear();
        this._rocheMeshes.clear();
        this._collisionMeshes.clear();
        this._lastData = null;
    }
}

import { WasmBridge } from './wasm-bridge.js';

// Scale 0 stays on the native WebSocket/CUDA source. Scale 1 and Scale 2 are
// independent in-page WASM engines, exposed through this facade so transport,
// telemetry, and binary-frame logic do not also own their API forwarding.
export class WebSocketScaleFallbackFacade {
    constructor() {
        this._fallback = null;
        this._fallbackInit = null;
    }

    _ensureFallback() {
        if (!this._fallback) {
            this._fallback = new WasmBridge();
            // The standalone PE/AE engines need the module but not a duplicate
            // full-size Scale-0 lattice. Use the smallest supported lattice.
            this._fallbackInit = this._fallback.init(9)
                .catch(err => {
                    console.warn('[WSBridge] Scale-1/2 WASM fallback failed to load;',
                        'engines unavailable in this session:', err);
                });
        }
        return this._fallback;
    }

    initPE() { this._ensureFallback().initPE(); }
    resetPE() { this._ensureFallback().resetPE(); }
    peAddParticle(catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff) {
        return this._ensureFallback().peAddParticle(
            catalogId, charge, x, y, z, vx, vy, vz, mass, r_eff,
        );
    }
    peAddLockedParticle(catalogId, charge, x, y, z, mass, r_eff) {
        return this._ensureFallback().peAddLockedParticle(
            catalogId, charge, x, y, z, mass, r_eff,
        );
    }
    peApplyEquilibriumOrbit(particleId, options = {}) {
        return this._ensureFallback().peApplyEquilibriumOrbit(particleId, options);
    }
    peApplyEquilibriumOrbitBatch(entries) {
        return this._ensureFallback().peApplyEquilibriumOrbitBatch?.(entries);
    }
    peScaleVelocity(particleId, scale) {
        return this._ensureFallback().peScaleVelocity(particleId, scale);
    }
    peSetSpinAxis(id, ax, ay, az) {
        return this._ensureFallback().peSetSpinAxis(id, ax, ay, az);
    }
    peGetForceDecomposition() { return this._ensureFallback().peGetForceDecomposition(); }
    peTick() { this._ensureFallback().peTick(); }
    peGetTick() { return this._ensureFallback().peGetTick(); }
    peGetObservationRevision() { return this._ensureFallback().peGetObservationRevision(); }
    peGetParticleData() { return this._ensureFallback().peGetParticleData(); }
    peGetDiagnostics() { return this._ensureFallback().peGetDiagnostics(); }
    peGetExtendedData() { return this._ensureFallback().peGetExtendedData(); }
    peGetForces() { return this._ensureFallback().peGetForces(); }
    peGetFieldSources() { return this._ensureFallback().peGetFieldSources(); }
    peSetDt(dt) { this._ensureFallback().peSetDt(dt); }
    peGetDt() { return this._ensureFallback().peGetDt(); }
    peSetSoftening(s) { this._ensureFallback().peSetSoftening(s); }
    peSetCoulomb(e) { this._ensureFallback().peSetCoulomb(e); }
    peSetDamping(e) { this._ensureFallback().peSetDamping(e); }
    peSetGravity(e) { this._ensureFallback().peSetGravity(e); }
    peSetLorentz(e) { this._ensureFallback().peSetLorentz(e); }
    peSetExchange(e) { this._ensureFallback().peSetExchange(e); }
    peSetStrong(e) { this._ensureFallback().peSetStrong(e); }
    peSetMagneticDipole(e) { this._ensureFallback().peSetMagneticDipole(e); }
    peSetSpinOrbit(e) { this._ensureFallback().peSetSpinOrbit(e); }
    peSetRadiation(e) { this._ensureFallback().peSetRadiation(e); }
    peSetRelativistic(e) { this._ensureFallback().peSetRelativistic(e); }
    peSetRelativisticVerlet(e) { this._ensureFallback().peSetRelativisticVerlet(e); }
    peGetToggle(name) { return this._ensureFallback().peGetToggle(name); }
    peGetBackendCapabilities() { return this._ensureFallback().peGetBackendCapabilities(); }
    peParticleCount() { return this._ensureFallback().peParticleCount(); }
    peClear() { this._ensureFallback().peClear(); }
    peExportCheckpoint() { return this._ensureFallback().peExportCheckpoint?.(); }
    peRestoreCheckpoint(checkpoint) { return this._ensureFallback().peRestoreCheckpoint?.(checkpoint); }
    peConfigureFinitePortBattery(size, capacity, chargeAmplitude, batteryAmplitude) { return this._ensureFallback().peConfigureFinitePortBattery?.(size, capacity, chargeAmplitude, batteryAmplitude); }
    peStepFinitePortBattery() { return this._ensureFallback().peStepFinitePortBattery?.(); }
    peReverseFinitePortBattery() { return this._ensureFallback().peReverseFinitePortBattery?.(); }
    peGetFinitePortBatterySnapshot() { return this._ensureFallback().peGetFinitePortBatterySnapshot?.(); }
    peGetParticleTypes() { return this._ensureFallback().peGetParticleTypes(); }
    peInspectParticle(id) { return this._ensureFallback().peInspectParticle(id); }

    initAE() { this._ensureFallback().initAE(); }
    resetAE() { this._ensureFallback().resetAE(); }
    aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N) {
        return this._ensureFallback().aeAddAtom(Z, x, y, z, vx, vy, vz, charge, N);
    }
    aeAddLockedAtom(Z, x, y, z, charge, N) {
        return this._ensureFallback().aeAddLockedAtom(Z, x, y, z, charge, N);
    }
    aeCreateBond(idA, idB, order, equilibriumDistance) {
        return this._ensureFallback().aeCreateBond(idA, idB, order, equilibriumDistance);
    }
    aeSetMoleculeReference(label) { return this._ensureFallback().aeSetMoleculeReference(label); }
    aeGetMoleculeDiagnostics() { return this._ensureFallback().aeGetMoleculeDiagnostics(); }
    aeTick() { return this._ensureFallback().aeTick(); }
    aeGetAtomData() { return this._ensureFallback().aeGetAtomData(); }
    aeGetDiagnostics() { return this._ensureFallback().aeGetDiagnostics(); }
    aeGetFieldSources() { return this._ensureFallback().aeGetFieldSources(); }
    aeGetForceDecomposition(want) { return this._ensureFallback().aeGetForceDecomposition(want); }
    aeGetRuntimeState() { return this._ensureFallback().aeGetRuntimeState(); }
    aeConfigureNuclearReaction(channelId) { return this._ensureFallback().aeConfigureNuclearReaction(channelId); }
    aeSetNuclearEnvironment(patch) { return this._ensureFallback().aeSetNuclearEnvironment(patch); }
    aeInjectNuclearParticle(kind) { return this._ensureFallback().aeInjectNuclearParticle(kind); }
    aeGetNuclearDiagnostics() { return this._ensureFallback().aeGetNuclearDiagnostics(); }
    aeGetNuclearVisuals() { return this._ensureFallback().aeGetNuclearVisuals(); }
    aeGetVelocities() { return this._ensureFallback().aeGetVelocities(); }
    aeGetDipoles() { return this._ensureFallback().aeGetDipoles(); }
    aeGetHBondPairs() { return this._ensureFallback().aeGetHBondPairs(); }
    aeSetDt(dt) { this._ensureFallback().aeSetDt(dt); }
    aeGetDt() { return this._ensureFallback().aeGetDt(); }
    aeSetSoftening(s) { this._ensureFallback().aeSetSoftening(s); }
    aeSetDamping(e) { this._ensureFallback().aeSetDamping(e); }
    aeSetBonding(e) { this._ensureFallback().aeSetBonding(e); }
    aeSetIonic(e) { this._ensureFallback().aeSetIonic(e); }
    aeSetVdw(e) { this._ensureFallback().aeSetVdw(e); }
    aeSetBondsForce(e) { this._ensureFallback().aeSetBondsForce(e); }
    aeSetSpeedLimit(e) { this._ensureFallback().aeSetSpeedLimit(e); }
    aeSetHBonds(e) { this._ensureFallback().aeSetHBonds(e); }
    aeSetAngleStrain(e) { this._ensureFallback().aeSetAngleStrain(e); }
    aeSetDipoleDipole(e) { this._ensureFallback().aeSetDipoleDipole(e); }
    aeSetThermostat(e) { this._ensureFallback().aeSetThermostat(e); }
    aeSetThermostatTemp(t) { this._ensureFallback().aeSetThermostatTemp(t); }
    aeSetElectronegativity(e) { this._ensureFallback().aeSetElectronegativity(e); }
    aePreBond() { this._ensureFallback().aePreBond(); }
    aeAtomCount() { return this._ensureFallback().aeAtomCount(); }
    aeInspectAtom(id) { return this._ensureFallback().aeInspectAtom(id); }
    aeClear() { this._ensureFallback().aeClear(); }
}

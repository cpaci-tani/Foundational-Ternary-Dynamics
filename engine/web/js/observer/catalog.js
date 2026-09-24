// @ts-check
import { FRACTAL_PRESETS } from './fractal-presets.js';
/** Declarative content for the Observer workspace. Values are presentation defaults. */
/** @typedef {{id:string,label:string,description:string}} CatalogEntry */
/** @typedef {{fov:number,sensitivity:number,invertY:boolean,speed:number,acceleration:number,grounded:boolean,worldUp:boolean,roll:number,axisLocks:readonly boolean[],reticle:boolean,doppler:boolean,beaming:boolean,optical:boolean,artisticShading:boolean,renderScale:number,fractalDetail?:number,waveWavelength?:number,waveSeparation?:number,waveAmplitude?:number,wavePhase?:number,polarizationMode?:'linear'|'circular'|'elliptical',autoQuality:boolean,pauseOnInspect:boolean,liveLink:boolean,overlayFilter:string,overlayFields:Readonly<Record<string,boolean>>,layers:Readonly<Record<string,boolean>>,bindings:Readonly<Record<string,string>>,gridSnap:number,scrollZoomSpeed?:number,storageCapMiB?:number,playbackSpeed?:number,mirrorWorld?:boolean,feedbackEnabled?:boolean,feedbackLayers?:number,feedbackDepth?:number,feedbackStrength?:number,feedbackScale?:number,forceGunEnabled?:boolean,forceGunSensitivity?:'delicate'|'normal'|'strong',forceGunMultiplier?:number}} ObserverSettings */
/** @param {string} id @param {string} label @param {string} [description] @returns {Readonly<CatalogEntry>} */
const named = (id, label, description = '') => Object.freeze({ id, label, description });

export const SHAPES = Object.freeze([
    named('sphere', 'Sphere'), named('box', 'Cube'), named('plane', 'Plane'),
    named('disk', 'Disk'), named('capsule', 'Capsule'), named('cylinder', 'Cylinder'),
    named('cone', 'Cone'), named('torus', 'Torus'), named('tetrahedron', 'Tetrahedron'),
    named('octahedron', 'Octahedron'), named('icosahedron', 'Icosahedron'),
    named('dodecahedron', 'Dodecahedron'), named('ellipsoid', 'Ellipsoid'),
    named('pyramid', 'Pyramid'), named('prism', 'Prism'), named('wedge', 'Wedge'),
    named('clock', 'Clock'), named('ruler', 'Ruler'), named('beacon', 'Beacon'),
    named('pulse', 'Light pulse'),
]);

export const ENVIRONMENT_PRESETS = Object.freeze([
    named('void', 'Infinite potential', 'An open plane and a quiet horizon.'),
    named('stars', 'Star field', 'A distant shell of fixed celestial landmarks.'),
    named('nebula', 'Nebula', 'Luminous color suspended around the observer.'),
    named('foam', 'Geometric foam', 'A surrounding network of cells.'),
    named('beyond', 'Beyond', 'Layered horizons and distant geometric forms.'),
    named('storm', 'Storm', 'Animated high-contrast atmospheric geometry.'),
    named('studio', 'Light studio', 'Neutral surfaces and measured illumination.'),
    named('workshop', 'Workshop', 'A geometric construction space.'),
    named('sunset', 'Sunset', 'A warm horizon with a distant light source.'),
    named('night', 'Night', 'A low-light field with clear point references.'),
    named('forest', 'Geometric forest', 'Repeated vertical forms for motion cues.'),
    named('urban', 'Geometric city', 'A repeating architectural environment.'),
    named('cubic-grid', 'Cubic grid', 'A three-dimensional reference lattice.'),
    named('wireframe-chamber', 'Wireframe chamber', 'Nested edges make perspective legible.'),
    named('concentric-spheres', 'Concentric spheres', 'Spherical shells at measured radii.'),
    named('nested-cubes', 'Nested cubes', 'Repeated cubical frames around the origin.'),
    named('polyhedral-cathedral', 'Polyhedral cathedral', 'Large intersecting polyhedral vaults.'),
    named('octahedral-halls', 'Octahedral halls', 'A procession of octahedral chambers.'),
    named('cuboctahedral-lattice', 'Cuboctahedral lattice', 'Edge and face geometry in a repeating field.'),
    named('stella-octangula', 'Stella octangula', 'Interlocking tetrahedra at environmental scale.'),
    named('radial-spokes', 'Radial spokes', 'Directional landmarks radiating from the origin.'),
    named('hexagonal-tunnel', 'Hexagonal tunnel', 'A long corridor of hexagonal rings.'),
    named('crystalline-corridor', 'Crystalline corridor', 'Faceted geometric walls and landmarks.'),
    named('layered-planes', 'Layered planes', 'Parallel sheets reveal depth and motion.'),
    ...FRACTAL_PRESETS,
]);

export const EXPERIMENTS = Object.freeze([
    named('baseline', 'A quiet frame', 'Start with a plane, a clock and simple landmarks.'),
    named('clocks', 'Separated clocks', 'Compare clocks at separated locations and their delayed appearance.'),
    named('moving-shapes', 'Passing geometry', 'Observe moving geometric objects through their arriving light.'),
    named('clock-avenue', 'Clock avenue', 'Move along a line of synchronized coordinate clocks.'),
    named('light-clock', 'Light clock', 'Compare a prescribed light-clock path across inertial frames.'),
    named('twin-journey', 'An out-and-back journey', 'Accumulate proper time along two prescribed worldlines.'),
    named('point-collision', 'Point collision', 'A controlled relativistic point-particle collision.'),
    named('delayed-edit', 'An edit seen later', 'Author a distant change and observe its light-travel delay.'),
]);

export const LAYERS = Object.freeze([
    named('grid', 'Reference grid'), named('polar', 'Polar grid'), named('axes', 'World axes'),
    named('sites', 'Reference-plane sites'), named('bonds', 'Reference-plane bonds'), named('wireframe', 'Box edge accents'),
    named('bounds', 'Bounding boxes'), named('vectors', 'Coordinate velocity arrows'),
    named('trajectories', 'Recorded spatial trails'), named('clocks', 'Clock faces'),
    named('pulses', 'Spherical pulse fronts'), named('lightCones', 'Spacetime light cones'),
    named('simultaneity', 'Relativity of simultaneity'), named('rings', 'Observer distance rings'),
    named('ghosts', 'Simultaneous ghosts'),
    named('lightPaths', 'Received-light events'), named('aberration', 'Aberration sky compass'),
    named('interference', 'Two-source interference'), named('standingWaves', 'Standing waves'),
    named('polarization', 'Electromagnetic polarization'),
]);

export const DEFAULT_BINDINGS = Object.freeze({
    forward: 'KeyW', backward: 'KeyS', left: 'KeyA', right: 'KeyD',
    up: 'Space', down: 'ControlLeft', boost: 'ShiftLeft', inspect: 'KeyE',
    world: 'Tab', reticle: 'KeyH', overlay: 'KeyO',
});

/** @type {Readonly<ObserverSettings>} */
export const DEFAULT_SETTINGS = Object.freeze({
    fov: 60, sensitivity: 0.002, invertY: false, speed: 0.15, acceleration: 0.4, scrollZoomSpeed: 1,
    grounded: false, worldUp: true, roll: 0, axisLocks: Object.freeze([false, false, false]),
    mirrorWorld: true, feedbackEnabled: true, feedbackLayers: 2, feedbackDepth: 3, feedbackStrength: 0.45, feedbackScale: 0.6,
    forceGunEnabled: true, forceGunSensitivity: 'normal', forceGunMultiplier: 1,
    reticle: true, doppler: true, beaming: false, optical: true, artisticShading: true,
    renderScale: 1, fractalDetail: 0.65, autoQuality: true, pauseOnInspect: true, liveLink: false, overlayFilter: 'selected', gridSnap: 0,
    waveWavelength: 4, waveSeparation: 4, waveAmplitude: 0.65, wavePhase: 0, polarizationMode: 'circular',
    overlayFields: Object.freeze({ name: true, position: true, velocity: true, properTime: true, distance: true, emissionTime: true }),
    layers: Object.freeze(Object.fromEntries(LAYERS.map(({ id }) => [id, ['ground', 'grid', 'shells', 'clocks', 'horizon'].includes(id)]))),
    bindings: DEFAULT_BINDINGS,
});

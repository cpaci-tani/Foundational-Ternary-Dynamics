// @ts-check
/** Explanations for the Observer's controls and reference measurements. */
/** @typedef {{title:string,text:string,math?:{latex:string,mathml:string,label:string}}} TooltipContent */
/** @typedef {{profile:string,curved:boolean,blackHole:boolean}} TooltipContext */

const equations = {
    gravitationalFrequency: {latex:'D_{\\rm grav}=\\frac{A_{\\rm emit}}{A_{\\rm receive}}',mathml:'<mrow><msub><mi>D</mi><mtext>grav</mtext></msub><mo>=</mo><mfrac><msub><mi>A</mi><mtext>emit</mtext></msub><msub><mi>A</mi><mtext>receive</mtext></msub></mfrac></mrow>',label:'The static gravitational frequency factor equals emission lapse divided by reception lapse.'},
    receiverFrequency: {latex:'D_{\\rm local}=\\gamma(1+\\boldsymbol\\beta\\cdot\\mathbf n_{\\rm back})',mathml:'<mrow><msub><mi>D</mi><mtext>local</mtext></msub><mo>=</mo><mi>γ</mi><mo>(</mo><mn>1</mn><mo>+</mo><mi mathvariant="bold">β</mi><mo>⋅</mo><msub><mi mathvariant="bold">n</mi><mtext>back</mtext></msub><mo>)</mo></mrow>',label:'The local receiver frequency factor is gamma times one plus the scalar product of beta with the backward sight-ray direction, both measured in the local static frame.'},
    beta: {latex:'\\beta=\\frac{|\\mathbf v|}{c}',mathml:'<mrow><mi>β</mi><mo>=</mo><mfrac><mrow><mo>|</mo><mi mathvariant="bold">v</mi><mo>|</mo></mrow><mi>c</mi></mfrac></mrow>',label:'Beta equals speed divided by the speed of light.'},
    betaComponent: {latex:'\\beta_i=\\frac{v_i}{c}',mathml:'<mrow><msub><mi>β</mi><mi>i</mi></msub><mo>=</mo><mfrac><msub><mi>v</mi><mi>i</mi></msub><mi>c</mi></mfrac></mrow>',label:'A beta component equals the corresponding velocity component divided by the speed of light.'},
    gamma: {latex:'\\gamma=\\frac{1}{\\sqrt{1-\\beta^2}}',mathml:'<mrow><mi>γ</mi><mo>=</mo><mfrac><mn>1</mn><msqrt><mrow><mn>1</mn><mo>−</mo><msup><mi>β</mi><mn>2</mn></msup></mrow></msqrt></mfrac></mrow>',label:'Gamma equals one divided by the square root of one minus beta squared.'},
    srClock: {latex:'d\\tau=\\frac{dt}{\\gamma}',mathml:'<mrow><mi>dτ</mi><mo>=</mo><mfrac><mi>dt</mi><mi>γ</mi></mfrac></mrow>',label:'Proper time increment equals coordinate time increment divided by gamma.'},
    grClock: {latex:'d\\tau=\\frac{A}{\\gamma}\\,dt',mathml:'<mrow><mi>dτ</mi><mo>=</mo><mfrac><mi>A</mi><mi>γ</mi></mfrac><mi>dt</mi></mrow>',label:'Proper time increment equals lapse divided by gamma times coordinate time increment.'},
    lapse: {latex:'A=\\sqrt{1-\\frac{r_s}{r}}',mathml:'<mrow><mi>A</mi><mo>=</mo><msqrt><mrow><mn>1</mn><mo>−</mo><mfrac><msub><mi>r</mi><mi>s</mi></msub><mi>r</mi></mfrac></mrow></msqrt></mrow>',label:'The Schwarzschild lapse is the square root of one minus Schwarzschild radius divided by areal radius.'},
    radius: {latex:'r_s=\\frac{2GM}{c^2}',mathml:'<mrow><msub><mi>r</mi><mi>s</mi></msub><mo>=</mo><mfrac><mrow><mn>2</mn><mi>G</mi><mi>M</mi></mrow><msup><mi>c</mi><mn>2</mn></msup></mfrac></mrow>',label:'Schwarzschild radius equals two times the gravitational mass parameter divided by the speed of light squared.'},
    frequency: {latex:'D=\\frac{f_{\\rm received}}{f_{\\rm emitted}},\\qquad z=\\frac{1}{D}-1',mathml:'<mrow><mi>D</mi><mo>=</mo><mfrac><msub><mi>f</mi><mtext>received</mtext></msub><msub><mi>f</mi><mtext>emitted</mtext></msub></mfrac><mo>,</mo><mspace width="1em"/><mi>z</mi><mo>=</mo><mfrac><mn>1</mn><mi>D</mi></mfrac><mo>−</mo><mn>1</mn></mrow>',label:'D is received frequency divided by emitted frequency. Redshift z equals one over D minus one.'},
    intensity: {latex:'\\frac{I_{\\rm received}}{I_{\\rm reference}}=D^4',mathml:'<mrow><mfrac><msub><mi>I</mi><mtext>received</mtext></msub><msub><mi>I</mi><mtext>reference</mtext></msub></mfrac><mo>=</mo><msup><mi>D</mi><mn>4</mn></msup></mrow>',label:'The reference bolometric ray-intensity factor is D to the fourth power.'},
    delay: {latex:'\\Delta t=t_{\\rm reception}-t_{\\rm emission}',mathml:'<mrow><mi>Δt</mi><mo>=</mo><msub><mi>t</mi><mtext>reception</mtext></msub><mo>−</mo><msub><mi>t</mi><mtext>emission</mtext></msub></mrow>',label:'Light travel time is reception coordinate time minus emission coordinate time.'},
    null: {latex:'c(t-t_e)=|\\mathbf x_{\\rm eye}(t)-\\mathbf x_{\\rm source}(t_e)|',mathml:'<mrow><mi>c</mi><mo>(</mo><mi>t</mi><mo>−</mo><msub><mi>t</mi><mi>e</mi></msub><mo>)</mo><mo>=</mo><mo>|</mo><msub><mi mathvariant="bold">x</mi><mtext>eye</mtext></msub><mo>(</mo><mi>t</mi><mo>)</mo><mo>−</mo><msub><mi mathvariant="bold">x</mi><mtext>source</mtext></msub><mo>(</mo><msub><mi>t</mi><mi>e</mi></msub><mo>)</mo><mo>|</mo></mrow>',label:'A Minkowski light ray connects the eye at reception to the source at emission, with distance equal to c times the travel time.'},
    force: {latex:'\\mathbf F=\\frac{d\\mathbf p}{dt},\\qquad \\mathbf p=m\\gamma\\mathbf v',mathml:'<mrow><mi mathvariant="bold">F</mi><mo>=</mo><mfrac><mrow><mi>d</mi><mi mathvariant="bold">p</mi></mrow><mi>dt</mi></mfrac><mo>,</mo><mspace width="1em"/><mi mathvariant="bold">p</mi><mo>=</mo><mi>m</mi><mi>γ</mi><mi mathvariant="bold">v</mi></mrow>',label:'Coordinate force is the coordinate-time derivative of momentum; relativistic momentum is mass times gamma times velocity.'},
    energy: {latex:'\\frac{E}{mc^2}=\\gamma,\\qquad \\frac{K}{mc^2}=\\gamma-1',mathml:'<mrow><mfrac><mi>E</mi><mrow><mi>m</mi><msup><mi>c</mi><mn>2</mn></msup></mrow></mfrac><mo>=</mo><mi>γ</mi><mo>,</mo><mspace width="1em"/><mfrac><mi>K</mi><mrow><mi>m</mi><msup><mi>c</mi><mn>2</mn></msup></mrow></mfrac><mo>=</mo><mi>γ</mi><mo>−</mo><mn>1</mn></mrow>',label:'Total energy per rest energy is gamma; kinetic energy per rest energy is gamma minus one.'},
    momentum: {latex:'\\frac{|\\mathbf p|}{mc}=\\gamma\\beta',mathml:'<mrow><mfrac><mrow><mo>|</mo><mi mathvariant="bold">p</mi><mo>|</mo></mrow><mrow><mi>m</mi><mi>c</mi></mrow></mfrac><mo>=</mo><mi>γ</mi><mi>β</mi></mrow>',label:'Momentum magnitude per mass times c is gamma times beta.'},
    killing: {latex:'\\frac{E_\\infty}{mc^2}=A\\gamma',mathml:'<mrow><mfrac><msub><mi>E</mi><mo>∞</mo></msub><mrow><mi>m</mi><msup><mi>c</mi><mn>2</mn></msup></mrow></mfrac><mo>=</mo><mi>A</mi><mi>γ</mi></mrow>',label:'Conserved energy at infinity per rest energy is lapse times gamma for an unforced geodesic.'},
    properDistance: {latex:'\\ell=\\int_R^r\\frac{\\mathrm{d}r^{\\prime}}{\\sqrt{1-r_s/r^{\\prime}}}',mathml:'<mrow><mi>ℓ</mi><mo>=</mo><msubsup><mo>∫</mo><mi>R</mi><mi>r</mi></msubsup><mfrac><mrow><mi>d</mi><msup><mi>r</mi><mo>′</mo></msup></mrow><msqrt><mrow><mn>1</mn><mo>−</mo><mfrac><msub><mi>r</mi><mi>s</mi></msub><msup><mi>r</mi><mo>′</mo></msup></mfrac></mrow></msqrt></mfrac></mrow>',label:'Proper radial distance on a constant Schwarzschild-time slice is the integral of dr divided by the square root of one minus Schwarzschild radius over r.'},
    opticalIndex: {latex:'n=\\frac{B}{A},\\qquad \\left|\\frac{d\\mathbf x}{dt}\\right|_{\\rm light}=\\frac{c}{n}',mathml:'<mrow><mi>n</mi><mo>=</mo><mfrac><mi>B</mi><mi>A</mi></mfrac><mo>,</mo><mspace width="1em"/><msub><mrow><mo>|</mo><mfrac><mrow><mi>d</mi><mi mathvariant="bold">x</mi></mrow><mi>dt</mi></mfrac><mo>|</mo></mrow><mtext>light</mtext></msub><mo>=</mo><mfrac><mi>c</mi><mi>n</mi></mfrac></mrow>',label:'The coordinate optical index is B over A; coordinate light speed is c over this index.'},
    hover: {latex:'a_{\\rm hover}=\\frac{GM}{r^2 A}',mathml:'<mrow><msub><mi>a</mi><mtext>hover</mtext></msub><mo>=</mo><mfrac><mrow><mi>G</mi><mi>M</mi></mrow><mrow><msup><mi>r</mi><mn>2</mn></msup><mi>A</mi></mrow></mfrac></mrow>',label:'Static hover proper acceleration equals GM divided by the product of areal radius squared and lapse.'},
    tidal: {latex:'T_r=\\frac{2GM}{r^3},\\qquad T_\\perp=-\\frac{GM}{r^3}',mathml:'<mrow><msub><mi>T</mi><mi>r</mi></msub><mo>=</mo><mfrac><mrow><mn>2</mn><mi>G</mi><mi>M</mi></mrow><msup><mi>r</mi><mn>3</mn></msup></mfrac><mo>,</mo><mspace width="1em"/><msub><mi>T</mi><mo>⊥</mo></msub><mo>=</mo><mo>−</mo><mfrac><mrow><mi>G</mi><mi>M</mi></mrow><msup><mi>r</mi><mn>3</mn></msup></mfrac></mrow>',label:'Radial tidal coefficient is two GM over r cubed; transverse coefficient is minus GM over r cubed.'},
    curvature: {latex:'R_{abcd}R^{abcd}=\\frac{12r_s^2}{r^6}',mathml:'<mrow><msub><mi>R</mi><mi>abcd</mi></msub><msup><mi>R</mi><mi>abcd</mi></msup><mo>=</mo><mfrac><mrow><mn>12</mn><msup><msub><mi>r</mi><mi>s</mi></msub><mn>2</mn></msup></mrow><msup><mi>r</mi><mn>6</mn></msup></mfrac></mrow>',label:'The Schwarzschild Kretschmann curvature invariant equals twelve Schwarzschild radius squared divided by areal radius to the sixth power.'},
    wave: {latex:'k=\\frac{2\\pi}{\\lambda}',mathml:'<mrow><mi>k</mi><mo>=</mo><mfrac><mrow><mn>2</mn><mi>π</mi></mrow><mi>λ</mi></mfrac></mrow>',label:'Wave number equals two pi divided by wavelength.'},
    horizonDistance: {latex:'\\ell=\\int_{r_s}^{r}\\frac{\\mathrm{d}r^{\\prime}}{\\sqrt{1-r_s/r^{\\prime}}}',mathml:'<mrow><mi>ℓ</mi><mo>=</mo><msubsup><mo>∫</mo><msub><mi>r</mi><mi>s</mi></msub><mi>r</mi></msubsup><mfrac><mrow><mi>d</mi><msup><mi>r</mi><mo>′</mo></msup></mrow><msqrt><mrow><mn>1</mn><mo>−</mo><mfrac><msub><mi>r</mi><mi>s</mi></msub><msup><mi>r</mi><mo>′</mo></msup></mfrac></mrow></msqrt></mfrac></mrow>',label:'Proper radial distance on a static slice is integrated from the horizon areal radius to the observer areal radius. No static observer is placed at the horizon.'},
    criticalImpact: {latex:'b_c=\\frac{3\\sqrt{3}}{2}\\,r_s',mathml:'<mrow><msub><mi>b</mi><mi>c</mi></msub><mo>=</mo><mfrac><mrow><mn>3</mn><msqrt><mn>3</mn></msqrt></mrow><mn>2</mn></mfrac><msub><mi>r</mi><mi>s</mi></msub></mrow>',label:'The critical null-ray impact parameter equals three square roots of three over two times Schwarzschild radius.'},
    shadow: {latex:'\\sin\\alpha=\\frac{b_c A}{r}',mathml:'<mrow><mi mathvariant="normal">sin</mi><mi>α</mi><mo>=</mo><mfrac><mrow><msub><mi>b</mi><mi>c</mi></msub><mi>A</mi></mrow><mi>r</mi></mfrac></mrow>',label:'The static black-hole shadow half-angle satisfies sine alpha equals critical impact parameter times lapse divided by areal radius. The angle branch changes inside the photon sphere.'},
    horizonArea: {latex:'\\mathcal A_H=4\\pi r_s^2',mathml:'<mrow><msub><mi>𝒜</mi><mi>H</mi></msub><mo>=</mo><mn>4</mn><mi>π</mi><msup><msub><mi>r</mi><mi>s</mi></msub><mn>2</mn></msup></mrow>',label:'Schwarzschild horizon area is four pi times Schwarzschild radius squared.'},
    solidAngle: {latex:'\\Omega=2\\pi(1-\\cos\\alpha)',mathml:'<mrow><mi>Ω</mi><mo>=</mo><mn>2</mn><mi>π</mi><mo>(</mo><mn>1</mn><mo>−</mo><mi mathvariant="normal">cos</mi><mi>α</mi><mo>)</mo></mrow>',label:'A circular shadow cone has solid angle two pi times one minus cosine of its half-angle.'},
    horizonCurvature: {latex:'\\left.R_{abcd}R^{abcd}\\right|_{r=r_s}=\\frac{12}{r_s^4}',mathml:'<mrow><msub><mrow><msub><mi>R</mi><mi>abcd</mi></msub><msup><mi>R</mi><mi>abcd</mi></msup></mrow><mrow><mi>r</mi><mo>=</mo><msub><mi>r</mi><mi>s</mi></msub></mrow></msub><mo>=</mo><mfrac><mn>12</mn><msup><msub><mi>r</mi><mi>s</mi></msub><mn>4</mn></msup></mfrac></mrow>',label:'The horizon value of the Kretschmann scalar is twelve divided by Schwarzschild radius to the fourth power.'},
};

/** @type {Record<string,string>} */
const settings = {
    fov:'The vertical perspective angle in degrees. A wider view shows more scene; it does not change velocity, clock rates or light physics.',
    sensitivity:'Radians of camera rotation per pointer-motion unit. Reduce this for precise aiming.',
    invertY:'Reverse the vertical response of mouse look. This changes controls, not the measured frame.',
    speed:'Navigation target speed as a fraction of light speed in SR, measured in the local static frame in GR. The session enforces its displayed speed bound.',
    acceleration:'Maximum navigation control strength. SR controls the coordinate force per rest mass; GR guided motion controls local spatial four-velocity per coordinate time. This is a prescribed control, not gravity.',
    grounded:'Restrict navigation to the reference plane instead of following the full three-dimensional view direction.',
    worldUp:'Keep the camera upright against the coordinate Y direction. Disable to permit the selected roll.',
    roll:'Rotate the camera around the line of sight, in radians. This rotates the view without changing the worldline.',
    reticle:'Show the center aiming mark used for inspection and the force gun. The target is determined by the current view ray.',
    doppler:'Shift the three reference spectral lines according to the received frequency ratio. Turning color off retains the same unshifted spectral baseline.',
    beaming:'Apply the fourth power of the received frequency factor to reference ray intensity. This is separate from the color display.',
    optical:'Trace the arriving image at each source emission event. The alternative compares simultaneous geometry; it supplies no received-light timing.',
    artisticShading:'Add presentation shading to surfaces. This is a visual treatment rather than a measured radiance or relativistic correction.',
    showAssistant:'Reveal the JEV console button. It is hidden initially; changing visibility does not authorize any assistant action.',
    renderScale:'Scale the internal image dimensions relative to the viewport. Lower values trade fine image detail for rendering speed.',
    autoQuality:'Adapt internal resolution to completed GPU timing measurements. Physics and recorded clocks use the same session state.',
    pauseOnInspect:'Pause while editing ordinary control drawers. The live Telemetry dropdown continues playback and navigation.',
    liveLink:'Allow passive updates from the retained lattice observation source. The observer sandbox never writes these observations back.',
    overlayFilter:'Choose which objects receive spatial labels: the selected source, all sources, or none.',
    gridSnap:'Round author placements to this coordinate spacing. Zero leaves positions unsnapped; this does not change physical integration.',
    scrollZoomSpeed:'Coordinate displacement per scroll notch. Paused scrolling relocates the observer and starts a new clock origin; it is not physical travel or a change of field of view.',
    fractalDetail:'Control sampling detail of the volumetric background. Higher values add appearance detail and GPU cost, without changing physics.',
    feedbackEnabled:'Enable bounded camera-image echoes as a background illustration. These are repeated rendered views, not causal feedback in the world.',
    feedbackLayers:'Number of overlapping background camera images per echo level.',
    feedbackDepth:'Bounded number of recursive camera-image levels. More levels require more rendering passes.',
    feedbackStrength:'Opacity of the repeated camera image. Zero makes the echo contribution invisible.',
    feedbackScale:'Internal resolution of camera echoes relative to the main view.',
    forceGunEnabled:'Enable the Playground spring tether. Left mouse pulls, right mouse pushes; release ends the force.',
    forceGunSensitivity:'Choose the spring, damping and force-limit preset used by the classical tether.',
    forceGunMultiplier:'Multiply the tether strength and force limit. Requested force can still exceed the available cap.',
    waveWavelength:'Spatial period of the analytic wave illustration, in displayed coordinates.',
    waveSeparation:'Coordinate spacing between the two illustrated scalar-wave sources.',
    waveAmplitude:'Amplitude of the analytic wave display. These waves do not exchange energy with world objects.',
    wavePhase:'Relative phase between the two reference sources, in radians.',
    polarizationMode:'Choose the geometric electric/magnetic ribbon pattern: linear, circular or elliptical. It is an analytic illustration.',
};
/** @type {Record<string,string>} */
const actions = {
    assistant:'Open or close the JEV console for this workspace. Opening it releases captured input without starting another simulation owner.',
    exit:'Return to the retained Lattice Sim workspace. Your observer session is retained until explicitly replaced.',
    'toggle-ui':'Hide or restore the interface while the scene keeps evolving. Hiding dismisses control drawers and the console; the small UI button remains.',
    playback:'Pause or resume the session. Pausing freezes physical integration and optical source histories.',
    'close-panel':'Close this drawer and return input to the scene. An inspection-owned pause resumes; a manual pause remains paused.',
    undo:'Restore the latest author checkpoint in a new timeline branch. Existing optical records belong to their recorded branch.',
    reset:'Replace this world with its preparation defaults and restart the experiment clocks.',
    'reset-observer':'Relocate the observer to its initial viewpoint and restart its clock origin. This is a preparation action, not a traveled path.',
    'create-object':'Create the chosen geometry in front of your current viewpoint as one author intervention.',
    'apply-object':'Apply the previewed properties to the authoritative current object in one edit. Earlier images arrive according to retained light history.',
    'discard-preview':'Discard unapplied object changes and show the current authoritative source again.',
    duplicate:'Create a new source with the selected object properties. The new object gets its own identity and light history.',
    delete:'End this source’s current existence. Light emitted before deletion may remain visible until its delay expires.',
    restore:'Restore this deleted source as a new current revision. Its older received images remain tied to their emission revision.',
    impulse:'Apply an instantaneous classical momentum change to the selected dynamic Playground body.',
    'impulse-play':'Apply the classical momentum change, then resume the world so the resulting motion is visible.',
    'emit-pulse':'Record a new emission event on this source. The pulse-front overlay illustrates its coordinate light cone.',
    'use-playground':'Prepare the classical body-mechanics profile, where gravity, springs and rigid-body collisions are supported.',
    joint:'Connect the two selected dynamic Playground sources with the declared spring rest length, stiffness and damping.',
    'world-physics':'Apply the staged classical gravity and collision settings together.',
    present:'Return source-history viewing to the present. This does not rewind or reconstruct the observer’s retained proper clock.',
    save:'Save the current world and view settings under a local name. Autosave remains independently opt-in.',
    export:'Download the current world and settings as a portable JSON document.',
    load:'Replace the current preparation with this saved local world after schema and reference-model validation.',
    'remove-save':'Remove this local saved document. This does not delete the world currently running in the workspace.',
    'refresh-saves':'Read the local saved-world list again.',
    'clear-saves':'Remove local named saves and autosaves after the explicit second confirmation. The active world stays open.',
    'reset-bindings':'Restore the initial Observer keyboard bindings.',
};
/** @type {Record<string,string>} */
const panels = {
    telemetry:'Open a live landscape grid of clocks, motion, geometry, received light and rendering diagnostics. Navigation and playback remain available.',
    objects:'Inspect current source properties and prepare author edits. Received images can refer to an earlier source revision.',
    forcegun:'Configure the classical Playground spring tether and its force limits.',
    world:'Choose the model, classical interactions and visual environment, or inspect the fixed GR preparation.',
    camera:'Configure navigation, perspective, constraints and optical presentation.',
    layers:'Choose explanatory coordinate geometry and source labels without altering the source physics.',
    phenomena:'Compare coordinate geometry, arriving light and analytic wave illustrations.',
    experiments:'Replace the sandbox with a declared repeatable reference preparation.',
    storage:'Save, export, validate and restore local observer worlds. Autosave is off initially.',
    lattice:'Inspect cached completed publications from the separate retained lattice owner.',
    help:'Inspect keyboard bindings, navigation behavior and interface preferences.',
};
/** @type {Record<string,string>} */
const entity = {
    name:'A display name for this source. Identity and revisions remain separate from the editable name.',
    position:'Current source-center coordinates. Applying a position edit is a preparation intervention, not continuous physical travel.',
    size:'Full rest-frame extents along the source’s local axes; these are diameters or widths, not half sizes.',
    rotation:'Current source-axis orientation, displayed in degrees. The source geometry is evaluated in its declared local/rest frame.',
    velocity:'Source coordinate velocity. SR values are fractions of light speed and must have total magnitude below the session bound.',
    color:'Tint for the three reference spectral lines; this is not a temperature, chemical composition or measured spectrum.',
    emission:'Reference surface-emission multiplier. It scales source brightness without assigning a physical luminosity.',
    mass:'Declared source rest-mass parameter in simulation units. The observer has no assigned mass, and a GR marker mass does not set the spacetime.',
    spectral:'Choose the source’s declared line spectrum. Frequency shifts move those emitted wavelengths before display color is evaluated.',
    coordinateForcePerMass:'Coordinate force divided by source rest mass for supported SR point markers; it changes momentum per coordinate time.',
    bodyType:'Dynamic bodies respond to classical forces; prescribed bodies follow declared motion; fixed landmarks stay in place.',
    overlay:'Allow this source to display spatial information when the global overlay filter includes it.',
    angularVelocity:'Classical body rotation rate, in radians per simulation time unit.',
    restitution:'Classical collision rebound coefficient. Zero is fully inelastic in the normal direction; one preserves that relative normal speed.',
    friction:'Classical contact-friction coefficient used by the Playground collision model.',
    damping:'Classical velocity-damping rate for this body; it removes motion in the adopted Playground model.',
    gravity:'Whether this Playground body responds to the selected world gravity.',
    collisions:'Whether this Playground source participates in body and reference-plane collision handling.',
};
/** @type {Record<string,string>} */
const environment = {
    preset:'Choose a distant visual shell or analytic background. It provides landmarks without changing gravitational or body dynamics.',
    seed:'Deterministic integer selecting a repeatable variation of the chosen environment.',
    radius:'Coordinate radius of the visual shell. This is a background distance, not a gravitating body radius.',
    density:'Appearance density of the environment’s features; it is not matter density or mass.',
    spacing:'Coordinate separation of repeated visual landmarks and applicable grids.',
    orientation:'Rotate the visual environment about its coordinate axis, displayed in degrees.',
    opacity:'Transparency of the visual environment, from invisible to fully displayed.',
    animationRate:'Rate of environment animation per simulation time. Zero freezes its appearance.',
    color:'Tint the distant visual shell without assigning a physical spectrum.',
    anchor:'Keep landmarks at the coordinate origin or make their visual shell follow the camera.',
};
/** @type {Record<string,string>} */
const layers = {
    grid:'Reference grid in the declared coordinate plane. Its spacing is a visual coordinate ruler, not an inferred physical lattice spacing.',
    polar:'Concentric coordinate circles and radial lines on the reference plane.',
    axes:'Coordinate X, Y and Z direction guides.',
    sites:'Illustrative reference-plane points. These are presentation landmarks, not sampled microscopic records.',
    bonds:'Illustrative reference-plane connections. They do not add springs or interactions.',
    wireframe:'Accent edges of supported box geometry to make received shape changes easier to see.',
    bounds:'Compare declared source bounds in the current view. Historical optical bounds use their recorded source event.',
    vectors:'Coordinate source-velocity arrows. They are not proper acceleration or a force-field measurement.',
    trajectories:'Spatial trails of retained source-center records, capped by the presentation budget.',
    clocks:'Clock faces sourced from each geometry marker’s accumulated proper-clock convention.',
    pulses:'Flat-space spherical fronts of recorded emission events. They are coordinate light-cone markers, not rendered photon shells.',
    lightCones:'Past and future Minkowski null cones through the observer event in the spacetime reference illustration.',
    simultaneity:'Compare coordinate-frame and observer-frame simultaneous slices. This is distinct from the emission events whose light is actually arriving.',
    rings:'Coordinate observer-centered distance rings. These are flat-space illustrative rulers.',
    ghosts:'Observer-simultaneous comparison geometry, including the supported rest-frame length transformation. Ghosts are not delayed optical images.',
    lightPaths:'Mark the received source-emission event and the null path to the present observer event.',
    aberration:'Compare coordinate directions and received observer-frame directions in a full-sky compass, independent of perspective field of view.',
    interference:'Coordinate-frame analytic interference of two scalar-wave sources. It does not interact with source bodies.',
    standingWaves:'Coordinate-frame analytic standing-wave pattern with fixed nodes; animation follows simulation time.',
    polarization:'Analytic transverse electric/magnetic ribbon orientations. These do not establish a Lorentz-transformed field simulation.',
};

/** @type {Record<string,string>} */
const telemetry = {
    reference:'The declared reference model used by this preparation: flat Minkowski SR, a Schwarzschild vacuum exterior, or classical Playground mechanics.',
    engine:'The active integration owner named by the session. This is distinct from the renderer and cached lattice publication.',
    integrator:'Version of the numerical transaction/integration convention used by retained worldline records.',
    session:'Identity of this retained sandbox world. Saved worlds preserve the declared source identity.',
    epoch:'Timeline branch number. Author interventions that branch history use a new epoch.',
    tick:'Completed observer-session integration transaction count.',
    playing:'Current transport state. Live telemetry observes it without taking ownership of playback.',
    playback:'Simulation time advanced per wall-clock presentation time. It does not redefine proper time.',
    'camera-preview':'Whether the displayed camera is the integrated session observer or a comparison view.',
    units:'Declared source units. A text unit label alone does not calibrate normalized coordinates to SI.',
    'body-count':'Number of currently alive source records; historical images may refer to other revisions.',
    'warning-count':'Number of retained session warnings relevant to its integration and preparation limits.',
    'coordinate-time':'Time coordinate of the reference frame; in the Schwarzschild chart it is normalized to a clock at infinity.',
    'proper-time':'Clock accumulated on the observer’s integrated worldline. Source-history scrubbing does not rewind that observer clock.',
    worldline:'Identity of the current observer clock path. Relocation starts a new clock origin.',
    'worldline-reason':'Preparation action that established the current observer clock origin.',
    'worldline-start':'Coordinate time recorded when this observer proper clock started.',
    'coordinate-elapsed':'Coordinate time elapsed since the same observer-worldline origin used for the paired clock comparison.',
    'clock-slip':'Difference between coordinate elapsed time and the observer clock measured from their shared origin. Unavailable during unpaired comparisons.',
    'clock-rate':'Instantaneous conversion from reference coordinate time to proper time along the current observer motion.',
    'coordinate-time-si':'Coordinate time converted to seconds using this GR preparation’s explicit length scale divided by c.',
    'proper-time-si':'Observer’s integrated proper clock converted to seconds using the declared GR scale.',
    'clock-slip-si':'Paired worldline clock difference converted to seconds; a missing paired origin remains unavailable.',
    'history-start':'Earliest retained source-emission time that the optical renderer can inspect.',
    'history-end':'Latest source-coordinate time currently being viewed.',
    'history-span':'Width of the retained optical source interval, including deliberately prepared prehistory when present.',
    'history-policy':'Configured retention window for source-worldline records.',
    'history-segments':'Number of retained source-motion segments available to arriving-light observations.',
    backlog:'Integration time queued but not yet advanced by the session owner.',
    'time-scale':'One normalized coordinate-time unit equals one declared length unit divided by c.',
    speed:'Observer speed in the local measuring frame, displayed as a fraction of c in relativistic profiles.',
    'speed-si':'Observer speed measured locally, converted to metres per second.',
    'coordinate-speed':'Magnitude of coordinate-position drift per coordinate time. In curved spacetime it differs from local measured speed.',
    'coordinate-speed-si':'Coordinate drift converted using the explicit length/time scale; it is not a local velocity measurement.',
    gamma:'Local Lorentz factor describing the observer’s velocity relative to the declared measuring frame.',
    rapidity:'Hyperbolic velocity parameter; its magnitude is the inverse hyperbolic tangent of beta.',
    'sr-clock-rate':'Special-relativistic clock rate relative to the local static frame, before the gravitational lapse factor.',
    'energy-rest':'Local total energy divided by rest energy; no observer rest mass needs to be assigned for this ratio.',
    'kinetic-rest':'Local kinetic energy divided by rest energy.',
    'momentum-rest':'Local momentum magnitude divided by rest mass times c.',
    'killing-energy':'Energy at infinity per rest energy. It is conserved for unforced geodesic motion in this static spacetime.',
    'motion-mode':'Guided navigation applies prescribed controls; free fall follows an exterior timelike geodesic.',
    'speed-cap':'Numerical/local-speed boundary for this implemented session, rather than a new law of relativity.',
    'cap-applied':'Whether this session reports application of its navigation speed cap.',
    'selected-distance':'Coordinate center-to-center separation from the selected current source. This differs from a received-light path.',
    'isotropic-radius':'Distance from the central coordinate origin in the isotropic chart.',
    'isotropic-radius-si':'Central isotropic coordinate radius converted to metres.',
    'areal-radius':'Radius defined by the circumference of a symmetry sphere divided by two pi.',
    'areal-height':'Difference of areal radii. This is not the proper radial distance measured along a static slice.',
    'proper-radial-distance':'Radial proper length from the declared reference radius on a constant Schwarzschild-time slice.',
    'radius-rs':'Current areal radius expressed in Schwarzschild-radius units.',
    'length-scale':'SI metres corresponding to one displayed isotropic coordinate unit.',
    'outer-boundary':'Finite outer boundary of the numerical optical domain; it is not a cosmological or event horizon.',
    'observer-mass':'The observer is a test worldline. No rest mass or kilogram calibration is assigned to it.',
    'selected-mass':'Current source mass parameter. It does not set the GR central mass or imply a kilogram calibration.',
    'star-mass-solar':'Central nominal gravitational mass parameter in multiples of the IAU nominal solar GM.',
    'star-gm':'Gravitational mass parameter GM used by the central reference metric, in cubic metres per second squared.',
    'star-mass-kg':'Mass estimate obtained by dividing nominal GM by measured G. It inherits G uncertainty and is not an exact nominal solar mass.',
    'stellar-radius':'Areal radius of the opaque compact-star emitting surface. A black-hole horizon has no emitting surface.',
    'schwarzschild-radius':'Areal Schwarzschild radius set by the central gravitational mass parameter.',
    'mean-density-proxy':'Mass divided by the Euclidean sphere volume formed from the areal radius. It is not a GR interior density or an equation of state.',
    lapse:'Static proper-time rate relative to the Schwarzschild coordinate time normalized at infinity.',
    'spatial-factor':'Local scale between isotropic coordinate length and static proper length.',
    'optical-index':'Ratio of the isotropic spatial factor to lapse, controlling coordinate light speed while local light speed remains c.',
    'coordinate-light-speed':'Coordinate propagation speed in the isotropic chart. Locally measured light still travels at c.',
    'local-compactness':'Schwarzschild radius divided by the observer’s current areal radius.',
    'surface-compactness':'Schwarzschild radius divided by the emitting surface’s areal radius.',
    'surface-lapse':'Static clock rate on the star’s emitting surface relative to coordinate time at infinity.',
    'surface-redshift':'Frequency redshift of the static emitting surface received by a static observer at infinity.',
    'observer-redshift':'Redshift to infinity of a hypothetical static emitter at the observer’s current radius.',
    'hover-acceleration':'Proper acceleration needed to remain static at this radius. It is a reference value, not the current guided-control thrust.',
    'surface-hover-acceleration':'Proper acceleration needed to support a static worldline on the star surface.',
    'radial-tidal':'Radial stretching coefficient for small separations in a radial freely falling orthonormal frame.',
    'transverse-tidal':'Transverse compression coefficient for small separations in a radial freely falling orthonormal frame.',
    kretschmann:'Coordinate-independent scalar measuring Schwarzschild vacuum curvature; it is formed by contracting the Riemann tensor with itself.',
    'escape-speed':'Ideal local radial speed whose test-particle energy reaches the threshold for escape to infinity in the extended reference metric.',
    'photon-sphere':'Areal radius of the formal unstable vacuum photon orbit. Its applicability depends on whether the star surface covers it.',
    'isco-radius':'Areal radius of the innermost stable circular timelike test-particle orbit in the Schwarzschild vacuum reference.',
    'optical-mode':'Whether the current view uses a received null ray, curved received ray, or simultaneous geometry.',
    'optical-status':'Whether the current sight ray has a valid retained source-emission witness. A miss has no source clock or frequency reading.',
    source:'Source identity at the event whose light reaches this view ray; this may be a historical source revision.',
    'source-revision':'Revision recorded at the received emission event, distinct from the source’s current editable revision.',
    'frequency-ratio':'Received frequency divided by emitted frequency for the current valid ray.',
    redshift:'Received wavelength/frequency redshift convention. Positive values are redshift; negative values are blueshift.',
    'wavelength-ratio':'Received wavelength divided by emitted wavelength, the reciprocal of the frequency ratio.',
    'bolometric-factor':'Reference bolometric ray-intensity multiplier. It is not total luminosity or integrated flux at the observer.',
    'emission-time':'Coordinate time at the source event whose light is now received.',
    'source-clock':'Source proper clock at the received emission event. It need not share the observer’s clock origin.',
    'optical-delay':'Reception coordinate time minus emission coordinate time along the valid received ray.',
    'optical-delay-si':'Coordinate light-travel delay converted to seconds using the explicit GR calibration.',
    'optical-path':'Coordinate arclength of the received ray. In the curved isotropic chart it differs from proper path length and light-travel time.',
    'optical-path-si':'Bent-ray isotropic coordinate arclength converted to metres.',
    'source-lapse':'Static lapse of the emitting source at the recorded emission event.',
    'receiver-lapse':'Static lapse at the observer’s current reception position, available independently of a source hit.',
    'gr-frequency-ratio':'Static emission lapse divided by static reception lapse.',
    'local-doppler-ratio':'Local receiver SR frequency factor after removing the static gravitational lapse ratio. Beta and the backward sight-ray direction n_back are measured in the local static frame; n_back points from reception toward the source or prescribed sky, opposite the incoming photon propagation.',
    'color-display':'Whether received frequency shifts are used for the displayed source colors.',
    'intensity-display':'Whether the received frequency factor changes reference ray intensity.',
    resolution:'Pixel dimensions of the current internal rendering target.',
    'render-scale':'Actual internal image scale after adaptive resolution decisions.',
    'requested-scale':'Requested image-resolution scale before adaptive limits.',
    'adaptive-quality':'Whether resolution adapts to completed GPU timing observations.',
    'render-time':'Latest completed GPU timer-query duration. It is not a wall-clock frame interval or physical integration step.',
    'gpu-budget':'GPU timing budget used by the adaptive image-resolution controller.',
    'traced-instances':'Number of geometry/history instances offered to the current optical renderer.',
    'moving-instances':'Number of traced instances with declared nonzero source motion.',
    'gpu-vendor':'Vendor string reported by the current browser graphics context.',
    'gpu-renderer':'Renderer/adapter identity reported by the current graphics context. Identity alone does not verify optical accuracy.',
    'float-readback':'Whether the current context supports the floating-point diagnostic readback used by the GPU regression tests.',
    'environment-status':'Readiness or limitation reported for the current visual environment.',
    'horizon-radius':'Areal event-horizon radius of the nonrotating Schwarzschild metric. It is a null boundary, not a solid or emitting surface.',
    'horizon-isotropic-radius':'Isotropic-coordinate horizon radius, equal to one quarter of the scaled Schwarzschild radius. It is distinct from the areal horizon radius.',
    'horizon-area':'Area of a Schwarzschild horizon symmetry sphere in square metres.',
    'critical-impact':'Vacuum null-geodesic impact threshold at the unstable photon sphere. Directions on opposite sides of the threshold can escape or be captured.',
    'static-shadow-angle':'Shadow half-angle for a static exterior observer aimed at the center. Outside the photon sphere the angle is arcsine of bc A/r; inside it the angle is pi minus that arcsine, so the shadow exceeds a hemisphere. A moving observer sees local SR aberration.',
    'static-shadow-solid-angle':'Angular area of the static reference shadow cone, in steradians. The whole sky contains four pi steradians.',
    'horizon-curvature':'Schwarzschild Kretschmann scalar evaluated at the areal horizon. Its value is finite there; no interior curvature is simulated.',
    'sky-shell-radius':'Areal radius of the prescribed finite static sky. This is a radiance boundary for the optical experiment, not a horizon or a simulated astrophysical source distribution.',
    'sky-shell-lapse':'Static proper-clock lapse at the finite sky shell used to normalize incoming reference light.',
    'ray-measurement-source':'Provenance of the current sight-ray result, distinguishing a compiled GPU readback from an independent mathematical reference.',
    'ray-impact-parameter':'Conserved Schwarzschild null-ray impact parameter for the current local reception direction. This is a coordinate-invariant ray constant in the spherical exterior model.',
    'observer-exterior-guard':'Numerical stopping radius for the supported exterior chart, placed outside the horizon. Stopping here does not mean a horizon crossing or impact on a physical surface.',
};

/** @param {string} id @param {TooltipContext} context */
function rowMath(id, context) {
    if(context.profile!=='sr' && ['gamma','rapidity','sr-clock-rate','energy-rest','kinetic-rest','momentum-rest','killing-energy'].includes(id))return undefined;
    if (['speed','speed-si'].includes(id)) return context.profile==='sr'?equations.beta:undefined;
    if (id.startsWith('velocity-')) return context.profile==='sr'?equations.betaComponent:undefined;
    if (id === 'gamma') return equations.gamma;
    if (['proper-time','proper-time-si','clock-rate'].includes(id)) return context.curved ? equations.grClock : context.profile === 'sr' ? equations.srClock : undefined;
    if (id === 'sr-clock-rate') return equations.srClock;
    if (['energy-rest','kinetic-rest'].includes(id)) return equations.energy;
    if (id === 'momentum-rest') return equations.momentum;
    if (id === 'killing-energy') return equations.killing;
    if (['lapse','surface-lapse','receiver-lapse','source-lapse'].includes(id)) return equations.lapse;
    if (id === 'schwarzschild-radius' || id === 'horizon-radius') return equations.radius;
    if (id === 'proper-radial-distance') return context.blackHole ? equations.horizonDistance : equations.properDistance;
    if (['optical-index','coordinate-light-speed'].includes(id)) return equations.opticalIndex;
    if (id === 'gr-frequency-ratio') return equations.gravitationalFrequency;
    if (id === 'local-doppler-ratio') return equations.receiverFrequency;
    if (id.includes('redshift') || ['frequency-ratio','wavelength-ratio'].includes(id)) return equations.frequency;
    if (['bolometric-factor','intensity-display'].includes(id)) return equations.intensity;
    if (id.startsWith('optical-delay') || id === 'emission-time') return equations.delay;
    if (id.includes('hover-acceleration')) return equations.hover;
    if (id.includes('tidal')) return equations.tidal;
    if (id === 'kretschmann') return equations.curvature;
    if (['critical-impact','ray-impact-parameter'].includes(id)) return id === 'critical-impact' ? equations.criticalImpact : undefined;
    if (id === 'static-shadow-angle') return equations.shadow;
    if (id === 'static-shadow-solid-angle') return equations.solidAngle;
    if (id === 'horizon-area') return equations.horizonArea;
    if (id === 'horizon-curvature') return equations.horizonCurvature;
    return undefined;
}

/** @param {string} text */
export function cleanTooltipText(text) {
    return String(text).replace(/\[(?:AXIOM|THEOREM|DERIVED|SELECTION|CONJECTURE|IMPOSED|EMERGENT|OPEN|PARAMETRIC|SYNTHESIS)\]/g, '').replace(/\s+/g, ' ').trim();
}

/** Resolve from source-owned identifiers; labels are used only for plain text.
 * @param {HTMLElement} target @param {TooltipContext} context @returns {TooltipContent}
 */
export function observerTooltipContent(target, context) {
    const label = target.getAttribute('aria-label') || target.querySelector('.observer-field-label')?.textContent || target.textContent || 'Observer information';
    const legend = target.matches('input,select') ? target.closest('fieldset')?.querySelector('legend')?.textContent : null;
    const title = cleanTooltipText(target.dataset.observerTooltipLabel || (legend ? legend + ' · ' + label : label)).slice(0, 140);
    const key = target.dataset.observerSetting;
    if (key) {
        const root = key.split('.')[0];
        let text = settings[root];
        if (root === 'axisLocks') text = 'Prevent navigation displacement along the named coordinate axis. The remaining axes remain available.';
        if (root === 'overlayFields') text = 'Show this recorded source quantity in spatial labels. Labels retain the same current/historical observation convention as the selected view.';
        if (root === 'layers') text = layers[key.split('.')[1]] || 'Show this explanatory geometry without altering source integration.';
        if(root==='speed' && context.profile==='playground')text='Target coordinate navigation speed in simulation length units per time unit. This classical preparation does not assign beta or a relativistic clock rate to it.';
        const math = root === 'doppler' ? equations.frequency : root === 'beaming' ? equations.intensity : root === 'speed' && context.profile==='sr' ? equations.beta : root === 'acceleration' && context.profile === 'sr' && !context.curved ? equations.force : root === 'optical' && !context.curved && context.profile==='sr' ? equations.null : root === 'waveWavelength' ? equations.wave : undefined;
        return {title,text:text || 'Control the declared ' + title.toLowerCase() + ' presentation setting.',math};
    }
    const field = target.dataset.observerEntityField;
    if (field) return {title,text:field.startsWith('velocity') && context.profile==='playground' ? 'Classical source coordinate velocity component in simulation length units per time unit. No light-speed normalization is assigned to this body value.' : entity[field.split('.')[0]] || 'Prepare this source property as part of an explicit author edit.',math:field.startsWith('coordinateForcePerMass') && context.profile==='sr' ? equations.force : field.startsWith('velocity') && context.profile === 'sr' ? equations.betaComponent : undefined};
    if (target.dataset.observerEnvironment !== undefined) return {title,text:environment[target.dataset.observerEnvironment] || 'Control this visual background parameter without changing source physics.'};
    const rowId = target.dataset.observerTelemetry || target.closest('[data-observer-telemetry]')?.getAttribute('data-observer-telemetry') || target.closest('.observer-telemetry-reading')?.querySelector('[data-observer-telemetry]')?.getAttribute('data-observer-telemetry');
    if (rowId) {
        const note = target.closest('.observer-telemetry-reading')?.querySelector('.observer-telemetry-note')?.textContent || '';
        let text = telemetry[rowId] || (rowId.startsWith('position-') ? 'Current observer-center coordinate along the named axis.' : rowId.startsWith('velocity-') ? 'Observer velocity component in the declared local or coordinate measuring frame.' : rowId.startsWith('lattice-') ? 'Completed cached lattice publication from its separate owner, source and sample tick.' : target.dataset.observerTooltipDescription || 'Current recorded ' + title.toLowerCase() + ' from the declared reference observation.');
        if(context.blackHole && rowId==='optical-status')text='Current exterior sight-ray classification: captured rays remain dark, escaped rays receive the prescribed finite stationary sky, and unresolved rays have no received emission measurement. An escaped sky ray can supply a frequency ratio and travel delay without a source-entity hit or retained source clock.';
        if(context.profile==='playground' && ['proper-time','proper-time-si','clock-rate','coordinate-time','speed'].includes(rowId))text=rowId==='speed'?'Coordinate navigation speed in simulation length units per time unit. No beta or SI light-speed calibration is assigned.':'Classical elapsed-time convention: the observer clock follows coordinate elapsed simulation time. This is not a relativistic proper-clock measurement.';
        if (note && !text.includes(note)) text += ' ' + note;
        return {title,text:cleanTooltipText(text),math:rowMath(rowId,context)};
    }
    const action = target.dataset.observerAction;
    if (action === 'panel') return {title,text:panels[target.dataset.value || ''] || 'Open this observer control drawer.'};
    if (action === 'compact-star-motion') return {title,text:target.dataset.value === 'freefall' ? 'Release the supported observer into an exterior timelike geodesic. Navigation force is disabled during free fall. The numerical exterior guard and speed limit remain active.' : 'Apply an ideal stopping impulse and hold the observer static. This requires support acceleration; it is not free fall.',math:equations.grClock};
    if (action === 'experiment') return {title:cleanTooltipText(target.closest('article')?.querySelector('h3,h4')?.textContent || title),text:cleanTooltipText(target.closest('article')?.querySelector('.observer-description')?.textContent || 'Replace the current preparation with this repeatable reference experiment. Its clocks and source histories restart.')};
    if (action) return {title,text:actions[action] || 'Perform this explicit observer action on the current preparation.'};
    if (target.hasAttribute('data-observer-binding')) return {title,text:'Choose the physical keyboard key for ' + title.toLowerCase() + '. Press a new key while this field is focused. Duplicate bindings are rejected.'};
    if (target.hasAttribute('data-observer-scrub')) return {title,text:'View a retained source-coordinate time. Scrubbing pauses source playback and does not rewind the observer’s retained proper clock.'};
    if (target.hasAttribute('data-observer-profile')) return {title,text:'Choose classical Playground body mechanics or the normalized Minkowski special-relativistic reference preparation.'};
    if (target.hasAttribute('data-observer-playback-speed')) return {title,text:'Change simulation playback per wall-clock time without redefining coordinate or proper clock units.'};
    if (target.hasAttribute('data-observer-units')) return {title,text:'Edit the visible unit label. This does not perform SI calibration or change the experiment’s normalized equations.'};
    if (target.hasAttribute('data-observer-autosave')) return {title,text:'Enable periodic local saves. Autosave is off initially; named saves are never silently evicted.'};
    if (target.hasAttribute('data-observer-storage-cap')) return {title,text:'Maximum local storage use in mebibytes. A mebibyte is 1,048,576 bytes; increasing it does not bypass browser quota limits.'};
    if (target.hasAttribute('data-observer-import')) return {title,text:'Choose an exported Observer JSON file. Import validates finite values, retained history and the declared reference-model conventions before replacing the current world.'};
    if (target.hasAttribute('data-observer-save-name')) return {title,text:'Name the local saved world. Its session identity and physics convention are retained separately.'};
    if (target.hasAttribute('data-observer-entity-select')) return {title,text:'Choose the source whose current properties you want to inspect. A received image can refer to an earlier revision of that source.'};
    if (target.hasAttribute('data-observer-create-shape')) return {title,text:'Choose the geometry for the next authored source. Full extents and rest axes are declared when the source is prepared.'};
    if (target.hasAttribute('data-observer-impulse')) return {title,text:'Classical momentum change along this coordinate axis. The velocity change is momentum impulse divided by body mass.'};
    if (target.hasAttribute('data-observer-joint-target')) return {title,text:'Choose the other Playground body that becomes the second endpoint of the spring.'};
    if (target.hasAttribute('data-observer-joint-field')) return {title,text:'Declare this spring rest length, restoring stiffness or dissipative damping in Playground simulation units.'};
    if (target.hasAttribute('data-observer-global-gravity')) return {title,text:'Uniform classical gravitational acceleration component along the named coordinate axis.'};
    if (target.hasAttribute('data-observer-world-physics')) return {title,text:'Stage the named classical gravity or collision parameter, then apply world physics to commit the whole preparation.'};
    if (target.hasAttribute('data-observer-lattice-field')) return {title,text:'Cached value published by the separate retained lattice owner. Its source, epoch and completed sample tick determine its meaning.'};
    if (target.hasAttribute('data-observer-worldline')) return {title:'Observer clock origin',text:'Identifies the observer path whose proper time is accumulated. Explicit relocation starts a new origin; clock differences require matching coordinate and proper-clock origins.'};
    if (target.hasAttribute('data-observer-history-coverage')) return {title:'Retained optical interval',text:context.blackHole?'The black-hole experiment has no emitting source entities or retained entity clocks. Its prescribed finite stationary sky is an optical boundary available at all reference times, independent of the empty entity-history interval. Escaped rays can therefore supply light travel and frequency measurements; captured or unresolved rays do not.':'Only source events inside the displayed retained interval can supply a delayed image. Earlier unavailable events are not replaced by the current source pose.'};
    if (target.hasAttribute('data-observer-compact-star')) return {title:context.blackHole?'Exterior observer reference':'Compact-star reference',text:context.blackHole?'The horizon radius belongs to the metric. A guided observer needs support; free fall stops at the implemented exterior guard rather than crossing the horizon. The displayed clock rate includes local motion and lapse.':'The tabletop image represents a calibrated compact-star exterior. Surface redshift to infinity differs from the current received shift. The displayed observer clock rate includes its actual local motion and lapse.',math:equations.grClock};
    if (target.hasAttribute('data-observer-force-gun')) return {title:'Classical tether effort',text:'Live requested force and force cap for the Playground tether. Effort is requested force relative to the available limit; exceeding the limit keeps the attachment but bounds the applied force. Mass and force retain simulation units.'};
    if (target.matches('legend.observer-field-label')) return {title,text:'Vector components refer to the declared coordinate or rest axes. Position and full dimensions use length units; rotations use the displayed angular units; velocities and impulses follow the active model convention.'};
    const metric = target.closest('.observer-hud-metric');
    if (metric) {
        const heading = metric.querySelector('.observer-hud-label')?.textContent || '';
        if (heading.includes('PROPER') || heading.includes('YOUR')) return {title:context.profile==='playground'?'Observer elapsed clock':'Observer proper clock',text:context.profile==='playground'?'Classical observer elapsed clock follows coordinate elapsed simulation time. This profile does not integrate a relativistic proper clock.':telemetry['proper-time'],math:context.curved ? equations.grClock : context.profile === 'sr' ? equations.srClock : undefined};
        if (heading.includes('SPEED')) return {title:'Observer speed',text:context.profile==='playground'?'Classical coordinate speed in simulation length units per time unit. No light-speed normalization is assigned.':telemetry.speed,math:context.profile === 'sr' ? equations.beta : undefined};
        if (heading.includes('LORENTZ')) return {title:'Lorentz factor',text:telemetry.gamma,math:equations.gamma};
        return {title:'Coordinate time',text:telemetry['coordinate-time']};
    }
    if (target.matches('summary')) return {title,text:cleanTooltipText(target.closest('details')?.querySelector('.observer-description')?.textContent || 'Expand or collapse these reference readouts. Values keep updating without changing playback.')};
    if (target.matches('.observer-canvas')) return {title:'First-person scene',text:'Click to capture pointer look. Hold your movement keys to travel; Escape releases capture. The view traces the declared reference scene. Hover explanations are suppressed while the pointer is captured.'};
    if (target.matches('.observer-reticle')) return {title:'Current sight ray',text:'The center crosshair defines the received direction used for source inspection and optical telemetry. Press E to inspect a valid source; a miss supplies no source clock.',math:context.curved ? undefined : context.profile === 'sr' ? equations.null : undefined};
    if (target.matches('.observer-spatial-label')) return {title:'Source observation label',text:'A pooled annotation of the current view’s source identity, emission event and selected quantities. A clock is the source clock at emission; a historical image is not replaced by the current source pose. Mirrored images share the original source history.',math:context.profile === 'sr' ? equations.srClock : undefined};
    if (target.hasAttribute('data-observer-spacetime')) return {title:'Spacetime slice',text:'A coordinate-time slice through the observer event. Cyan slopes are null rays; purple compares the observer simultaneity slice; gold marks the received source event. Projected off-axis histories are explanatory coordinate geometry.',math:equations.null};
    if (target.hasAttribute('data-observer-aberration')) return {title:'Full-sky aberration',text:'Compare coordinate source directions with received observer-frame directions over the whole sky. The diagram is independent of perspective field of view and does not assign finite source travel times.',math:equations.beta};
    if (target.closest('.observer-gizmo')) return {title,text:'Author transform handle: choose move, rotate or scale, then drag a named coordinate axis. Dragging updates a preview; releasing commits one source revision. Grid snapping affects author placement only.'};
    if (target.hasAttribute('data-observer-frame-readout')) return {title:'Reference frame clocks',text:'Local beta and gamma determine the reference SR clock rate; coordinate time is shown separately.',math:equations.srClock};
    if (target.hasAttribute('data-observer-received-event')) return {title:'Received emission event',text:'Coordinate delay, source clock and frequency ratio belong to this valid sight-ray emission witness. A stale or missing witness supplies no replacement event.',math:equations.delay};
    if (target.matches('.observer-telemetry-count')) return {title:'Category readout count',text:'Number of declared measurements in this category. A missing calibration or current sight-ray witness remains unavailable rather than being filled with an assumed value.'};
    if (target.matches('.observer-telemetry-summary')) return {title:'Telemetry status',text:'Transport state, current readout count and reference model for this live dropdown. Readings observe the current session; opening telemetry does not pause it.'};
    if (target.matches('.observer-title')) return {title:'Mind’s Eye observer',text:'Explore the declared special-relativistic, curved-spacetime or classical reference preparation in first person. Settings, source authoring and optical observations have distinct roles.'};
    if (target.matches('.observer-badge')) return {title:'Reference and rendering badge',text:'The active model, optical-view convention or current image-quality reduction. A quality reduction changes the image resolution rather than source clocks or physical velocity.'};
    if (target.matches('.observer-eyebrow')) return {title:'Observer context',text:'This label identifies the workspace or the current sight-ray information shown below it. Source inspection uses the event whose light reaches the present view.'};
    if (target.matches('.observer-section-title')) return {title,text:cleanTooltipText(target.parentElement?.querySelector('.observer-description')?.textContent || 'This section groups the named observer controls; each setting has its own explanation.')};
    if (target.matches('.observer-panel-head h2')) return {title,text:panels[target.closest('[data-observer-panel]')?.getAttribute('data-observer-panel') || ''] || 'This drawer groups the current observer controls and source-backed measurements. Close it to return focus to the scene.'};
    if (target.matches('.observer-experiment h4')) return {title,text:cleanTooltipText(target.closest('article')?.querySelector('.observer-description')?.textContent || 'A repeatable declared reference preparation; entering it replaces the current sandbox and restarts its clocks.')};
    if (target.matches('.observer-save strong')) return {title:'Saved world',text:'Local name of this stored preparation. Its independent session identity, physics convention, settings and retained histories are validated when loaded.'};
    if (target.matches('input,select')) return {title,text:'Set the declared ' + title.toLowerCase() + ' value for this preparation. Input bounds and reference-model restrictions apply.'};
    return {title:target.matches('.observer-description') ? 'Measurement note' : title,text:cleanTooltipText(target.dataset.observerTooltipDescription || (target.matches('.observer-description,.observer-worldline-readout,.observer-history-readout,[data-observer-status]') ? target.textContent : null) || 'Current view information from the declared observer preparation. Its source identity and clock convention remain separate from editable world properties.')};
}

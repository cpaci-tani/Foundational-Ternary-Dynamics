import * as THREE from 'three';

export const GLOBAL_CLOCK_PHASES = Object.freeze([
    Object.freeze({
        name: 'read', title: 'Read', color: 0x60a5fa,
        description: 'Reads the settled cell records and computes the next field deltas without committing them yet.',
    }),
    Object.freeze({
        name: 'write', title: 'Write', color: 0xa78bfa,
        description: 'Commits the field update and any enabled damping, noise, genesis, or expiry terms.',
    }),
    Object.freeze({
        name: 'pair', title: 'Pair production', color: 0xf472b6,
        description: 'Runs the optional correlated pair-production transaction when that physics term is enabled.',
    }),
    Object.freeze({
        name: 'Gauss', title: 'Gauss projection', color: 0x22d3ee,
        description: 'Runs the optional finite-lattice constraint projection that targets the selected Gauss relation.',
    }),
    Object.freeze({
        name: 'latency', title: 'Latency solve', color: 0x818cf8,
        description: 'Solves the optional imposed latency field used by the engine causal-budget mapping.',
    }),
    Object.freeze({
        name: 'forces', title: 'Forces', color: 0xfb923c,
        description: 'Evaluates enabled field-mediated force terms and updates the velocity budget.',
    }),
    Object.freeze({
        name: 'movement', title: 'Movement', color: 0xfacc15,
        description: 'Applies guarded integer transport, collision, bounce, annihilation, and self-field transfer.',
    }),
    Object.freeze({
        name: 'boundary', title: 'Boundary', color: 0x34d399,
        description: 'Settles the selected dispersal, reflective, or periodic finite-box boundary transaction.',
    }),
    Object.freeze({
        name: 'weak/triad', title: 'Weak / triad', color: 0xc084fc,
        description: 'Runs the optional weak-transmutation and triad-binding sector after movement and boundary settlement.',
    }),
    Object.freeze({
        name: 'proper time', title: 'Proper-time mapping', color: 0x2dd4bf,
        description: 'Accumulates the selected mapped proper-time rate and advances an enabled imposed internal clock phase.',
    }),
]);

const STATIC_ENTRIES = Object.freeze({
    clock: Object.freeze({
        title: 'Global ordinal clock',
        status: '[AXIOM + ENGINE ORDER]',
        color: 0x7dd3fc,
        description: 'One displayed tick is one complete deterministic engine transaction. It is an ordinal update count, not wall-clock seconds.',
    }),
    dial: Object.freeze({
        title: 'Ten-stage transaction dial',
        status: '[IMPLEMENTED ORDER]',
        color: 0xe0f2fe,
        description: 'The ten marks partition the implemented Scale-0 transaction order. They do not expose a simultaneously observable sub-tick state.',
    }),
    hand: Object.freeze({
        title: 'Ordinal hand',
        status: '[AXIOM READOUT]',
        color: 0xfbbf24,
        description: 'A base-ten odometer for settled global ticks. The hand advances only after the full transaction has completed.',
    }),
    rate: Object.freeze({
        title: 'Mapped local-rate band',
        status: '[IMPOSED MAPPING]',
        color: 0x38bdf8,
        description: 'Color encodes the slowest mapped rate τ′min=√max(0,1−Bmax): cyan is unconstrained, amber is loaded, and rose approaches the causal limit. This is not recovered spacetime.',
    }),
    cursor: Object.freeze({
        title: 'Transaction replay cursor',
        status: '[VISUALIZATION]',
        color: 0xffffff,
        description: 'The white cursor replays the ordered stages after a settled tick. It is presentation timing, not live telemetry from inside the synchronous transaction.',
    }),
    arrow: Object.freeze({
        title: 'Forward update direction',
        status: '[AXIOM + SELECTION]',
        color: 0xfbbf24,
        description: 'Clockwise marks the adopted forward update direction. The selected non-injective expiry sector supplies that arrow; the reversible wave sector alone does not.',
    }),
    c4: Object.freeze({
        title: 'C4 / quartic-clock reference',
        status: '[CONDITIONAL · OPEN]',
        color: 0xc084fc,
        description: 'The muted fourfold motif references the conditional C4 and quartic-carrier programme. It is not production telemetry and does not claim that Scale 0 already realizes a native G* clock.',
    }),
});

const hexColor = value => `#${Number(value).toString(16).padStart(6, '0')}`;

export function tagClockHover(object, key) {
    if (object?.userData) object.userData.clockHoverKey = key;
    return object;
}

export class GlobalClockHoverController {
    constructor({ renderer, camera, container, getClock, getState }) {
        this.renderer = renderer;
        this.camera = camera;
        this.container = container;
        this.getClock = getClock;
        this.getState = getState;
        this.raycaster = new THREE.Raycaster();
        this.raycaster.params.Line.threshold = 0.45;
        this.pointer = new THREE.Vector2();
        this.overlay = null;
        this.refs = null;
        this._onPointerMove = event => this._handlePointerMove(event);
        this._onPointerLeave = () => this.hide();
    }

    init() {
        const canvas = this.renderer?.domElement;
        if (!canvas || !this.container || typeof document === 'undefined') return this;
        const overlay = document.createElement('div');
        overlay.className = 'scale0-clock-hover';
        overlay.setAttribute('role', 'tooltip');
        overlay.hidden = true;

        const header = document.createElement('div');
        header.className = 'scale0-clock-hover-header';
        const swatch = document.createElement('span');
        swatch.className = 'scale0-clock-hover-swatch';
        const heading = document.createElement('span');
        heading.className = 'scale0-clock-hover-title';
        header.append(swatch, heading);

        const status = document.createElement('div');
        status.className = 'scale0-clock-hover-status';
        const description = document.createElement('div');
        description.className = 'scale0-clock-hover-description';
        const live = document.createElement('div');
        live.className = 'scale0-clock-hover-live';
        overlay.append(header, status, description, live);
        this.container.appendChild(overlay);
        this.overlay = overlay;
        this.refs = { swatch, heading, status, description, live };

        canvas.addEventListener('pointermove', this._onPointerMove, { passive: true });
        canvas.addEventListener('pointerleave', this._onPointerLeave, { passive: true });
        return this;
    }

    _entryFor(key) {
        if (key?.startsWith('phase-')) {
            const index = Number(key.slice(6));
            const phase = GLOBAL_CLOCK_PHASES[index];
            if (!phase) return null;
            return {
                title: `Stage ${index + 1} · ${phase.title}`,
                status: '[IMPLEMENTED ORDER]',
                color: phase.color,
                description: phase.description,
            };
        }
        return STATIC_ENTRIES[key] ?? null;
    }

    _liveText(key) {
        const state = this.getState?.() ?? {};
        if (key === 'rate') {
            if (!state.hasCausalBudget) return 'Live causal-budget telemetry is unavailable.';
            const projection = state.projectionEvents > 0
                ? ` · ${state.projectionEvents} causal projection event${state.projectionEvents === 1 ? '' : 's'}`
                : '';
            return `Live Bmax ${state.causalBudget.toFixed(4)} · τ′min ${state.rate.toFixed(4)}${projection}`;
        }
        if (key === 'hand' || key === 'clock' || key === 'dial') {
            return `Live tick ${state.tick ?? 0} · ${state.running ? 'running' : 'idle'}`;
        }
        if (key === 'cursor') {
            const phase = GLOBAL_CLOCK_PHASES[state.activeReplayPhase];
            return phase ? `Replaying: ${phase.title}` : 'No transaction replay is active.';
        }
        if (key === 'arrow') return 'Direction: clockwise / increasing global tick.';
        if (key === 'c4') return 'Production clock telemetry: absent by design.';
        if (key?.startsWith('phase-')) return 'Optional stages run only when their owning physics terms are enabled.';
        return '';
    }

    _hoverKeyFromHit(hit, clock) {
        let object = hit?.object ?? null;
        while (object && object !== clock) {
            const key = object.userData?.clockHoverKey;
            if (key) {
                if (key === 'cursor' && Number(object.material?.opacity) < 0.08) return null;
                return key;
            }
            object = object.parent;
        }
        return clock?.userData?.clockHoverKey ?? null;
    }

    _handlePointerMove(event) {
        if (document.documentElement.dataset.tooltips === 'off') {
            this.hide();
            return;
        }
        const clock = this.getClock?.();
        const canvas = this.renderer?.domElement;
        if (!clock?.visible || !canvas || !this.camera) {
            this.hide();
            return;
        }
        const rect = canvas.getBoundingClientRect();
        if (rect.width <= 0 || rect.height <= 0) return;
        this.pointer.set(
            ((event.clientX - rect.left) / rect.width) * 2 - 1,
            -((event.clientY - rect.top) / rect.height) * 2 + 1,
        );
        this.raycaster.setFromCamera(this.pointer, this.camera);
        const hits = this.raycaster.intersectObjects(clock.children, true);
        let key = null;
        for (const hit of hits) {
            key = this._hoverKeyFromHit(hit, clock);
            if (key) break;
        }
        if (!key) {
            this.hide();
            return;
        }
        this.show(key, event.clientX, event.clientY);
    }

    show(key, clientX, clientY) {
        const entry = this._entryFor(key);
        if (!entry || !this.overlay || !this.refs) return;
        this.refs.heading.textContent = entry.title;
        this.refs.status.textContent = entry.status;
        this.refs.description.textContent = entry.description;
        this.refs.live.textContent = this._liveText(key);
        this.refs.live.hidden = !this.refs.live.textContent;
        this.refs.swatch.style.background = hexColor(entry.color);
        this.overlay.dataset.clockHoverKey = key;
        this.overlay.hidden = false;
        this._position(clientX, clientY);
    }

    _position(clientX, clientY) {
        if (!this.overlay || this.overlay.hidden) return;
        const rect = this.container.getBoundingClientRect();
        const gap = 14;
        const margin = 10;
        const width = this.overlay.offsetWidth;
        const height = this.overlay.offsetHeight;
        let left = clientX - rect.left + gap;
        let top = clientY - rect.top + gap;
        if (left + width > rect.width - margin) left = clientX - rect.left - width - gap;
        if (top + height > rect.height - margin) top = clientY - rect.top - height - gap;
        this.overlay.style.left = `${Math.max(margin, left)}px`;
        this.overlay.style.top = `${Math.max(margin, top)}px`;
    }

    hide() {
        if (this.overlay) this.overlay.hidden = true;
    }

    dispose() {
        const canvas = this.renderer?.domElement;
        canvas?.removeEventListener('pointermove', this._onPointerMove);
        canvas?.removeEventListener('pointerleave', this._onPointerLeave);
        this.overlay?.remove();
        this.overlay = null;
        this.refs = null;
    }
}

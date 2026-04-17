import { resolveLayoutMode, resolveOrientation, isCompactLayout, isTabletLayout } from './layout-state.js';

/**
 * Observes viewport size and emits shell layout snapshots.
 */
export class BreakpointService {
    constructor({ onChange } = {}) {
        this._onChange = typeof onChange === 'function' ? onChange : null;
        this._snapshot = null;
        this._handleResize = this._handleResize.bind(this);
    }

    start() {
        this._publishSnapshot();
        window.addEventListener('resize', this._handleResize, { passive: true });
        window.addEventListener('orientationchange', this._handleResize, { passive: true });
        return this.getSnapshot();
    }

    stop() {
        window.removeEventListener('resize', this._handleResize);
        window.removeEventListener('orientationchange', this._handleResize);
    }

    getSnapshot() {
        return this._snapshot;
    }

    _handleResize() {
        this._publishSnapshot();
    }

    _publishSnapshot() {
        const next = this._buildSnapshot();
        const prev = this._snapshot;
        const changed = !prev
            || prev.width !== next.width
            || prev.height !== next.height
            || prev.layoutMode !== next.layoutMode
            || prev.orientation !== next.orientation;
        if (!changed) return;
        this._snapshot = next;
        if (this._onChange) this._onChange(next, prev);
    }

    _buildSnapshot() {
        const width = window.innerWidth;
        const height = window.innerHeight;
        const layoutMode = resolveLayoutMode(width);
        const orientation = resolveOrientation(width, height);
        return {
            width,
            height,
            layoutMode,
            orientation,
            isCompact: isCompactLayout(layoutMode),
            isTablet: isTabletLayout(layoutMode),
        };
    }
}

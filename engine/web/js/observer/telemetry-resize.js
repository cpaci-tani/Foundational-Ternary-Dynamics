// @ts-check
/** Telemetry presentation sizing only; resizing never changes the simulation. */
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';

/** @typedef {{width:number,height:number}} PanelSize */
/** @typedef {{pointerId:number,x:number,y:number,width:number,height:number}} ResizeDrag */

/**
 * Keep the centered telemetry panel within the viewport and above visible tools.
 * Preferred dimensions belong to this UI lifetime and survive closing the panel.
 * @param {{panel:HTMLElement,handle:HTMLElement,root:HTMLElement,description:HTMLElement}} options
 */
export function createTelemetryResize({ panel, handle, root, description }) {
    const scope = new LifetimeScope();
    let active = false;
    let disposed = false;
    /** @type {PanelSize|null} */ let preferred = null;
    /** @type {ResizeDrag|null} */ let drag = null;

    function availableSize() {
        const viewportWidth = Math.min(window.innerWidth, document.documentElement.clientWidth || window.innerWidth);
        const rootBounds = root.getBoundingClientRect();
        const panelBounds = panel.getBoundingClientRect();
        const margin = viewportWidth <= 600 ? 10 : viewportWidth <= 900 ? 16 : 20;
        const center = rootBounds.left + rootBounds.width / 2;
        const left = Math.max(margin, rootBounds.left + margin);
        const right = Math.min(viewportWidth - margin, rootBounds.right - margin);
        let bottom = Math.min(window.innerHeight, rootBounds.bottom) - 12;
        for (const control of root.querySelectorAll('.observer-toolbar, .observer-transport')) {
            const style = window.getComputedStyle(control);
            if (!control.getClientRects().length || style.display === 'none' || style.visibility === 'hidden') continue;
            bottom = Math.min(bottom, control.getBoundingClientRect().top - 12);
        }
        return {
            width: Math.max(1, Math.floor(2 * Math.min(center - left, right - center))),
            height: Math.max(1, Math.floor(bottom - panelBounds.top)),
        };
    }

    function announceSize() {
        if (!active || disposed) return;
        const bounds = panel.getBoundingClientRect();
        description.textContent = `Width ${Math.round(bounds.width)} CSS pixels; height ${Math.round(bounds.height)} CSS pixels. Arrow keys resize; Shift uses larger steps; Home or double-click restores default.`;
    }

    /** @param {PanelSize} requested @param {boolean} remember */
    function applySize(requested, remember) {
        const available = availableSize();
        const next = {
            width: Math.max(Math.min(320, available.width), Math.min(available.width, requested.width)),
            height: Math.max(Math.min(200, available.height), Math.min(available.height, requested.height)),
        };
        if (remember) preferred = next;
        panel.style.width = `${next.width}px`;
        panel.style.height = `${next.height}px`;
        panel.dataset.observerTelemetrySized = 'true';
        announceSize();
    }

    function cancelDrag() {
        const previous = drag;
        drag = null;
        delete panel.dataset.observerTelemetryResizing;
        if (previous && handle.hasPointerCapture(previous.pointerId)) {
            handle.releasePointerCapture(previous.pointerId);
        }
    }

    function clearSize() {
        panel.style.removeProperty('width');
        panel.style.removeProperty('height');
        delete panel.dataset.observerTelemetrySized;
    }

    function reset() {
        if (disposed) return;
        cancelDrag();
        preferred = null;
        clearSize();
        announceSize();
    }

    /** @param {PointerEvent} event */
    function startDrag(event) {
        if (!active || disposed || drag || event.button !== 0 || !event.isPrimary) return;
        event.preventDefault();
        event.stopPropagation();
        const bounds = panel.getBoundingClientRect();
        drag = { pointerId: event.pointerId, x: event.clientX, y: event.clientY, width: bounds.width, height: bounds.height };
        panel.dataset.observerTelemetryResizing = 'true';
        handle.focus({ preventScroll: true });
        handle.setPointerCapture(event.pointerId);
    }

    /** @param {PointerEvent} event */
    function moveDrag(event) {
        if (!drag || event.pointerId !== drag.pointerId) return;
        event.preventDefault();
        event.stopPropagation();
        applySize({
            // The panel stays centered, so the opposite edge moves equally far.
            width: drag.width + 2 * (event.clientX - drag.x),
            height: drag.height + event.clientY - drag.y,
        }, true);
    }

    /** @param {PointerEvent} event */
    function endDrag(event) {
        if (!drag || event.pointerId !== drag.pointerId) return;
        event.preventDefault();
        event.stopPropagation();
        cancelDrag();
    }

    /** @param {KeyboardEvent} event */
    function keyResize(event) {
        if (!active || disposed) return;
        if (event.key === 'Home') {
            event.preventDefault();
            event.stopPropagation();
            reset();
            return;
        }
        const step = event.shiftKey ? 80 : 20;
        const widthDelta = event.key === 'ArrowRight' ? step : event.key === 'ArrowLeft' ? -step : 0;
        const heightDelta = event.key === 'ArrowDown' ? step : event.key === 'ArrowUp' ? -step : 0;
        if (!widthDelta && !heightDelta) return;
        event.preventDefault();
        event.stopPropagation();
        cancelDrag();
        const bounds = panel.getBoundingClientRect();
        applySize({ width: bounds.width + widthDelta, height: bounds.height + heightDelta }, true);
    }

    scope.on(handle, 'pointerdown', startDrag);
    scope.on(window, 'pointermove', moveDrag, { passive: false });
    scope.on(window, 'pointerup', endDrag);
    scope.on(window, 'pointercancel', endDrag);
    scope.on(handle, 'lostpointercapture', event => {
        if (drag && event.pointerId === drag.pointerId) cancelDrag();
    });
    scope.on(handle, 'keydown', keyResize);
    scope.on(handle, 'dblclick', event => {
        if (!active || disposed) return;
        event.preventDefault();
        event.stopPropagation();
        reset();
    });
    scope.on(window, 'resize', () => {
        if (!active || disposed) return;
        cancelDrag();
        if (preferred) applySize(preferred, false);
        else announceSize();
    });

    return {
        open() {
            if (disposed) return;
            active = true;
            if (preferred) applySize(preferred, false);
            else announceSize();
        },
        close() {
            cancelDrag();
            active = false;
            clearSize();
        },
        reset,
        dispose() {
            if (disposed) return;
            cancelDrag();
            active = false;
            disposed = true;
            clearSize();
            scope.dispose();
        },
    };
}

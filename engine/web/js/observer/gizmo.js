// @ts-check
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';

/** Screen-space author handles. Drags preview freely and commit one revision. */
export class ObserverGizmo {
    /** @param {{host:HTMLElement,getEntity:()=>any,onPreview:(entity:any)=>void,onCommit:(id:string,patch:Record<string,unknown>)=>void,getSnap:()=>number}} options */
    constructor({ host, getEntity, onPreview, onCommit, getSnap }) {
        this.scope = new LifetimeScope();
        this.element = document.createElement('div');
        this.element.className = 'observer-gizmo';
        this.element.hidden = true;
        this.element.setAttribute('aria-label', 'Author transform handles');
        this.element.innerHTML = '<select aria-label="Transform tool"><option value="position">Move</option><option value="rotation">Rotate</option><option value="size">Scale</option></select><button type="button" data-axis="0" aria-label="Drag X axis">X ↔</button><button type="button" data-axis="1" aria-label="Drag Y axis">Y ↕</button><button type="button" data-axis="2" aria-label="Drag Z axis">Z ↗</button><span>AUTHOR PREVIEW</span>';
        host.append(this.element);
        /** @type {{pointer:number,x:number,y:number,axis:number,field:string,entity:any,patch:Record<string,unknown>}|null} */
        this.drag = null;
        this.scope.on(this.element, 'pointerdown', (/** @type {PointerEvent} */ event) => {
            const button = event.target instanceof Element ? event.target.closest('button[data-axis]') : null;
            const entity = getEntity();
            if (!(button instanceof HTMLButtonElement) || !entity?.alive) return;
            event.preventDefault();
            button.setPointerCapture(event.pointerId);
            this.drag = { pointer: event.pointerId, x: event.clientX, y: event.clientY,
                axis: Number(button.dataset.axis), field: /** @type {HTMLSelectElement} */ (this.element.querySelector('select')).value,
                entity: structuredClone(entity), patch: {} };
        });
        this.scope.on(this.element, 'pointermove', (/** @type {PointerEvent} */ event) => {
            const drag = this.drag;
            if (!drag || event.pointerId !== drag.pointer) return;
            const value = [...drag.entity[drag.field]];
            const distance = drag.axis === 1 ? drag.y - event.clientY : event.clientX - drag.x;
            const step = drag.field === 'rotation' ? Math.PI / 180 : 0.02;
            let n = value[drag.axis] + distance * step;
            const snap = getSnap();
            if (snap > 0 && drag.field !== 'rotation') n = Math.round(n / snap) * snap;
            if (drag.field === 'size') n = Math.max(0.02, n);
            value[drag.axis] = n;
            drag.patch = { [drag.field]: value };
            onPreview({ ...drag.entity, ...drag.patch });
        });
        this.scope.on(this.element, 'pointerup', (/** @type {PointerEvent} */ event) => {
            const drag = this.drag;
            if (!drag || drag.pointer !== event.pointerId) return;
            this.drag = null;
            if (Object.keys(drag.patch).length) onCommit(drag.entity.id, drag.patch);
            else onPreview(null);
        });
        this.scope.on(this.element, 'pointercancel', () => { this.drag = null; onPreview(null); });
    }
    /** @param {boolean} visible */
    show(visible) { this.element.hidden = !visible; }
    dispose() { this.scope.dispose(); this.element.remove(); }
}

import { formatLength } from './measure.js';
import { SCALE_LANDMARKS, metresToVoxels, visibleLandmarks } from './scale-landmarks.js';

/** A length in voxels as metres, three decimals (step = the length itself). */
const metresText = (voxels) => formatLength(voxels, voxels);

/**
 * Reference rings for the zoom-out beyond the lattice. Each ring is a circle
 * whose diameter is one reference length, centred on the lattice, so a ring
 * that fills the view means the view is about that wide. Nothing here is
 * simulated: the layer says so, and names the next reference further out
 * while the view crosses the range in which no ring is on screen.
 *
 * Like the other readouts, every element remembers what it last wrote and
 * writes only a change.
 */

const SVG_NS = 'http://www.w3.org/2000/svg';
const LATTICE_DOT_MAX_PX = 8;
const LABEL_PAD_PX = 14;
// Labels are one line each; closer than this they would overlap.
const LABEL_GAP_PX = 24;
// Every label sits where its ring crosses one diagonal, so the labels read
// outward in the same order as the rings: innermost lowest.
const LABEL_ANGLE = (40 * Math.PI) / 180;

export function createLandmarkRings() {
    const root = document.createElement('div');
    root.className = 'scale-landmarks';
    root.hidden = true;
    const svg = document.createElementNS(SVG_NS, 'svg');
    svg.setAttribute('class', 'scale-landmarks-rings');
    svg.setAttribute('aria-hidden', 'true');
    const centre = document.createElement('div');
    centre.className = 'scale-landmarks-centre';
    const centreDot = document.createElement('span');
    centreDot.className = 'scale-landmarks-dot';
    const centreLabel = document.createElement('span');
    centreLabel.className = 'scale-landmarks-centre-label';
    centre.append(centreDot, centreLabel);
    const caption = document.createElement('div');
    caption.className = 'scale-landmarks-caption';
    caption.setAttribute('role', 'note');
    const captionFixed = document.createElement('p');
    captionFixed.textContent = 'Nothing is simulated beyond the lattice. Rings are measured reference lengths, not FTD results.';
    const captionNext = document.createElement('p');
    captionNext.className = 'scale-landmarks-next';
    caption.append(captionFixed, captionNext);
    root.append(svg, centre, caption);

    const items = SCALE_LANDMARKS.map((landmark) => {
        const circle = document.createElementNS(SVG_NS, 'circle');
        circle.setAttribute('class', `scale-landmark-ring scale-landmark-ring--${landmark.kind}`);
        circle.setAttribute('visibility', 'hidden');
        svg.append(circle);
        const label = document.createElement('div');
        label.className = `scale-landmark-label scale-landmark-label--${landmark.kind}`;
        label.dataset.landmark = landmark.id;
        label.title = landmark.basis;
        label.textContent = `${landmark.label} · ${metresText(metresToVoxels(landmark.metres))}`;
        label.hidden = true;
        root.append(label);
        return {
            landmark, circle, label, angle: LABEL_ANGLE,
            shown: false, cx: NaN, cy: NaN, r: NaN, labelShown: false, lx: NaN, ly: NaN, flip: null,
        };
    });
    const shown = { hidden: true, centreHidden: null, centreX: NaN, centreY: NaN, centreText: '', next: null };

    const hideItem = (item) => {
        if (item.shown) {
            item.shown = false;
            item.circle.setAttribute('visibility', 'hidden');
        }
        if (item.labelShown) {
            item.labelShown = false;
            item.label.hidden = true;
        }
    };

    return {
        el: root,
        /**
         * @param {null|{x:number,y:number,pixelsPerVoxel:number,viewWidth:number,viewHeight:number,latticeVoxels:number}} frame
         *        null hides the layer (the view is not beyond the lattice).
         */
        render(frame) {
            const hidden = !frame;
            if (shown.hidden !== hidden) {
                shown.hidden = hidden;
                root.hidden = hidden;
            }
            if (!frame) return;
            const { x, y, pixelsPerVoxel, viewWidth, viewHeight } = frame;
            const onScreen = new Map(
                visibleLandmarks(pixelsPerVoxel, viewWidth, viewHeight).map((entry) => [entry.id, entry.diameterPx]),
            );
            let next = null;
            const placed = [];
            for (const item of items) {
                const diameterPx = onScreen.get(item.landmark.id);
                if (diameterPx === undefined) {
                    hideItem(item);
                    // The first landmark too large to draw is the next one out.
                    if (!next && metresToVoxels(item.landmark.metres) * pixelsPerVoxel > Math.max(viewWidth, viewHeight)) {
                        next = item.landmark;
                    }
                    continue;
                }
                const r = diameterPx / 2;
                if (!item.shown) {
                    item.shown = true;
                    item.circle.setAttribute('visibility', 'visible');
                }
                if (x !== item.cx || y !== item.cy || r !== item.r) {
                    item.cx = x;
                    item.cy = y;
                    item.r = r;
                    item.circle.setAttribute('cx', x.toFixed(2));
                    item.circle.setAttribute('cy', y.toFixed(2));
                    item.circle.setAttribute('r', r.toFixed(2));
                }
                // The label rides on the ring; if that point is off screen
                // the ring itself is too, at least on this side.
                const lx = x + r * Math.cos(item.angle);
                const ly = y - r * Math.sin(item.angle);
                const visible = lx > LABEL_PAD_PX && lx < viewWidth - LABEL_PAD_PX
                    && ly > LABEL_PAD_PX && ly < viewHeight - LABEL_PAD_PX;
                if (item.labelShown !== visible) {
                    item.labelShown = visible;
                    item.label.hidden = !visible;
                }
                if (!visible) continue;
                placed.push({ item, lx, ly });
            }
            // Rings of similar size (the W, Z, Higgs and top lie within a
            // factor of 2.2) put their labels close together. Working outward
            // from the smallest ring, each label is moved just clear of the
            // one below it, so the order on screen stays the order of the rings.
            for (let i = 0; i < placed.length; i++) {
                const entry = placed[i];
                if (i > 0) entry.ly = Math.min(entry.ly, placed[i - 1].ly - LABEL_GAP_PX);
                const { item, lx, ly } = entry;
                // Text runs away from the centre; near the right edge it runs left.
                const flip = lx > viewWidth - 260;
                if (lx !== item.lx || ly !== item.ly || flip !== item.flip) {
                    item.lx = lx;
                    item.ly = ly;
                    item.flip = flip;
                    item.label.style.transform = flip
                        ? `translate3d(${lx.toFixed(1)}px, ${ly.toFixed(1)}px, 0) translate(calc(-100% - 8px), -50%)`
                        : `translate3d(${lx.toFixed(1)}px, ${ly.toFixed(1)}px, 0) translate(8px, -50%)`;
                }
            }

            const latticePx = Math.max(1, frame.latticeVoxels || 1) * pixelsPerVoxel;
            const centreHidden = latticePx > LATTICE_DOT_MAX_PX;
            if (shown.centreHidden !== centreHidden) {
                shown.centreHidden = centreHidden;
                centre.hidden = centreHidden;
            }
            if (!centreHidden) {
                if (x !== shown.centreX || y !== shown.centreY) {
                    shown.centreX = x;
                    shown.centreY = y;
                    centre.style.transform = `translate3d(${x.toFixed(1)}px, ${y.toFixed(1)}px, 0)`;
                }
                const text = `lattice · ${metresText(frame.latticeVoxels || 1)}`;
                if (text !== shown.centreText) {
                    shown.centreText = text;
                    centreLabel.textContent = text;
                }
            }
            if (next !== shown.next) {
                shown.next = next;
                captionNext.textContent = next
                    ? `Next reference out: ${next.label} at ${metresText(metresToVoxels(next.metres))}.`
                    : '';
            }
        },
    };
}

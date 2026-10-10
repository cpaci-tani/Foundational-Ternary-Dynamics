/** Small clock faces for the voxels around the one under the view. */

// Each face remembers what it last wrote and writes only a change. The
// element itself cannot be compared against: `style.transform` reads back
// `0px` where `0` was written.
export function createVoxelClocks() {
    const root = document.createElement('div');
    root.className = 'voxel-clocks';
    const pool = [];

    return {
        el: root,
        render(clocks) {
            const list = clocks || [];
            while (pool.length < list.length) {
                const el = document.createElement('div');
                el.className = 'voxel-clock';
                const hand = document.createElement('span');
                hand.className = 'voxel-clock-hand';
                el.append(hand);
                root.append(el);
                pool.push({ el, hand, hidden: false, x: NaN, y: NaN, opacity: NaN, phase: NaN, turn: '' });
            }
            for (let i = 0; i < pool.length; i++) {
                const clock = list[i];
                const item = pool[i];
                const hidden = !clock;
                if (item.hidden !== hidden) {
                    item.hidden = hidden;
                    item.el.hidden = hidden;
                }
                if (!clock) continue;
                if (clock.x !== item.x || clock.y !== item.y) {
                    item.x = clock.x;
                    item.y = clock.y;
                    item.el.style.transform = `translate3d(${clock.x}px, ${clock.y}px, 0) translate(-50%, -50%)`;
                }
                if (clock.opacity !== item.opacity) {
                    item.opacity = clock.opacity;
                    item.el.style.opacity = String(clock.opacity);
                }
                if (!Object.is(clock.phase, item.phase)) {
                    item.phase = clock.phase;
                    const turn = (clock.phase * 360).toFixed(1);
                    if (turn !== item.turn) {
                        item.turn = turn;
                        item.hand.style.transform = `rotate(${turn}deg)`;
                    }
                }
            }
        },
    };
}

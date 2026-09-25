/** Small clock faces for the voxels around the one under the view. */

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
                pool.push({ el, hand });
            }
            for (let i = 0; i < pool.length; i++) {
                const clock = list[i];
                const item = pool[i];
                item.el.hidden = !clock;
                if (!clock) continue;
                item.el.style.left = `${clock.x}px`;
                item.el.style.top = `${clock.y}px`;
                item.el.style.opacity = String(clock.opacity);
                item.hand.style.transform = `rotate(${(clock.phase * 360).toFixed(1)}deg)`;
            }
        },
    };
}

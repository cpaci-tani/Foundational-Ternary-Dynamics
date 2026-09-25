/** Small bracket over the body, shown once that body is smaller than 5 pixels. */

export function createOuterRuler() {
    const el = document.createElement('div');
    el.className = 'live-ruler live-ruler-outer';
    el.hidden = true;
    el.title = 'Length of the space around the drawn body. Appears when that body is smaller than 5 pixels.';
    const bracket = document.createElement('div');
    bracket.className = 'live-ruler-bracket';
    bracket.append(
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-bar' }),
        Object.assign(document.createElement('span'), { className: 'live-ruler-cap' }),
    );
    const label = document.createElement('div');
    label.className = 'live-ruler-label';
    el.append(bracket, label);

    let signature = '';

    return {
        el,
        render(bracketScale, formatLength, active) {
            el.hidden = !active;
            if (!active) {
                signature = '';
                return;
            }
            const next = `${bracketScale.px}|${bracketScale.units}`;
            if (next === signature) return;
            signature = next;
            bracket.style.width = `${bracketScale.px}px`;
            label.textContent = `outer · ${formatLength(bracketScale.units, bracketScale.units)}`;
        },
    };
}

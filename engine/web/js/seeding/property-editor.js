import {node, button} from './component-editor.js';
import {advisoryMessage, displayValue, propertyError, propertyTooltip, sliderBounds, stepChoice} from './property-schema.js';

let sequence = 0;

/** Every scientific input has a numeric editor; continuous inputs also have a slider. */
export function propertyEditor(property, initialValue, change, {reset, modified = false} = {}) {
    let value = initialValue;
    const root = node('div', undefined, 'seed-property');
    root.dataset.propertyKey = property.key;
    root.dataset.modified = String(modified);
    root.dataset.active = String(property.active !== false);
    const title = node('div', undefined, 'seed-property-title');
    const label = node('label', property.label);
    const input = node('input', undefined, 'ctrl-input seed-number');
    input.type = 'number'; input.id = `seed-property-${++sequence}`; label.htmlFor = input.id;
    input.setAttribute('aria-label', property.label);
    input.dataset.uiTooltip = propertyTooltip(property);
    const help = button('?', () => {
        explanation.hidden = !explanation.hidden;
        help.setAttribute('aria-expanded', String(!explanation.hidden));
    });
    help.classList.add('seed-property-help'); help.setAttribute('aria-label', `Help for ${property.label}`);
    help.dataset.uiTooltip = propertyTooltip(property); help.setAttribute('aria-expanded', 'false');
    const explanation = node('p', propertyTooltip(property), 'seed-help-text');
    explanation.id = `${input.id}-help`; explanation.hidden = true;
    help.setAttribute('aria-controls', explanation.id);
    title.append(label, help);
    if (reset) {
        const restore = button('↺', reset); restore.classList.add('seed-property-reset');
        restore.setAttribute('aria-label', `Reset ${property.label} to preset`);
        title.append(restore);
    }
    const controls = node('div', undefined, 'seed-number-row');
    let options = property.options || [];
    let codes = options.map(([key], i) => typeof key === 'number' ? key : i);
    input.step = options.length ? '1' : String(property.type === 'real' ? 'any' : property.step);
    input.min = String(options.length ? Math.min(...codes) : property.min);
    input.max = String(options.length ? Math.max(...codes) : property.max);
    function commit(next) { value = next; root.dataset.modified = 'true'; refresh(); change(next); }
    function step(direction) {
        if (options.length) return commit(stepChoice(property, value, direction));
        if (propertyError(property, value)) return;
        const next = Number((value + direction * property.step).toPrecision(15));
        if (!propertyError(property, next)) commit(next);
    }
    const down = button('−', () => step(-1)), up = button('+', () => step(1));
    down.setAttribute('aria-label', `Decrease ${property.label}`); up.setAttribute('aria-label', `Increase ${property.label}`);
    controls.append(down, input, up);
    const readable = node('output', undefined, 'seed-property-value');
    const message = node('p', undefined, 'seed-property-message');
    message.id = `${input.id}-message`; message.setAttribute('aria-live', 'polite');
    input.setAttribute('aria-describedby', `${message.id} ${input.id}-help`);
    const slider = property.type === 'real' && !options.length ? node('input', undefined, 'seed-slider') : null;
    if (slider) {
        slider.type = 'range'; slider.step = 'any'; slider.dataset.uiTooltip = propertyTooltip(property);
        slider.setAttribute('aria-label', `${property.label} slider`);
        slider.addEventListener('input', () => commit(slider.valueAsNumber));
    }
    input.addEventListener('change', () => {
        const code = input.valueAsNumber;
        const index = codes.indexOf(code);
        commit(options.length ? (index < 0 ? NaN : options[index][0]) : code);
    });
    input.addEventListener('keydown', event => {
        if (options.length && ['ArrowUp', 'ArrowDown'].includes(event.key)) {
            event.preventDefault(); step(event.key === 'ArrowUp' ? 1 : -1);
        }
    });
    root.addEventListener('keydown', event => {
        if (event.key === 'Escape') { explanation.hidden = true; help.setAttribute('aria-expanded', 'false'); }
    });
    function refresh() {
        const optionIndex = options.findIndex(([key]) => key === value);
        input.value = Number.isNaN(value) ? '' : String(options.length ? codes[optionIndex] ?? '' : value);
        readable.textContent = displayValue(property, value);
        input.setAttribute('aria-valuetext', readable.textContent);
        const error = propertyError(property, value);
        input.setAttribute('aria-invalid', String(!!error));
        message.textContent = error || advisoryMessage(property, value);
        message.dataset.error = String(!!error); message.hidden = !message.textContent;
        if (slider) {
            const [min, max] = sliderBounds(property, value);
            slider.min = String(min); slider.max = String(max); slider.value = String(value);
            slider.disabled = min === max;
        }
    }
    root.updateProperty = (next, nextValue, nextModified = modified) => {
        Object.assign(property, next);
        root.dataset.modified = String(nextModified);
        root.dataset.active = String(property.active !== false);
        options = property.options || [];
        codes = options.map(([key], i) => typeof key === 'number' ? key : i);
        input.dataset.uiTooltip = propertyTooltip(property);
        help.dataset.uiTooltip = propertyTooltip(property);
        explanation.textContent = propertyTooltip(property);
        if (root.contains(document.activeElement)) return;
        value = nextValue;
        input.min = String(options.length ? Math.min(...codes) : property.min);
        input.max = String(options.length ? Math.max(...codes) : property.max);
        refresh();
    };
    root.append(title, controls);
    if (slider) root.append(slider);
    root.append(readable, message, explanation); refresh();
    return root;
}

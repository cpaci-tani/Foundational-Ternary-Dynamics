/** Shared presentation rules. Defaults and bounds come from the constructor. */
export function displayValue(property, value) {
    const choice = property.options?.find(([key]) => key === value);
    if (choice) return `${typeof value === 'number' ? value : property.options.indexOf(choice)} — ${choice[1]}`;
    if (typeof value === 'boolean') return value ? '1 — On' : '0 — Off';
    return `${value}${property.units ? ` ${property.units}` : ''}`;
}

export function propertyError(property, value) {
    if (property.options?.length) return property.options.some(([key]) => key === value) ? '' : 'Choose one of the admitted states.';
    if (typeof value !== 'number' || !Number.isFinite(value)) return 'Enter a finite number.';
    if (property.type !== 'real' && !Number.isSafeInteger(value)) return 'Enter an exact integer.';
    if (value < property.min || value > property.max) return `Legal range: ${property.min} to ${property.max}${property.units ? ` ${property.units}` : ''}.`;
    return '';
}

export function advisoryMessage(property, value) {
    if (propertyError(property, value) || property.options?.length) return '';
    const [lo, hi] = property.recommended;
    return value < lo || value > hi ? `Outside the recommended starting range (${lo}–${hi}). This legal value will be retained.` : '';
}

export function sliderBounds(property, value) {
    const [lo, hi] = property.recommended;
    const valid = typeof value === 'number' && Number.isFinite(value) && !propertyError(property, value);
    return [Math.max(property.min, Math.min(lo, valid ? value : lo)),
        Math.min(property.max, Math.max(hi, valid ? value : hi))];
}

export function stepChoice(property, value, direction) {
    const options = property.options || [];
    const index = options.findIndex(([key]) => key === value);
    if (index < 0) return value;
    return options[Math.max(0, Math.min(options.length - 1, index + direction))][0];
}

export function propertyTooltip(property) {
    const recommendation = property.options?.length
        ? property.options.map(([value]) => displayValue(property, value)).join('; ')
        : `${property.recommended[0]} to ${property.recommended[1]}${property.units ? ` ${property.units}` : ''}`;
    return `${property.description}\nPreset default: ${displayValue(property, property.default)}.\n`
        + `Recommended: ${recommendation}. ${property.recommendationBasis}\n`
        + `Legal bounds: ${property.min} to ${property.max}.`
        + (property.defaultExpression ? `\nDefault expression: ${property.defaultExpression}.` : '')
        + (property.dependencies?.length ? `\nDepends on: ${property.dependencies.join(', ')}. Reset restores the constructor default.` : '');
}

export function assertPropertyDescriptor(property) {
    for (const key of ['key', 'label', 'description', 'group', 'type', 'binding', 'recommendationBasis']) {
        if (typeof property[key] !== 'string' || !property[key].trim()) throw new Error(`Missing seed property ${key}: ${property.key}`);
    }
    if (!['real', 'integer', 'choice'].includes(property.type)
        || !Number.isFinite(property.min) || !Number.isFinite(property.max) || property.min > property.max
        || !Number.isFinite(property.step) || property.step <= 0
        || !Array.isArray(property.recommended) || property.recommended.length !== 2
        || property.recommended.some(n => !Number.isFinite(n))
        || property.recommended[0] > property.recommended[1]
        || property.recommended[0] < property.min || property.recommended[1] > property.max
        || propertyError(property, property.default)) throw new Error(`Invalid seed descriptor: ${property.key}`);
    return property;
}

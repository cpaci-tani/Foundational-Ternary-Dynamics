/**
 * Verification Lab — central experiment registry.
 * Aggregates all category files into one flat list for the UI.
 */

import { QUANTUM_EXPERIMENTS } from './categories/quantum.js';
import { CONSERVATION_EXPERIMENTS } from './categories/conservation.js';

export const CATEGORIES = [
    { id: 'quantum',      label: 'Quantum' },
    { id: 'conservation', label: 'Conservation' },
    // Future:
    // { id: 'em',        label: 'EM' },
    // { id: 'strong',    label: 'Strong' },
    // { id: 'gravity',   label: 'Gravity' },
    // { id: 'emergence', label: 'Emergence' },
];

export const EXPERIMENTS = [
    ...QUANTUM_EXPERIMENTS,
    ...CONSERVATION_EXPERIMENTS,
];

export function experimentsByCategory(categoryId) {
    return EXPERIMENTS.filter((e) => e.category === categoryId);
}

export function getExperiment(id) {
    return EXPERIMENTS.find((e) => e.id === id) || null;
}

/**
 * Render one of the three Verify tiers: hard predictions, parametric
 * insertions, or unpredicted measurements. Each tier has a distinct
 * heading and description that primes the reader for what *kind* of
 * evidence they're about to see — this is the honesty layer.
 */
import { renderRow } from './row.js';

const TIER_META = {
    hard: {
        label: 'Hard predictions',
        blurb: 'Theorem- or selection-level derivations. Each row lists the inputs that went into the FTD value so the reader can count the knobs.',
    },
    parametric: {
        label: 'Parametric insertions',
        blurb: 'Standard-Model formulas with FTD numbers plugged in. Agreement here is bookkeeping, not a prediction about nature.',
    },
    unpredicted: {
        label: 'Measurements with no FTD claim',
        blurb: 'Published measurements FTD does not currently address. Listed explicitly so gaps are visible.',
    },
};

export function renderTier(tierId, rows) {
    const meta = TIER_META[tierId] || { label: tierId, blurb: '' };
    const body = rows.length
        ? rows.map(renderRow).join('\n')
        : `<p class="verify-tier-empty">No rows in this tier yet.</p>`;
    return `
        <section class="verify-tier verify-tier--${tierId}" data-tier="${tierId}">
            <header class="verify-tier-header">
                <h2 class="verify-tier-label">${meta.label}</h2>
                <p class="verify-tier-blurb">${meta.blurb}</p>
                <span class="verify-tier-count">${rows.length} row${rows.length === 1 ? '' : 's'}</span>
            </header>
            <div class="verify-tier-body">
                ${body}
            </div>
        </section>
    `;
}

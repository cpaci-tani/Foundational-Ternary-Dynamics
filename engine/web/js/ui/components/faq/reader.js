/**
 * FAQ entry reader — 4 fixed sections with epistemic tag chips on
 * ftdAngle bullets. Plugs into SidebarLibraryComponent via renderReader.
 */

import { renderMathInHtml } from '../../math-format/render.js';

function escapeHtml(s) {
    return String(s ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#39;');
}

function renderParagraphs(paras) {
    return (paras || []).map((p) => `<p>${renderMathInHtml(escapeHtml(p))}</p>`).join('');
}

function renderAngleBullets(bullets) {
    return (bullets || []).map((b) => `
        <li class="faq-angle-bullet">
            <span class="faq-tag faq-tag--${escapeHtml(b.tag.toLowerCase())}">${escapeHtml(b.tag)}</span>
            <span class="faq-angle-text">${renderMathInHtml(escapeHtml(b.text))}</span>
        </li>
    `).join('');
}

function renderOpenBullets(bullets) {
    return (bullets || []).map((b) => `
        <li class="faq-open-bullet">
            <span class="faq-tag faq-tag--open">OPEN</span>
            <span class="faq-open-text">${renderMathInHtml(escapeHtml(b))}</span>
        </li>
    `).join('');
}

function renderTheoryRefs(refs) {
    if (!refs || !refs.length) return '';
    const items = refs.map((r) => `<li><code>${escapeHtml(r)}</code></li>`).join('');
    return `
        <div class="faq-theory-refs">
            <span class="faq-section-label">Theory references</span>
            <ul class="faq-theory-list">${items}</ul>
        </div>
    `;
}

export function renderFaqReader(entry) {
    if (!entry) {
        return `
            <div class="faq-empty-reader">
                <h3>No question selected</h3>
                <p>Pick a question from the list.</p>
            </div>
        `;
    }
    return `
        <h3 class="faq-question">${renderMathInHtml(escapeHtml(entry.question))}</h3>
        <section class="faq-reader-section faq-reader-section--problem">
            <h4 class="faq-section-label">The problem</h4>
            ${renderParagraphs(entry.problem)}
        </section>
        <section class="faq-reader-section faq-reader-section--mainstream">
            <h4 class="faq-section-label">Why mainstream physics struggles</h4>
            ${renderParagraphs(entry.mainstreamStruggle)}
        </section>
        <section class="faq-reader-section faq-reader-section--angle">
            <h4 class="faq-section-label">FTD's angle</h4>
            <ul class="faq-angle-list">${renderAngleBullets(entry.ftdAngle)}</ul>
        </section>
        <section class="faq-reader-section faq-reader-section--open">
            <h4 class="faq-section-label">What's still open</h4>
            <ul class="faq-open-list">${renderOpenBullets(entry.stillOpen)}</ul>
        </section>
        ${renderTheoryRefs(entry.theoryRefs)}
    `;
}

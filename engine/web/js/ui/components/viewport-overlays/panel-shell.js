/**
 * Shared viewport overlay panel shell — scales 1–5 (Scale 0 uses s0-overlay-panel).
 */

/**
 * @param {string} label
 * @param {string} [hint]
 * @param {string} contentHtml
 */
export function overlaySection(label, hint, contentHtml) {
  const hintHtml = hint
    ? `<span class="scale-overlay-section-hint">${hint}</span>`
    : '';
  return `
    <div class="scale-overlay-section">
      <span class="scale-overlay-section-label">${label}</span>
      ${hintHtml}
      ${contentHtml}
    </div>`;
}

/**
 * @param {string} [className]
 * @param {string} innerHtml
 */
export function overlayRow(className, innerHtml) {
  const cls = className ? ` scale-overlay-row-${className}` : '';
  return `<div class="scale-overlay-row${cls}">${innerHtml}</div>`;
}

/**
 * @param {object} opts
 * @param {string} opts.id
 * @param {string} opts.scaleClass - visibility class, e.g. scale1-only or scale-ae
 * @param {string} opts.title
 * @param {string} [opts.footnote]
 * @param {string} opts.bodyHtml
 * @param {string} [opts.legendHtml]
 */
export function createScaleOverlayPanel(opts) {
  const {
    id,
    scaleClass,
    title,
    footnote = '',
    bodyHtml,
    legendHtml = '',
  } = opts;

  const container = document.createElement('div');
  container.id = id;
  container.className = `${scaleClass} viewport-overlay-panel scale-overlay-panel`;

  const footnoteBlock = footnote
    ? `<p class="scale-overlay-footnote">${footnote}</p>`
    : '';

  container.innerHTML = `
    <header class="scale-overlay-header">
      <span class="scale-overlay-title">${title}</span>
      <div class="scale-overlay-collapse-slot"></div>
    </header>
    <div class="scale-overlay-body">
      ${footnoteBlock}
      ${bodyHtml}
    </div>
    ${legendHtml}
  `;
  return container;
}

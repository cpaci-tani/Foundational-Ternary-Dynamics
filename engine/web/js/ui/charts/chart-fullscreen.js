/**
 * Shared fullscreen portal for chart cards.
 * Any .chart-card element can be portaled into the fullscreen overlay
 * by calling attachFullscreen(cardEl).  The overlay is a singleton
 * created lazily on first use.
 */

function getOrCreateOverlay() {
    let overlay = document.getElementById('chart-fullscreen-overlay');
    if (overlay) return overlay;

    overlay = document.createElement('div');
    overlay.id = 'chart-fullscreen-overlay';
    overlay.innerHTML = `
        <div class="chart-fs-backdrop"></div>
        <div class="chart-fs-content"></div>
    `;
    document.body.appendChild(overlay);

    overlay.querySelector('.chart-fs-backdrop').addEventListener('click', () => {
        const card = overlay.querySelector('.chart-card');
        if (card?._ftdCard) card._ftdCard._exitFullscreen();
    });

    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && overlay.classList.contains('is-open')) {
            const card = overlay.querySelector('.chart-card');
            if (card?._ftdCard) card._ftdCard._exitFullscreen();
        }
    });

    return overlay;
}

/**
 * Attach fullscreen portal behaviour to a .chart-card element.
 * Looks for a .chart-card-expand button inside cardEl and binds click.
 * Sets cardEl._ftdCard = { _enterFullscreen, _exitFullscreen, _isFullscreen }.
 */
export function attachFullscreen(cardEl) {
    const state = {
        _isFullscreen:   false,
        _originalParent: null,
        _originalNext:   null,

        _enterFullscreen() {
            const overlay = getOrCreateOverlay();
            this._originalParent = cardEl.parentNode;
            this._originalNext   = cardEl.nextSibling;
            overlay.querySelector('.chart-fs-content').appendChild(cardEl);
            cardEl.classList.add('is-fs');
            overlay.classList.add('is-open');
            this._isFullscreen = true;
        },

        _exitFullscreen() {
            const overlay = document.getElementById('chart-fullscreen-overlay');
            if (!overlay) return;
            if (this._originalParent) {
                this._originalParent.insertBefore(cardEl, this._originalNext || null);
            }
            cardEl.classList.remove('is-fs');
            overlay.classList.remove('is-open');
            this._isFullscreen = false;
            // Force immediate resize after DOM move — ResizeObserver fires
            // asynchronously and the oversized canvas overflows until it does.
            requestAnimationFrame(() => {
                cardEl.querySelectorAll('.chart-card-plot').forEach((plot) => {
                    if (typeof plot._ftdResize === 'function') plot._ftdResize();
                });
            });
        },
    };

    cardEl._ftdCard = state;

    const btn = cardEl.querySelector('.chart-card-expand');
    if (btn) {
        btn.addEventListener('click', (e) => {
            e.stopPropagation();
            state._isFullscreen ? state._exitFullscreen() : state._enterFullscreen();
        });
    }
}

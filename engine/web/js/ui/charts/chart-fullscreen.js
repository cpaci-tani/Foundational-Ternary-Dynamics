/**
 * Shared fullscreen portal for chart cards.
 * Any .chart-card element can be portaled into the fullscreen overlay
 * by calling attachFullscreen(cardEl).  The overlay is a singleton
 * created lazily on first use.
 *
 * Active-card stack (audit pass 2 FS-1/FS-2): the overlay only ever
 * displays one card at a time, but `_enterFullscreen` may be called
 * on card B while card A is still fullscreen (e.g. user clicks expand
 * on a second card via keyboard shortcut). We serialize by tracking
 * the active card and calling its `_exitFullscreen` before swapping.
 */

let _activeCard = null;   // module-private: the currently-fullscreen .chart-card

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
        if (_activeCard?._ftdCard) _activeCard._ftdCard._exitFullscreen();
    });

    document.addEventListener('keydown', (e) => {
        if (e.key === 'Escape' && overlay.classList.contains('is-open')) {
            if (_activeCard?._ftdCard) _activeCard._ftdCard._exitFullscreen();
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
            // FS-2: if another card is already fullscreen, exit it first
            // so the overlay never holds two .chart-card children.
            if (_activeCard && _activeCard !== cardEl && _activeCard._ftdCard?._isFullscreen) {
                _activeCard._ftdCard._exitFullscreen();
            }
            const overlay = getOrCreateOverlay();
            this._originalParent = cardEl.parentNode;
            this._originalNext   = cardEl.nextSibling;
            overlay.querySelector('.chart-fs-content').appendChild(cardEl);
            cardEl.classList.add('is-fs');
            overlay.classList.add('is-open');
            this._isFullscreen = true;
            _activeCard = cardEl;
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
            if (_activeCard === cardEl) _activeCard = null;
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

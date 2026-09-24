import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { initSettingsModal } from '../ui/components/settings-modal/component.js?v=2';
/** Bind once against live state; dispose before rebinding. */
export function wireSettings({ getViewport = () => null } = {}) {
    const scope = new LifetimeScope();

    initSettingsModal();
    const root = document.documentElement;
    const modal = document.getElementById('settings-modal');
    const btnOpen = document.getElementById('btn-settings');
    const btnClose = document.getElementById('settings-close');
    const slider = document.getElementById('settings-ui-scale');
    const valDisplay = document.getElementById('settings-scale-val');
    const glassToggle = document.getElementById('settings-glass-enabled');
    const glassThicknessSlider = document.getElementById('settings-glass-thickness');
    const glassThicknessValue = document.getElementById('settings-glass-thickness-val');
    const glassControls = document.getElementById('settings-glass-controls');
    const btnReset = document.getElementById('settings-reset');
    const settingsButtons = Array.from(document.querySelectorAll('[data-setting][data-value]'));

    const DEFAULT_SETTINGS = Object.freeze({
        scale: 1.0,
        theme: 'default',
        glass: 'off',
        glassThickness: 16,
        density: 'comfortable',
        panelWidth: 'standard',
        tooltips: 'on',
        statusBar: 'shown',
    });

    const STORAGE_KEYS = Object.freeze({
        scale: 'ftd-ui-scale',
        theme: 'ftd-theme',
        glass: 'ftd-glassmorphism',
        glassThickness: 'ftd-glass-thickness',
        density: 'ftd-density',
        panelWidth: 'ftd-panel-width',
        tooltips: 'ftd-tooltips',
        statusBar: 'ftd-status-bar',
    });

    function setChoiceGroup(settingName, value) {
        settingsButtons.forEach((button) => {
            if (button.dataset.setting !== settingName) return;
            const selected = button.dataset.value === value;
            button.classList.toggle('active', selected);
            button.setAttribute('aria-pressed', String(selected));
        });
    }

    function persist(key, value) {
        try { localStorage.setItem(key, String(value)); } catch (e) { }
    }

    const GLASS_THICKNESS_MIN = 4;
    const GLASS_THICKNESS_MAX = 32;
    let glassThicknessFrame = 0;
    let pendingGlassThickness = null;

    function normalizeGlassThickness(value) {
        const parsed = Number(value);
        if (!Number.isFinite(parsed)) return DEFAULT_SETTINGS.glassThickness;
        return Math.min(GLASS_THICKNESS_MAX, Math.max(GLASS_THICKNESS_MIN, Math.round(parsed)));
    }

    function updateGlassThicknessDisplay(thickness) {
        if (glassThicknessSlider) glassThicknessSlider.value = String(thickness);
        if (glassThicknessValue) glassThicknessValue.textContent = `${thickness} px`;
    }

    function applyGlassThickness(value) {
        if (glassThicknessFrame) cancelAnimationFrame(glassThicknessFrame);
        glassThicknessFrame = 0;
        pendingGlassThickness = null;
        const thickness = normalizeGlassThickness(value);
        root.style.setProperty('--glass-thickness', `${thickness}px`);
        root.style.setProperty('--glass-blur-low', `${thickness / 2}px`);
        root.style.setProperty('--glass-blur-mid', `${thickness}px`);
        root.style.setProperty('--glass-blur-high', `${thickness * 1.5}px`);
        updateGlassThicknessDisplay(thickness);
        persist(STORAGE_KEYS.glassThickness, thickness);
    }

    function queueGlassThickness(value) {
        pendingGlassThickness = normalizeGlassThickness(value);
        updateGlassThicknessDisplay(pendingGlassThickness);
        if (glassThicknessFrame) return;
        glassThicknessFrame = requestAnimationFrame(() => {
            const thickness = pendingGlassThickness;
            glassThicknessFrame = 0;
            pendingGlassThickness = null;
            applyGlassThickness(thickness);
        });
    }

    function applyGlassMode(value) {
        const mode = value === 'on' ? 'on' : 'off';
        const enabled = mode === 'on';
        root.dataset.glass = mode;
        if (glassToggle) {
            glassToggle.checked = enabled;
            glassToggle.setAttribute('aria-checked', enabled ? 'true' : 'false');
        }
        if (glassThicknessSlider) glassThicknessSlider.disabled = !enabled;
        if (glassControls) {
            glassControls.classList.toggle('is-disabled', !enabled);
            glassControls.setAttribute('aria-disabled', enabled ? 'false' : 'true');
        }
        persist(STORAGE_KEYS.glass, mode);
    }

    // ── Scale ──
    function applyScale(s) {
        s = Number.isFinite(s) ? Math.min(1.6, Math.max(0.8, s)) : DEFAULT_SETTINGS.scale;
        // Write the USER knob (--ui-scale-base). The effective --ui-scale is
        // derived in tokens.css (= base) and may be multiplied per-breakpoint
        // in responsive.css (mobile = base × 1.2) without losing this setting.
        root.style.setProperty('--ui-scale-base', s);
        if (slider) slider.value = s;
        if (valDisplay) valDisplay.textContent = Math.round(s * 100) + '%';
        document.querySelectorAll('.settings-preset').forEach(b => {
            b.classList.toggle('active', Math.abs(parseFloat(b.dataset.scale) - s) < 0.01);
        });
        if (root.dataset.statusBar === 'hidden') {
            root.style.setProperty('--status-bar-offset', '0px');
        } else {
            root.style.setProperty('--status-bar-offset', 'calc(28px * var(--ui-scale))');
        }
        persist(STORAGE_KEYS.scale, s);
        scope.timeout(() => getViewport()?.resize?.(), 100);
    }

    // ── Theme ──
    let themeReleaseRaf = 0;
    function applyTheme(name) {
        if (!['default', 'abyss', 'light', 'nord', 'parchment'].includes(name)) name = DEFAULT_SETTINGS.theme;
        root.dataset.themeChanging = 'true';
        if (themeReleaseRaf) cancelAnimationFrame(themeReleaseRaf);
        if (name === 'default') {
            root.removeAttribute('data-theme');
        } else {
            root.setAttribute('data-theme', name);
        }
        document.querySelectorAll('.theme-swatch').forEach(sw => {
            const on = sw.dataset.theme === name;
            sw.classList.toggle('active', on);
            sw.setAttribute('aria-checked', on ? 'true' : 'false');
            sw.tabIndex = on ? 0 : -1;
        });
        persist(STORAGE_KEYS.theme, name);
        themeReleaseRaf = requestAnimationFrame(() => {
            themeReleaseRaf = requestAnimationFrame(() => {
                delete root.dataset.themeChanging;
                themeReleaseRaf = 0;
            });
        });
    }



    function applyDensity(mode) {
        root.dataset.density = mode;
        setChoiceGroup('density', mode);
        persist(STORAGE_KEYS.density, mode);
    }

    function applyPanelWidth(mode) {
        root.dataset.panelWidth = mode;
        setChoiceGroup('panel-width', mode);
        scope.timeout(() => getViewport()?.resize?.(), 80);
        persist(STORAGE_KEYS.panelWidth, mode);
    }

    function applyTooltipMode(mode) {
        root.dataset.tooltips = mode;
        if (mode === 'off') document.getElementById('ui-tooltip')?.setAttribute('hidden', '');
        setChoiceGroup('tooltips', mode);
        persist(STORAGE_KEYS.tooltips, mode);
    }

    function applyStatusBar(mode) {
        root.dataset.statusBar = mode;
        root.style.setProperty(
            '--status-bar-offset',
            mode === 'hidden' ? '0px' : 'calc(28px * var(--ui-scale))',
        );
        scope.timeout(() => getViewport()?.resize?.(), 80);
        setChoiceGroup('status-bar', mode);
        persist(STORAGE_KEYS.statusBar, mode);
    }

    // ── Load saved settings ──
    try {
        const savedScale = localStorage.getItem(STORAGE_KEYS.scale);
        applyScale(savedScale ? parseFloat(savedScale) : DEFAULT_SETTINGS.scale);
        applyTheme(localStorage.getItem(STORAGE_KEYS.theme) || DEFAULT_SETTINGS.theme);
        const savedGlassThickness = localStorage.getItem(STORAGE_KEYS.glassThickness);
        applyGlassThickness(savedGlassThickness ?? DEFAULT_SETTINGS.glassThickness);
        applyGlassMode(localStorage.getItem(STORAGE_KEYS.glass) || DEFAULT_SETTINGS.glass);
        applyDensity(localStorage.getItem(STORAGE_KEYS.density) || DEFAULT_SETTINGS.density);
        applyPanelWidth(localStorage.getItem(STORAGE_KEYS.panelWidth) || DEFAULT_SETTINGS.panelWidth);
        applyTooltipMode(localStorage.getItem(STORAGE_KEYS.tooltips) || DEFAULT_SETTINGS.tooltips);
        applyStatusBar(localStorage.getItem(STORAGE_KEYS.statusBar) || DEFAULT_SETTINGS.statusBar);
    } catch (e) { }

    // ── Modal open/close ──
    let returnFocus = null;
    const close = () => {
        modal?.classList.remove('visible');
        returnFocus?.focus?.({ preventScroll: true });
        returnFocus = null;
    };
    if (btnOpen && modal) scope.on(btnOpen, 'click', () => {
        returnFocus = document.activeElement;
        modal.classList.add('visible');
        btnClose?.focus({ preventScroll: true });
    });
    if (btnClose && modal) scope.on(btnClose, 'click', close);
    if (modal) scope.on(modal, 'click', (e) => { if (e.target === modal) close(); });
    scope.on(document, 'keydown', (e) => {
        if (!modal?.classList.contains('visible')) return;
        if (e.key === 'Escape') { e.preventDefault(); close(); }
        if (e.key !== 'Tab') return;
        const targets = [...modal.querySelectorAll('button, input, select, textarea, a[href], [tabindex]')]
            .filter(el => !el.disabled && el.tabIndex >= 0 && el.checkVisibility());
        if (!targets.length) return;
        const first = targets[0], last = targets[targets.length - 1];
        if (e.shiftKey && (document.activeElement === first || !modal.contains(document.activeElement))) {
            e.preventDefault(); last.focus();
        } else if (!e.shiftKey && (document.activeElement === last || !modal.contains(document.activeElement))) {
            e.preventDefault(); first.focus();
        }
    });
    scope.defer(() => { modal?.classList.remove('visible'); returnFocus = null; });

    // ── Scale controls ──
    if (slider) scope.on(slider, 'input', () => applyScale(parseFloat(slider.value)));
    document.querySelectorAll('.settings-preset').forEach(btn => {
        scope.on(btn, 'click', () => applyScale(parseFloat(btn.dataset.scale)));
    });

    // ── Theme controls ──
    document.querySelectorAll('.theme-swatch').forEach(sw => {
        scope.on(sw, 'click', () => applyTheme(sw.dataset.theme));
        scope.on(sw, 'keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                applyTheme(sw.dataset.theme);
                return;
            }
            if (e.key !== 'ArrowRight' && e.key !== 'ArrowLeft'
                && e.key !== 'ArrowDown' && e.key !== 'ArrowUp') return;
            e.preventDefault();
            const all = [...document.querySelectorAll('.theme-swatch')];
            const i = all.indexOf(sw);
            if (i < 0) return;
            const dir = (e.key === 'ArrowRight' || e.key === 'ArrowDown') ? 1 : -1;
            const next = all[(i + dir + all.length) % all.length];
            applyTheme(next.dataset.theme);
            next.focus();
        });
    });

    // ── Glass controls ──
    if (glassToggle) {
        scope.on(glassToggle, 'change', () => applyGlassMode(glassToggle.checked ? 'on' : 'off'));
    }
    if (glassThicknessSlider) {
        scope.on(glassThicknessSlider, 'input', () => queueGlassThickness(glassThicknessSlider.value));
        // Pointer release / keyboard commit flushes synchronously so an
        // immediate reload cannot strand the final value in a queued frame.
        scope.on(glassThicknessSlider, 'change', () => applyGlassThickness(glassThicknessSlider.value));
    }

    // ── Other preference controls ──
    settingsButtons.forEach((button) => {
        scope.on(button, 'click', () => {
            const setting = button.dataset.setting;
            const value = button.dataset.value;
            if (!setting || !value) return;
            if (setting === 'density') applyDensity(value);
            else if (setting === 'panel-width') applyPanelWidth(value);
            else if (setting === 'tooltips') applyTooltipMode(value);
            else if (setting === 'status-bar') applyStatusBar(value);
        });
    });

    // ── Reset all ──
    if (btnReset) {
        scope.on(btnReset, 'click', () => {
            applyScale(DEFAULT_SETTINGS.scale);
            applyTheme(DEFAULT_SETTINGS.theme);
            applyGlassThickness(DEFAULT_SETTINGS.glassThickness);
            applyGlassMode(DEFAULT_SETTINGS.glass);
            applyDensity(DEFAULT_SETTINGS.density);
            applyPanelWidth(DEFAULT_SETTINGS.panelWidth);
            applyTooltipMode(DEFAULT_SETTINGS.tooltips);
            applyStatusBar(DEFAULT_SETTINGS.statusBar);
        });
    }

    scope.defer(() => { cancelAnimationFrame(glassThicknessFrame); cancelAnimationFrame(themeReleaseRaf); delete root.dataset.themeChanging; });
    return () => scope.dispose();
}

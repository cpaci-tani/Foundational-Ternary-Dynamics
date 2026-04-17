import { BreakpointService } from './breakpoint-service.js';
import { ensureShellTemplate } from './shell-template.js';
import { PanelDockController } from './panel-dock-controller.js';
import { MobilePanelController } from './mobile-panel.js';
import { LoadingOverlayComponent } from '../components/loading-overlay/component.js';
import { TopbarComponent } from '../components/topbar/component.js';
import { WorkspaceTabsComponent } from '../components/workspace-tabs/component.js';
import { PanelDockComponent } from '../components/panel-dock/component.js?v=2';
import { ViewportFrameComponent } from '../components/viewport-frame/component.js';
import { ViewportOverlaysComponent } from '../components/viewport-overlays/component.js?v=5';
import { TooltipComponent } from '../components/tooltips/component.js';
import { KnowledgeBaseComponent } from '../components/knowledge-base/component.js';
import { ensurePanelResources } from '../components/panel-resources/component.js';
import { annotatePanelElements, getPanelLabel, getPanelRegistry, validatePanelRegistry } from '../scale-registry/panel-registry.js';
import { createScaleUiRegistry } from '../scale-registry/register-scale-ui.js';
import { registerLegacyToolbarUi } from './register-legacy-toolbar-ui.js';
import { registerScale0ToolbarUI } from '../../scales/scale0/ui/register-scale0-ui.js';
import { registerScale1ToolbarUI } from '../../scales/scale1/ui/register-scale1-ui.js';
import { registerScale2ToolbarUI } from '../../scales/scale2/ui/register-scale2-ui.js';
import { registerScale3ToolbarUI } from '../../scales/scale3/ui/register-scale3-ui.js';
import { registerScale4ToolbarUI } from '../../scales/scale4/ui/register-scale4-ui.js';
import { registerScale5ToolbarUI } from '../../scales/scale5/ui/register-scale5-ui.js';
import { registerScale11ToolbarUI } from '../../scales/scale11/ui/register-scale11-ui.js';
import { registerScale23ToolbarUI } from '../../scales/scale23/ui/register-scale23-ui.js';
import { registerScale12ToolbarUI } from '../../scales/scale12/ui/register-scale12-ui.js';

/**
 * Shell facade around the current dashboard DOM.
 *
 * Phase 0 responsibilities:
 * - annotate the existing DOM as shell regions
 * - create future mount roots
 * - own responsive layout state
 * - own panel dock behavior
 */
export class AppShell {
    constructor({ app, onViewportResize = null } = {}) {
        this.app = app || document.getElementById('app');
        this.onViewportResize = typeof onViewportResize === 'function' ? onViewportResize : null;
        this.registry = null;
        this.breakpoints = null;
        this.panelDock = null;
        this.mobilePanel = null;
        this.loadingOverlay = null;
        this.topbar = null;
        this.workspaceTabs = null;
        this.panelDockView = null;
        this.viewportFrame = null;
        this.scaleUiRegistry = null;
        this.tooltips = null;
        this.knowledgeBase = null;
    }

    init() {
        this.registry = ensureShellTemplate(this.app);
        this.scaleUiRegistry = createScaleUiRegistry();
        registerScale0ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale1ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale2ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale3ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale23ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale4ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale5ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale11ToolbarUI(this.scaleUiRegistry.toolbar);
        registerScale12ToolbarUI(this.scaleUiRegistry.toolbar);
        registerLegacyToolbarUi(this.scaleUiRegistry.toolbar);
        ensurePanelResources(this.getRegion('panels'));
        annotatePanelElements(this.getRegion('panels'), this.scaleUiRegistry.panels);
        const validation = validatePanelRegistry(this.getRegion('panels'), this.scaleUiRegistry.panels);
        if (!validation.ok) console.warn('[ui-shell] Panel registry validation errors:', validation.errors);
        this.loadingOverlay = new LoadingOverlayComponent(document.getElementById('loading-overlay')).init();
        this.viewportFrame = new ViewportFrameComponent(this.getRegion('viewport')).init();
        // Topbar must init before viewport overlays so the toolbar is measured
        // before overlays are inserted (see ViewportOverlaysComponent for details).
        this.topbar = new TopbarComponent({
            app: this.app,
            toolbar: this.getRegion('toolbar'),
            toolbarRegistry: this.scaleUiRegistry.toolbar,
        }).init();
        this.knowledgeBase = new KnowledgeBaseComponent({ app: this.app }).init();
        const viewportOverlays = new ViewportOverlaysComponent(this.getRegion('viewport')).init();
        // Wire resize: when the toolbar height changes, re-insert overlays so Chrome's
        // compositor layers are recreated at the new position.
        this.topbar.onResize = (h) => viewportOverlays.updateTopOffset(h);
        this.workspaceTabs = new WorkspaceTabsComponent(this.getRegion('tabs'), this.scaleUiRegistry.panels).init();
        this.panelDockView = new PanelDockComponent(this.getRegion('panels')).init();
        this.tooltips = new TooltipComponent({ app: this.app }).init();
        this.breakpoints = new BreakpointService({
            onChange: (snapshot) => this._applySnapshot(snapshot),
        });
        const snapshot = this.breakpoints.start();
        if (snapshot) this._applySnapshot(snapshot);
        this.app.dataset.shellReady = 'true';
        return this;
    }

    bindPanelDock({ activeTab = 'controls', onTabActivated = null } = {}) {
        if (!this.panelDock) {
            this.panelDock = new PanelDockController({
                app: this.app,
                tabBar: this.getRegion('tabs'),
                panelArea: this.getRegion('panels'),
                toggleButton: document.getElementById('btn-panel-toggle'),
                resizeHandle: document.getElementById('panel-resizer'),
                compactSelect: document.getElementById('tab-select-mobile'),
                onTabActivated,
                onViewportResize: this.onViewportResize,
            });
            this.panelDock.bind({ initialActiveTab: activeTab });
            const current = this.breakpoints?.getSnapshot();
            if (current) this.panelDock.setCompactMode(current.isCompact);
            this.panelDock.applyScaleFilter(this.app?.dataset.activeScale || '0', activeTab);
            this.setActivePanelTitle(
                this.getRegion('tabs')?.querySelector(`.tab[data-panel="${activeTab}"]`)?.textContent?.trim() || 'Controls'
            );
            document.getElementById('btn-panel-hide-mobile')?.addEventListener('click', () => {
                this.panelDock?.setCollapsed(true);
            });

            // Mobile swipe-to-dismiss + body scroll lock
            this.mobilePanel = new MobilePanelController({
                app: this.app,
                panelArea: this.getRegion('panels'),
                resizer: document.getElementById('panel-resizer'),
                dockController: this.panelDock,
            }).init();

            // Re-sync scroll lock on viewport resize (mobile ↔ desktop transitions)
            window.addEventListener('resize', () => this.mobilePanel?._syncScrollLock(), { passive: true });
        }
        return this.panelDock;
    }

    getRegion(name) {
        return this.registry?.getRegion(name) || null;
    }

    getMount(name) {
        return this.registry?.getMount(name) || null;
    }

    setActiveScale(scaleIndex) {
        this.app?.setAttribute('data-active-scale', String(scaleIndex));
        return this.panelDock?.applyScaleFilter(String(scaleIndex), 'controls') || 'controls';
    }

    activatePanel(panelName, options) {
        this.panelDock?.activate(panelName, options);
    }

    setActivePanelTitle(label) {
        this.panelDockView?.setActiveTitle(label);
    }

    getPanelLabel(panelId) {
        return getPanelLabel(panelId);
    }

    listPanels() {
        return getPanelRegistry();
    }

    _applySnapshot(snapshot) {
        if (!this.app) return;
        this.app.dataset.layoutMode = snapshot.layoutMode;
        this.app.dataset.orientation = snapshot.orientation;
        this.app.dataset.compact = snapshot.isCompact ? 'true' : 'false';
        this.app.dataset.tablet = snapshot.isTablet ? 'true' : 'false';
        document.documentElement.dataset.layoutMode = snapshot.layoutMode;
        document.documentElement.dataset.orientation = snapshot.orientation;
        this.panelDock?.setCompactMode(snapshot.isCompact);
    }
}

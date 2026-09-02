import {
    PARTICLE_LOG_CATEGORIES,
    scale1ParticleLedger,
} from '../../telemetry/particle-ledger.js?v=2';

const MAX_RENDERED_EVENTS = 240;
const MAX_RENDERED_HIERARCHY_NODES = 360;

function escapeHtml(value) {
    return String(value ?? '')
        .replaceAll('&', '&amp;')
        .replaceAll('<', '&lt;')
        .replaceAll('>', '&gt;')
        .replaceAll('"', '&quot;')
        .replaceAll("'", '&#039;');
}

function fmtNumber(value, digits = 3) {
    if (!Number.isFinite(value)) return '--';
    const abs = Math.abs(value);
    if ((abs > 0 && abs < 1e-3) || abs >= 1e4) return value.toExponential(2);
    return value.toFixed(digits).replace(/(\.\d*?[1-9])0+$|\.0+$/, '$1');
}

function fmtVector(vector) {
    if (!vector) return '(--, --, --)';
    return `(${fmtNumber(vector.x, 2)}, ${fmtNumber(vector.y, 2)}, ${fmtNumber(vector.z, 2)})`;
}

function severityIcon(severity) {
    if (severity === 'critical') return '!';
    if (severity === 'important') return '\u25C6';
    return '\u2022';
}

function particleTree(cluster, focus, maxNodes = MAX_RENDERED_HIERARCHY_NODES) {
    const byId = new Map(cluster.particles.map(particle => [particle.id, particle]));
    const children = new Map(cluster.particles.map(particle => [particle.id, []]));
    for (const particle of cluster.particles) {
        if (particle.hierarchyParentId === null) continue;
        children.get(particle.hierarchyParentId)?.push(particle);
    }
    for (const rows of children.values()) {
        rows.sort((a, b) => b.dynamicEnergy - a.dynamicEnergy || a.id - b.id);
    }

    let renderedNodes = 0;
    const renderNode = (particle, depth = 0) => {
        if (renderedNodes >= maxNodes) return '';
        renderedNodes++;
        const descendants = children.get(particle.id) || [];
        const anchor = particle.id === cluster.anchorId;
        const inspected = focus?.kind === 'particle' && focus.particleId === particle.id;
        const nearest = particle.nearestId === null ? 'none'
            : `#${particle.nearestId} at ${fmtNumber(particle.nearestDistance, 3)} lu`;
        const parent = particle.hierarchyParentId === null ? 'cluster root'
            : `influenced through #${particle.hierarchyParentId}`;
        return `
            <li class="particle-log-node${anchor ? ' is-anchor' : ''}${inspected ? ' is-inspected' : ''}" data-particle-id="${particle.id}">
                <button type="button" class="particle-log-node-select" data-inspect-particle="${particle.id}"
                        aria-pressed="${inspected ? 'true' : 'false'}"
                        title="Focus inspection on particle #${particle.id} and suppress overlays from unrelated records.">
                    <span class="particle-log-node-row" style="--particle-depth:${depth}">
                        <span class="particle-log-node-branch" aria-hidden="true">${anchor ? '\u25CE' : '\u2514'}</span>
                        <span class="particle-log-node-id">#${particle.id}</span>
                        ${anchor ? '<span class="particle-log-anchor-badge">energy anchor</span>' : ''}
                        <span class="particle-log-node-energy">E* ${fmtNumber(particle.dynamicEnergy)}</span>
                    </span>
                    <span class="particle-log-node-meta">
                        q=${fmtNumber(particle.charge, 1)} \u00B7 v=${fmtNumber(particle.speed)} \u00B7
                        F=${fmtNumber(particle.netForce)} \u00B7 ${escapeHtml(parent)} \u00B7 nearest ${escapeHtml(nearest)}
                    </span>
                    <span class="particle-log-node-position">r ${escapeHtml(fmtVector(particle.position))}</span>
                </button>
                ${descendants.length ? `<ul>${descendants.map(child => renderNode(child, depth + 1)).join('')}</ul>` : ''}
            </li>
        `;
    };

    const root = byId.get(cluster.anchorId) || cluster.particles[0];
    if (!root) return '';
    const tree = renderNode(root);
    const hidden = Math.max(0, cluster.particles.length - renderedNodes);
    return `<ul class="particle-log-tree">${tree}</ul>${hidden
        ? `<div class="particle-log-empty">${hidden} additional records are retained but not mounted in the DOM.</div>`
        : ''}`;
}

export class ParticleLogPanelComponent {
    constructor(element, ledger = scale1ParticleLedger) {
        this.el = element;
        this.ledger = ledger;
        this.visibleCategories = new Set(PARTICLE_LOG_CATEGORIES.map(category => category.id));
        this.lastHierarchyRevision = -1;
        this.lastEventRevision = -1;
        this.initialized = false;
    }

    init() {
        if (!this.el || this.initialized) return this;
        this.initialized = true;
        const toggleRoot = this.el.querySelector('#particle-log-category-toggles');
        if (toggleRoot) {
            toggleRoot.innerHTML = PARTICLE_LOG_CATEGORIES.map(category => `
                <label class="particle-log-category" style="--log-category-color:${category.color}"
                       title="Show or hide ${category.label.toLowerCase()} events in the running ledger.">
                    <input type="checkbox" data-log-category="${category.id}" checked>
                    <span class="particle-log-category-dot" aria-hidden="true"></span>
                    <span>${category.label}</span>
                </label>
            `).join('');
            toggleRoot.addEventListener('change', event => {
                const input = event.target.closest?.('[data-log-category]');
                if (!input) return;
                if (input.checked) this.visibleCategories.add(input.dataset.logCategory);
                else this.visibleCategories.delete(input.dataset.logCategory);
                this.lastEventRevision = -1;
                this.update(true);
            });
        }

        this.el.querySelector('#particle-log-clear')?.addEventListener('click', () => {
            this.ledger.clearEvents();
            this.lastEventRevision = -1;
            this.update(true);
        });

        this.update(true);
        return this;
    }

    _renderEvents(view) {
        const list = this.el.querySelector('#particle-log-event-list');
        if (!list) return;
        const matching = view.events.filter(event => this.visibleCategories.has(event.category));
        const visible = matching.slice(-MAX_RENDERED_EVENTS);
        this.el.querySelector('#particle-log-tick').textContent = String(view.tick);
        this.el.querySelector('#particle-log-events').textContent = String(view.retainedEventCount);
        this.el.querySelector('#particle-log-retention').textContent =
            `${view.retainedEventCount} / ${view.maxEvents} retained`;
        this.el.querySelector('#particle-log-visible-events').textContent = visible.length === matching.length
            ? String(visible.length) : `${visible.length} / ${matching.length}`;
        if (!visible.length) {
            list.innerHTML = '<div class="particle-log-empty">No retained events match the visible categories.</div>';
            return;
        }
        list.innerHTML = visible.map(event => {
            const category = PARTICLE_LOG_CATEGORIES.find(row => row.id === event.category);
            const particles = event.particleIds.length
                ? `<span class="particle-log-event-particles">${event.particleIds.map(id => `#${id}`).join(', ')}</span>` : '';
            const delta = event.energyDelta === null ? ''
                : `<span class="particle-log-event-delta">\u0394E ${fmtNumber(event.energyDelta)}</span>`;
            return `
                <article class="particle-log-event is-${escapeHtml(event.severity)}"
                         style="--log-event-color:${category?.color || '#94a3b8'}"
                         data-event-id="${event.id}" data-event-category="${escapeHtml(event.category)}">
                    <div class="particle-log-event-marker" aria-hidden="true">${severityIcon(event.severity)}</div>
                    <div class="particle-log-event-content">
                        <div class="particle-log-event-head">
                            <time>T ${fmtNumber(event.tick, 0)}</time>
                            <span class="particle-log-event-category">${escapeHtml(category?.label || event.category)}</span>
                            ${particles}${delta}
                        </div>
                        <strong>${escapeHtml(event.title)}</strong>
                        ${event.detail ? `<p>${escapeHtml(event.detail)}</p>` : ''}
                        <div class="particle-log-event-source">${escapeHtml(event.source)}${event.status ? ` \u00B7 [${escapeHtml(String(event.status).toUpperCase())}]` : ''}</div>
                    </div>
                </article>
            `;
        }).join('');
        if (this.el.querySelector('#particle-log-follow')?.checked) {
            requestAnimationFrame(() => {
                const scrollOwner = list.closest('.floating-window-body') ||
                    this.el.querySelector('.particle-log-shell');
                if (scrollOwner) scrollOwner.scrollTop = scrollOwner.scrollHeight;
            });
        }
    }

    update(force = false) {
        if (!this.initialized) return;
        const view = this.ledger.getView();
        if (force || view.hierarchyRevision !== this.lastHierarchyRevision) {
            this.el.querySelector('#particle-log-tick').textContent = String(view.tick);
            this.lastHierarchyRevision = view.hierarchyRevision;
        }
        if (force || view.eventRevision !== this.lastEventRevision) {
            this._renderEvents(view);
            this.lastEventRevision = view.eventRevision;
        }
    }

    destroy() {
        this.initialized = false;
        this.el = null;
    }
}

export class InteractionHierarchyPanelComponent {
    constructor(element, ledger = scale1ParticleLedger, inspector = null) {
        this.el = element;
        this.ledger = ledger;
        this.inspector = inspector;
        this.lastHierarchyRevision = -1;
        this.collapsedClusters = new Set();
        this.expandAll = true;
        this.initialized = false;
        this._onInspectionChange = () => this.update(true);
    }

    init() {
        if (!this.el || this.initialized) return this;
        this.initialized = true;
        document.addEventListener('ftd:scale1-inspection-change', this._onInspectionChange);
        this.el.querySelector('#interaction-hierarchy-expand')?.addEventListener('click', event => {
            this.expandAll = !this.expandAll;
            this.collapsedClusters.clear();
            this.el.querySelectorAll('.particle-log-cluster').forEach(details => {
                details.open = this.expandAll;
                if (!this.expandAll) this.collapsedClusters.add(details.dataset.clusterKey);
            });
            event.currentTarget.textContent = this.expandAll ? 'Collapse' : 'Expand';
        });
        this.el.querySelector('#interaction-hierarchy-root')?.addEventListener('toggle', event => {
            const details = event.target.closest?.('.particle-log-cluster');
            if (!details) return;
            if (details.open) this.collapsedClusters.delete(details.dataset.clusterKey);
            else this.collapsedClusters.add(details.dataset.clusterKey);
            if (details.open && !details.querySelector('.particle-log-tree')) {
                queueMicrotask(() => this.update(true));
            }
        }, true);
        this.el.querySelector('#interaction-hierarchy-root')?.addEventListener('click', event => {
            const particleButton = event.target.closest?.('[data-inspect-particle]');
            if (particleButton) {
                event.preventDefault();
                event.stopPropagation();
                this.inspector?.selectPEParticle?.(Number(particleButton.dataset.inspectParticle));
                return;
            }
            const summary = event.target.closest?.('summary[data-inspect-cluster]');
            if (!summary) return;
            event.preventDefault();
            const details = summary.closest('.particle-log-cluster');
            if (details) details.open = !details.open;
            const cluster = this.ledger.getView().hierarchy.clusters
                .find(candidate => candidate.key === summary.dataset.inspectCluster);
            if (cluster) {
                this.inspector?.selectPECluster?.(
                    cluster,
                    this.ledger.getView().hierarchy.energyBasis,
                );
            }
        });
        this.el.querySelector('#interaction-hierarchy-clear-focus')?.addEventListener('click', () => {
            this.inspector?.clearPEInspection?.();
        });
        this.update(true);
        return this;
    }

    setInspector(inspector) {
        this.inspector = inspector;
        this.update(true);
        return this;
    }

    _renderHierarchy(view) {
        const { hierarchy } = view;
        const focus = this.inspector?.getPEInspectionFocus?.() || null;
        const root = this.el.querySelector('#interaction-hierarchy-root');
        if (!root) return;
        this.el.querySelector('#interaction-hierarchy-tick').textContent = String(view.tick);
        this.el.querySelector('#interaction-hierarchy-particles').textContent = String(hierarchy.particles.length);
        this.el.querySelector('#interaction-hierarchy-clusters').textContent = String(hierarchy.clusters.length);
        this.el.querySelector('#interaction-hierarchy-anchor').textContent = hierarchy.globalAnchorId === null
            ? '--' : `#${hierarchy.globalAnchorId}`;
        this.el.querySelector('#interaction-hierarchy-energy-basis').textContent = hierarchy.energyBasis === 'mass_fallback'
            ? 'dormant mass fallback' : 'dynamic activity';
        const clearFocus = this.el.querySelector('#interaction-hierarchy-clear-focus');
        if (clearFocus) clearFocus.disabled = !focus;

        if (!hierarchy.particles.length) {
            root.innerHTML = '<div class="particle-log-empty">No active Scale 1 particle records.</div>';
            return;
        }

        const perClusterBudget = Math.max(1, Math.floor(
            MAX_RENDERED_HIERARCHY_NODES / Math.max(1, hierarchy.clusters.length)));
        root.innerHTML = `
            <div class="particle-log-global">
                <span class="particle-log-global-label">System energy barycenter</span>
                <strong>${escapeHtml(fmtVector(hierarchy.globalCenter))}</strong>
                <span>anchored visually by particle #${hierarchy.globalAnchorId}</span>
                <span>\u03A3E* ${fmtNumber(hierarchy.totalDynamicEnergy)}</span>
            </div>
            <div class="particle-log-clusters">
                ${hierarchy.clusters.map(cluster => `
                    <details class="particle-log-cluster${focus?.kind === 'cluster' && focus.key === cluster.key ? ' is-inspected' : ''}" data-cluster-key="${escapeHtml(cluster.key)}"
                        ${this.collapsedClusters.has(cluster.key) ? '' : 'open'}>
                        <summary data-inspect-cluster="${escapeHtml(cluster.key)}"
                                 aria-label="Focus ${escapeHtml(cluster.id)} and toggle cluster details"
                                 title="Focus inspection on ${escapeHtml(cluster.id)} and suppress overlays from particles outside this live cluster.">
                            <span class="particle-log-cluster-name">${cluster.id}</span>
                            <span>${cluster.particles.length} particle${cluster.particles.length === 1 ? '' : 's'}</span>
                            <span>anchor #${cluster.anchorId}</span>
                            <span>E* ${fmtNumber(cluster.energy)}</span>
                        </summary>
                        <div class="particle-log-cluster-center">energy center ${escapeHtml(fmtVector(cluster.center))}</div>
                        ${this.collapsedClusters.has(cluster.key)
                            ? '' : particleTree(cluster, focus, perClusterBudget)}
                    </details>
                `).join('')}
            </div>
        `;
    }

    update(force = false) {
        if (!this.initialized) return;
        const view = this.ledger.getView();
        if (force || view.hierarchyRevision !== this.lastHierarchyRevision) {
            this._renderHierarchy(view);
            this.lastHierarchyRevision = view.hierarchyRevision;
        }
    }

    destroy() {
        document.removeEventListener('ftd:scale1-inspection-change', this._onInspectionChange);
        this.initialized = false;
        this.el = null;
    }
}

export function initParticleLogPanel() {
    const element = document.getElementById('panel-particle-log');
    if (!element) return null;
    return new ParticleLogPanelComponent(element).init();
}

export function initInteractionHierarchyPanel(inspector = null) {
    const element = document.getElementById('panel-interaction-hierarchy');
    if (!element) return null;
    return new InteractionHierarchyPanelComponent(element, scale1ParticleLedger, inspector).init();
}

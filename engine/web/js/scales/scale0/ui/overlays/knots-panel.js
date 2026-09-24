import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import {
    isPanelLive,
    PANEL_VISIBILITY_CHANGE_EVENT,
} from '../../../../ui/panels/panel-visibility.js?v=2';
import {
    getScale0State,
    isScale0AuthoritativeGenerationReady,
    subscribeScale0Qualification,
    isKnotTrackingActive,
    isKnotZonesActive,
    markFieldDirty,
    setKnotTracking,
    setKnotTrackingApplicability,
    setKnotZonesApplicability,
    setKnotZonesRequested,
} from '../../state/store.js';
import { getFieldLineKnotTracker, forEachKnotTracker } from '../../runtime/field-line-knots.js';
import { RingBuffer, telemetryHub } from '../../../../telemetry-hub.js';
import { ChartHoverTooltip } from '../../../../ui/charts/chart-hover-tooltip.js';
import { TickHistoryControl } from '../../../../ui/charts/history-window.js';
import { ScenarioApplicabilityBinding } from '../../../../ui/utils/scenario-applicability-binding.js';
import { bindCanvasTip, cells, drawContribChart, drawEnergyLines, drawKnotBars, fmtNum, mergeContrib } from './knots-chart-view.js';

const PANEL_ID = 'knots-panel';
const EMPTY_SCENARIO_ID = 'empty';

// Event type integer order — matches the tracker's event enum:
// 0=Birth 1=Death 2=Persist 3=Fission 4=Fusion 5=Ambiguous.
const EVENT_NAMES = ['Birth', 'Death', 'Persist', 'Fission', 'Fusion', 'Ambig'];
const EVENT_GLYPH = ['✦', '•', '·', '⑂', '⑃', '?'];
// Plain-English meaning of each lifecycle event — shown as the per-row hover tooltip.
const EVENT_DESC = [
    'A new knot formed where the field-lines began to bunch and cross.',
    'A knot dissolved — its field-lines spread back out and it is no longer a clump.',
    'The knot carried over from the previous frame with the same identity.',
    'One knot split into two as its tangle pulled apart.',
    'Two knots merged into one as their tangles overlapped.',
    'An ambiguous reshuffle — several knots split and merged at once, so identities could not be matched one-to-one.',
];

// Field-line knots are detected + tracked entirely in JS by FieldLineKnotTracker
// (a module singleton shared with the E-field overlay job, which feeds it the
// rebuilt streamlines). The panel only READS the tracker — no engine/bridge call.
function buildPanel() {
    const root = document.createElement('div');
    root.id = PANEL_ID;
    root.dataset.applicability = 'applicable';
    root.innerHTML = `
      <div class="knots-applicable-content">
      <div class="kp-title">Field-Line Knots <small>· sampled streamline clumps, effective view · native knots draw with Native transport</small></div>
      <div class="kp-head">
        <span id="kp-track-dot" title="● = tracking on, ○ = off">○</span>
        <span id="kp-alive" title="Knots currently tracked, by field family. A knot is a clump where the field-lines bunch and cross.">tracking off</span>
        <div class="kp-tally" id="kp-tally" title="Knot lifecycle this run — born / died, and split (one knot → two) / merged (two → one)">—</div>
        <div class="kp-contrib-sum" id="kp-contrib-sum" title="How much of the scenario's actual field energy each knot family accounts for — a genuine measurement, not the geometric line counts"></div>
      </div>
      <label class="kp-ctl" title="Detect + track the clumps where E field-lines bunch and cross (observation-only)">
        <input type="checkbox" id="kp-toggle-tracking"> <b>Track field-line knots</b> (per rebuild)
      </label>
      <label class="kp-ctl" title="Show the wireframe boxes around detected field-line knots">
        <input type="checkbox" id="kp-toggle-overlay"> <b>Show knot overlays</b>
      </label>
      <label class="kp-ctl" title="Give each tracked knot its own color (boxes + rows). The selected knot is always white. Off = uniform cyan.">
        <input type="checkbox" id="kp-toggle-color"> <b>Per-knot colors</b>
      </label>
      <label class="kp-ctl" title="How readily a field-line clump counts as a knot. Higher = more (fainter) clumps detected; lower = only the densest.">
        <b class="kp-sensitivity-label">Sensitivity</b>
        <input class="kp-sensitivity-range" type="range" id="kp-sensitivity" min="0" max="100" value="50">
        <span class="kp-sensitivity-value" id="kp-sens-val">50%</span>
      </label>
      <div class="kp-em" id="kp-em">
        <div class="kp-em-h" title="Engine diagnostics U_E + U_B: U_E=½|wave_vel|² and U_B=(C_SPEED²/2)|curl J|². This sum is not the total engine Hamiltonian.">EM DIAGNOSTIC ENERGY <span class="kp-em-tot" id="kp-em-totals"></span></div>
        <div class="kp-em-legend" title="total = diagnostic E+B sum; magnetic includes C_SPEED²; wave equals the electric diagnostic and is not an additional partition. Hover the chart for live values."><span class="kp-key-energy">▬</span> total <span class="kp-key-flux">▬</span> electric <span class="kp-key-magnetic">▬</span> magnetic <span class="kp-key-wave">▬</span> wave</div>
        <canvas class="kp-em-chart" id="kp-em-chart" data-ui-tooltip-skip>Hover for values</canvas>
        <div class="kp-em-h2" title="How the EM energy is split across the individual knots — each bar is one knot's share. Hover a bar for its value.">estimated energy in each clump region</div>
        <canvas class="kp-em-bars" id="kp-em-bars" data-ui-tooltip-skip>Hover for per-knot values</canvas>
      </div>
      <div class="kp-list" id="kp-list" title="One row per tracked knot. Click a row to expand its details, highlight it in white in the 3-D view, and chart its energy share over time."></div>
      <div class="kp-feed-h" title="Knot lifecycle, newest first: ✦ born · • died · ⑂ split in two · ⑃ two merged into one. Each row is tick · field · event · (knots before → after). Hover a row for what it means.">RECENT EVENTS</div>
      <div class="kp-feed" id="kp-feed"></div>
      <div class="kp-note">
        <b>Electric</b>, <b>magnetic</b>, and <b>flux</b> knots are the three streamline families
        of the same substrate — tracking rebuilds them even with overlays off.
        Turn on <b>Radiative E</b>, <b>B Field</b>, or <b>Flux Lines</b> to <i>see</i> the lines.
        <b class="kp-key-energy">energy / flux / charge</b> = each knot's share of the
        scenario's actual field over its region — sampled estimates of reference fields.
        The live dashboard reports a <b>volume-weighted stride-sampled estimate</b>
        of flux |J|, energy ½(E²+B²) in the tracker convention (B normalization may differ from the native audit), and charge |∇·J|; it is marked ≈ and is not an exact full-volume integral.
        The <b>drawn field-line shape</b> (segments / crossings / legs / length) depends on how the lines are seeded —
        it's a Feynman-diagram <b>analogy</b>, <b>NOT</b> a physical amplitude. Ages are counted in whole ticks.
      </div>
      </div>
      <section class="mode-unavailable knots-inapplicable"
               data-applicability="inapplicable" role="status" hidden>
        <strong>Not applicable — imposed null control</strong>
        <p>Scenario 1 · Empty defines no field-line or streamline-sweep domain.
           No field extraction, RK4 line integration, clump detection, lifecycle
           tracking, or contribution measurement is performed.</p>
        <p>A displayed zero-knot count would imply a detector run that did not
           occur. This control is not evidence for physical vacuum or topological
           triviality, and rendered streamline clumps are not knot invariants.</p>
      </section>`;
    return root;
}

export function mountKnotsPanel(host) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    const historyControl = new TickHistoryControl(panel, {
        id: 'knots-panel',
        defaultTicks: 240,
        onChange: () => update(),
    });
    const el = (id) => panel.querySelector(`#${id}`);
    const applicableContent = panel.querySelector('.knots-applicable-content');
    const inapplicableMessage = panel.querySelector('.knots-inapplicable');

    // Scenario EM-energy history (sampled at the panel's 4 Hz from the engine's
    // energy audit). emTotal = ½(E²+B²); E/B/wave are the components.
    const emHub = { emTotal: new RingBuffer(240), eField: new RingBuffer(240), bField: new RingBuffer(240), wave: new RingBuffer(240) };
    let emResetVersion = -1;
    let lastAuditStamp = null;
    let inapplicable = false;
    let disposed = false;
    let armSub = null;
    let liveSub = null;
    let measurementActive = false;
    let updateCount = 0;
    let scenarioBinding = null;

    // Shared hover tooltip for all charts (value-at-cursor). The static charts are
    // bound once here; the per-knot history chart (rebuilt each paint) binds in update().
    const chartTip = new ChartHoverTooltip(panel);
    bindCanvasTip(el('kp-em-chart'), chartTip, panel);
    bindCanvasTip(el('kp-em-bars'), chartTip, panel);

    const trackCb = el('kp-toggle-tracking');
    const overlayCb = el('kp-toggle-overlay');
    const colorCb = el('kp-toggle-color');

    // Per-knot colors: when on, each knot gets its own deterministic color in
    // both the viewport boxes and the panel rows; the selected knot is white
    // regardless. When off, knots are uniform cyan.
    colorCb.checked = getFieldLineKnotTracker('e').getPerKnotColor();
    colorCb.addEventListener('change', (e) => {
        if (inapplicable || panel.dataset.applicability !== 'applicable') return;
        forEachKnotTracker((t) => t.setPerKnotColor(e.target.checked));
        markFieldDirty();   // recolor the flowlines + boxes on the next sweep
        update();
    });

    // Detection sensitivity: higher → lower density threshold → more clumps
    // qualify as knots. Re-detection happens in the next overlay sweep, so mark
    // the field dirty to force one.
    const sensSlider = el('kp-sensitivity');
    const sensVal = el('kp-sens-val');
    sensSlider.value = Math.round(getFieldLineKnotTracker('e').getSensitivity() * 100);
    sensVal.textContent = sensSlider.value + '%';
    sensSlider.addEventListener('input', (e) => {
        if (inapplicable || panel.dataset.applicability !== 'applicable') return;
        const pct = +e.target.value;
        sensVal.textContent = pct + '%';
        forEachKnotTracker((t) => t.setSensitivity(pct / 100));
        markFieldDirty();
    });

    // The overlay checkbox drives the VISUAL flag (colored boxes around the
    // detected knots). The boxes are meaningless without tracking data, so
    // enabling the overlay auto-enables tracking (and syncs its checkbox).
    overlayCb.checked = !!getScale0State().knotZonesRequested;
    overlayCb.addEventListener('change', (e) => {
        if (inapplicable || panel.dataset.applicability !== 'applicable') return;
        const on = e.target.checked;
        setKnotZonesRequested(on);
        if (on && !getScale0State().knotTracking) {
            setKnotTracking(true);
            trackCb.checked = true;
        }
        window.__ftdCtx?.viewport?.toggleKnotZones?.(
            isKnotZonesActive(getScale0State()),
        );
    });

    // The tracking checkbox enables the JS FieldLineKnotTracker recorder (fed from
    // the E-field overlay job). Reset on un-check so stale knots/zones clear.
    trackCb.checked = !!getScale0State().knotTracking;
    trackCb.addEventListener('change', (e) => {
        if (inapplicable || panel.dataset.applicability !== 'applicable') return;
        setKnotTracking(e.target.checked);
        if (!e.target.checked) forEachKnotTracker((t) => t.reset());
        window.__ftdCtx?.viewport?.toggleKnotZones?.(
            isKnotZonesActive(getScale0State()),
        );
    });

    let expandedKey = null;   // "<field>:<id>" of the expanded/selected knot row
    let listStructureKey = '';
    let listRows = new Map();
    let listHeaders = new Map();
    let listRenderRaf = 0;
    let pendingListRender = null;
    let lastFeedHtml = null;
    let feedRenderRaf = 0;
    let pendingFeedRender = null;
    let chartRenderRaf = 0;
    let pendingChartRender = null;

    function setHtmlIfChanged(node, html) {
        if (node.innerHTML !== html) node.innerHTML = html;
    }

    function renderEmptyList(list, trackingOn) {
        if (listRenderRaf) cancelAnimationFrame(listRenderRaf);
        listRenderRaf = 0;
        pendingListRender = null;
        listStructureKey = '';
        listRows.clear();
        listHeaders.clear();
        if (!trackingOn) {
            setHtmlIfChanged(list, '<div class="kp-empty">tracking off — enable "Track field-line knots" to detect knots</div>');
        } else {
            setHtmlIfChanged(list, '<div class="kp-empty">0 knots — tracking is running a streamline sweep (overlays optional); '
                + 'knots form where field-lines bunch (no particles needed)</div>');
        }
    }

    function commitListStructure(list, nextStructureKey, html) {
        listStructureKey = nextStructureKey;
        list.innerHTML = html;
        listRows = new Map();
        listHeaders = new Map();
        list.querySelectorAll('[data-kp-field-head]').forEach((head) => {
            listHeaders.set(head.dataset.kpFieldHead, head.querySelector('[data-kp-field-count]'));
        });
        list.querySelectorAll('.kp-row').forEach((r) => {
            listRows.set(r.dataset.slot, {
                row: r,
                dot: r.querySelector('[data-kp-dot]'),
                dotColor: null,
                id: r.querySelector('[data-kp-id]'),
                cells: r.querySelector('[data-kp-cells]'),
                age: r.querySelector('[data-kp-age]'),
                energy: r.querySelector('[data-kp-energy]'),
                flux: r.querySelector('[data-kp-flux]'),
                charge: r.querySelector('[data-kp-charge]'),
            });
            r.onclick = () => {
                const fk = r.dataset.field, id = +r.dataset.id, selectedKey = `${fk}:${id}`;
                expandedKey = (expandedKey === selectedKey ? null : selectedKey);
                forEachKnotTracker((t, f) => t.setSelected((expandedKey && f === fk) ? id : -1));
                markFieldDirty();
                update();
            };
        });
        list.querySelectorAll('canvas.kp-chart').forEach((cv) => bindCanvasTip(cv, chartTip, panel));
    }

    function scheduleListStructure(list, nextStructureKey, html) {
        pendingListRender = { list, nextStructureKey, html };
        if (listRenderRaf) return;
        listRenderRaf = requestAnimationFrame(() => {
            listRenderRaf = 0;
            const pending = pendingListRender;
            pendingListRender = null;
            if (!pending || disposed || !panel.isConnected || !measurementActive || !isPanelLive(host)) return;
            commitListStructure(pending.list, pending.nextStructureKey, pending.html);
        });
    }

    function scheduleFeedRender(feed, html) {
        pendingFeedRender = { feed, html };
        if (feedRenderRaf) return;
        feedRenderRaf = requestAnimationFrame(() => {
            feedRenderRaf = 0;
            const pending = pendingFeedRender;
            pendingFeedRender = null;
            if (!pending || disposed || !panel.isConnected || !measurementActive || !isPanelLive(host)) return;
            pending.feed.innerHTML = pending.html;
            lastFeedHtml = pending.html;
        });
    }

    function scheduleChartRender(payload) {
        pendingChartRender = payload;
        if (chartRenderRaf) return;
        chartRenderRaf = requestAnimationFrame(() => {
            chartRenderRaf = 0;
            const next = pendingChartRender;
            pendingChartRender = null;
            if (!next || disposed || !panel.isConnected || !measurementActive || !isPanelLive(host)) return;
            if (next.drawEnergy) {
                drawEnergyLines(el('kp-em-chart'), [
                    { rb: emHub.emTotal, color: '#f6c453', width: 1.7, label: 'total (E+B)' },
                    { rb: emHub.eField, color: '#5ad2e0', label: 'electric' },
                    { rb: emHub.bField, color: '#f08bb0', label: 'magnetic' },
                    { rb: emHub.wave, color: '#9be08b', label: 'wave' },
                ], historyControl);
            }
            drawKnotBars(el('kp-em-bars'), mergeContrib(next.eC, next.bC, next.jC));
        });
    }

    function update() {
        // Empty and pending scenario generations are a hard scientific
        // boundary. Direct/manual calls remain inert before tracker, telemetry,
        // canvas, or DOM access.
        if (inapplicable || getScale0State().currentScenarioId === EMPTY_SCENARIO_ID) {
            if (!inapplicable) setEmptyApplicability(true);
            return;
        }
        if (panel.dataset.applicability !== 'applicable' || !measurementActive
            || !isScale0AuthoritativeGenerationReady(getScale0State())) return;
        if (!isPanelLive(host)) return;
        updateCount++;
        const trackingOn = !!getScale0State().knotTracking;
        el('kp-track-dot').textContent = trackingOn ? '●' : '○';

        if (!trackingOn) {
            el('kp-alive').textContent = 'tracking off';
            el('kp-tally').textContent = '';
            el('kp-contrib-sum').textContent = '';
            el('kp-em').style.display = 'none';
            renderEmptyList(el('kp-list'), false);
            setHtmlIfChanged(el('kp-feed'), '');
            lastFeedHtml = '';
            return;
        }
        el('kp-em').style.display = '';

        const E = getFieldLineKnotTracker('e'), B = getFieldLineKnotTracker('b');
        const J = getFieldLineKnotTracker('flux');
        const FIELDS = [
            { key: 'e', tag: 'E', name: 'Electric', tr: E },
            { key: 'b', tag: 'B', name: 'Magnetic', tr: B },
            { key: 'flux', tag: 'J', name: 'Flux', tr: J },
        ];
        const eAgg = E.getAggregate(), bAgg = B.getAggregate(), jAgg = J.getAggregate();
        const unknownFields = FIELDS.filter(f => f.tr.getTelemetry()?.status === 'sample-tick-unavailable');
        if (unknownFields.length === FIELDS.length) {
            if (feedRenderRaf) cancelAnimationFrame(feedRenderRaf);
            if (chartRenderRaf) cancelAnimationFrame(chartRenderRaf);
            feedRenderRaf = chartRenderRaf = 0;
            pendingFeedRender = pendingChartRender = null;
            lastFeedHtml = '';
            el('kp-alive').textContent = 'sample time unavailable';
            el('kp-tally').textContent = 'No current time-qualified clump observation';
            el('kp-contrib-sum').textContent = '';
            el('kp-em').style.display = 'none';
            renderEmptyList(el('kp-list'), false);
            el('kp-list').innerHTML = '<div class="kp-empty">The current sampler has no engine-tick provenance. Clump counts and histories are unavailable; no zero count was measured.</div>';
            setHtmlIfChanged(el('kp-feed'), '');
            return;
        }
        const eC = E.getContributions(), bC = B.getContributions(), jC = J.getContributions();
        const pct = (v) => `${Math.round((v || 0) * 100)}%`;

        // Header counts (plain words) + per-field "how much energy these knots hold".
        const eTel0 = E.getTelemetry(), bTel0 = B.getTelemetry(), jTel0 = J.getTelemetry();
        const dropped = (eTel0.dropped || 0) + (bTel0.dropped || 0) + (jTel0.dropped || 0);
        const countLabel = tel => tel.status === 'sample-tick-unavailable' ? 'unavailable' : tel.count;
        setHtmlIfChanged(el('kp-alive'), `<b>${countLabel(eTel0)}</b> electric · <b>${countLabel(bTel0)}</b> magnetic · <b>${countLabel(jTel0)}</b> flux clumps`
            + (dropped ? ` <span class="kp-dim">(showing largest; ${dropped} more dropped)</span>` : ''));
        el('kp-tally').textContent =
            `${(eAgg.births || 0) + (bAgg.births || 0) + (jAgg.births || 0)} born · ${(eAgg.deaths || 0) + (bAgg.deaths || 0) + (jAgg.deaths || 0)} died`
            + ` · ${(eAgg.fissions || 0) + (bAgg.fissions || 0) + (jAgg.fissions || 0)} split · ${(eAgg.fusions || 0) + (bAgg.fusions || 0) + (jAgg.fusions || 0)} merged`;

        const anyC = (eC.count && eC.totals.energy > 0) || (bC.count && bC.totals.energy > 0) || (jC.count && jC.totals.energy > 0);
        const sampleStride = Math.max(eC.sampling?.energyStride || 1, bC.sampling?.energyStride || 1,
            jC.sampling?.fluxStride || 1);
        const estimateTag = sampleStride > 1 ? `≈ stride ${sampleStride} estimate · ` : '';
        const contributionUnavailable = [eC, bC, jC].some(c =>
            c.status === 'sample-provenance-unavailable' || c.status === 'sample-tick-unavailable');
        setHtmlIfChanged(el('kp-contrib-sum'), contributionUnavailable
            ? '<span style="opacity:.7">Contribution measurements unavailable: the field samples do not identify a matching source, state and tick.</span>'
            : anyC
                ? `${estimateTag}These knots hold <b>${pct(eC.captured.energyFrac)}</b> of the field energy (electric) and <b>${pct(bC.captured.energyFrac)}</b> (magnetic)`
                : '<span style="opacity:.7">waiting for a streamline sweep to measure each knot\'s share</span>');

        // ── Scenario EM energy: total + electric/magnetic breakdown over time ──
        // From the engine's energy audit; EM field energy U = ½(E²+B²).
        const nextResetVersion = telemetryHub.getResetVersion?.(0) ?? 0;
        if (nextResetVersion !== emResetVersion) {
            emResetVersion = nextResetVersion;
            lastAuditStamp = null;
            for (const buffer of Object.values(emHub)) buffer.clear();
        }
        const auditMeta = telemetryHub.getScale0TelemetryMeta?.('audit') ?? null;
        const audit = auditMeta && !auditMeta.stale ? telemetryHub.s0?.audit : null;
        const emRoot = el('kp-em');
        emRoot.dataset.telemetryState = audit ? 'current' : 'stale';
        if (audit) {
            const eEn = audit.EFieldEnergy ?? audit.eFieldEnergy;
            const bEn = audit.BFieldEnergy ?? audit.bFieldEnergy;
            const wv = audit.waveEnergy;
            const U = [eEn, bEn].every(Number.isFinite) ? eEn + bEn : Number.NaN;
            const stamp = `${auditMeta.sourceEpoch ?? auditMeta.epoch ?? 'local'}:`
                + `${auditMeta.stateVersion ?? auditMeta.tick ?? auditMeta.snapshotVersion}`;
            if (stamp !== lastAuditStamp) {
                lastAuditStamp = stamp;
                emHub.emTotal.push(U, auditMeta.tick);
                emHub.eField.push(eEn, auditMeta.tick);
                emHub.bField.push(bEn, auditMeta.tick);
                emHub.wave.push(wv, auditMeta.tick);
            }
            setHtmlIfChanged(el('kp-em-totals'), Number.isFinite(U)
                ? `<b>${fmtNum(U)}</b> total · electric ${pct(U > 0 ? eEn / U : 0)} · magnetic ${pct(U > 0 ? bEn / U : 0)}`
                : '<b>—</b> total · one or more audit channels unavailable');
        } else {
            setHtmlIfChanged(el('kp-em-totals'), '<b>—</b> awaiting a current energy-audit snapshot');
        }
        // per-knot quantization bars — both E and B families merged
        scheduleChartRender({ drawEnergy: !!audit, eC, bC, jC });

        // Per-knot list — E, then B, then flux, each row tagged + field-hued.
        // Decode stride 8: [0..2] cx,cy,cz · [3] segs · [4] crossings · [5] legs · [6] length · [7] |Φ|
        const list = el('kp-list');
        let html = '', anyKnots = false;
        const structure = [];
        const headerUpdates = [];
        const rowUpdates = [];
        for (const fld of FIELDS) {
            const tel = fld.tr.getTelemetry();
            if (!tel.count) continue;
            anyKnots = true;
            const contrib = fld.tr.getContributions();
            const cIdx = new Map();
            for (let i = 0; i < (contrib.count || 0); i++) cIdx.set(contrib.ids[i], i);
            const selectedId = fld.tr.getSelected();
            const perColor = fld.tr.getPerKnotColor();
            const f = tel.fields, S = tel.stride || 8;
            // Keep the live list bounded independently of the detector's
            // scientific maxKnots=40. The panel exposes the total count and
            // explicitly reports the omitted remainder; the four dominant
            // knots per field remain individually inspectable while the 4 Hz
            // monitor stays inside its strict callback budget at L=97.
            const MAX_ROWS = 4;
            const shown = Math.min(tel.count, MAX_ROWS);
            const headHue = Math.round(knotHue(0, fld.key) * 360);
            structure.push(`${fld.key}:${shown}`);
            headerUpdates.push({ key: fld.key, count: String(tel.count) });
            html += `<div class="kp-field-h" data-kp-field-head="${fld.key}" style="color:hsl(${headHue},70%,62%)" title="Knots detected in the ${fld.name.toLowerCase()} field's streamlines. Each is a clump where the ${fld.tag}-field lines bunch and cross.">${fld.name}-field knots · <span data-kp-field-count>${tel.count}</span></div>`;
            for (let k = 0; k < shown; k++) {
                const id = tel.ids[k]; const key = `${fld.key}:${id}`; const slotKey = `${fld.key}:${k}`;
                const segs = f[k * S + 3] | 0, xings = f[k * S + 4] | 0, legs = f[k * S + 5] | 0;
                const dotCol = (id === selectedId) ? '#ffffff'
                    : (perColor ? `hsl(${Math.round(knotHue(id, fld.key) * 360)},85%,62%)` : (fld.key === 'b' ? '#6fcf86' : fld.key === 'flux' ? '#f6c453' : '#3fd0e0'));
                const ci = cIdx.get(id);
                const meta = `<span class="kp-dim"><span data-kp-cells>${cells(tel.size[k])}</span> · age <span data-kp-age>${tel.age[k]}</span>t</span>`;
                const body = `<span class="kp-cn">energy <b data-kp-energy>${ci != null ? pct(contrib.energyFrac[ci]) : '…'}</b> · flux <b data-kp-flux>${ci != null ? pct(contrib.fluxFrac[ci]) : '…'}</b> · charge <b data-kp-charge>${ci != null ? pct(contrib.chargeFrac[ci]) : '…'}</b></span> · ${meta}`;
                const rowTitle = `${fld.name}-field knot #${id} — energy/flux/charge are this knot's share of the whole scenario's field (measured over its region). `
                    + `"cells" is the knot's size on the detection grid, "age" is how many ticks it has lived. Click to expand and highlight it in the 3-D view.`;
                structure.push(`${slotKey}:${key === expandedKey ? 1 : 0}`);
                rowUpdates.push({ slotKey, id, title: rowTitle, dotCol, cells: cells(tel.size[k]), age: String(tel.age[k]),
                    energy: ci != null ? pct(contrib.energyFrac[ci]) : '…',
                    flux: ci != null ? pct(contrib.fluxFrac[ci]) : '…',
                    charge: ci != null ? pct(contrib.chargeFrac[ci]) : '…' });
                html += `<div class="kp-row" data-slot="${slotKey}" data-field="${fld.key}" data-id="${id}" title="${rowTitle}">`
                     +  `<span data-kp-dot style="color:${dotCol}">●</span> <b data-kp-id>#${id}</b> · ${body}`;
                if (key === expandedKey) {
                    const cx = f[k * S].toFixed(0), cy = f[k * S + 1].toFixed(0), cz = f[k * S + 2].toFixed(0);
                    const ex = tel.extents[k * 3].toFixed(1), ey = tel.extents[k * 3 + 1].toFixed(1), ez = tel.extents[k * 3 + 2].toFixed(1);
                    const len = f[k * S + 6], fm = f[k * S + 7].toFixed(2);
                    const dx = tel.dirs[k * 3].toFixed(2), dy = tel.dirs[k * 3 + 1].toFixed(2), dz = tel.dirs[k * 3 + 2].toFixed(2);
                    const cstr = (ci != null)
                        ? `<b>share of the scenario</b> <i class="kp-dim">[measured]</i><br>`
                          + `&nbsp;&nbsp;energy <b>${pct(contrib.energyFrac[ci])}</b> · flux <b>${pct(contrib.fluxFrac[ci])}</b> · charge <b>${pct(contrib.chargeFrac[ci])}</b><br>`
                          + `&nbsp;&nbsp;<span class="kp-dim">absolute: U ${fmtNum(contrib.energy[ci])} · |Φ| ${fmtNum(contrib.flux[ci])} · Q ${fmtNum(contrib.charge[ci])}</span><br>`
                          + `<span class="kp-legend"><span style="color:#f6c453">▬</span> energy <span style="color:#5ad2e0">▬</span> flux <span style="color:#c98bf0">▬</span> charge · share over time</span>`
                          + `<canvas class="kp-chart" data-cid="${key}"></canvas>`
                        : '<span class="kp-dim">measuring this knot\'s field share…</span><br>';
                    html += `<div class="kp-det">`
                         +  `<span class="kp-dim">born tick ${tel.birth[k]} · peak ${cells(tel.peak[k])} · center (${cx},${cy},${cz}) · size (${ex},${ey},${ez})</span><br>`
                         +  cstr
                         +  `<span class="kp-geo">drawn field-line shape <i>[seeding-dependent analogy, not physical]</i>: ${fmtNum(segs)} segments · ${fmtNum(xings)} crossings · ${legs} legs · length ${fmtNum(len)}</span><br>`
                         +  `<span class="kp-dim">net flux |Φ| ${fm}, pointing (${dx},${dy},${dz})</span>`
                         +  `</div>`;
                }
                html += `</div>`;
            }
            if (tel.count > MAX_ROWS) html += `<div class="kp-empty">… and ${tel.count - MAX_ROWS} more ${fld.name.toLowerCase()} knots (showing first ${MAX_ROWS})</div>`;
        }
        if (!anyKnots) {
            renderEmptyList(list, true);
        } else {
            const nextStructureKey = structure.join('|');
            if (nextStructureKey !== listStructureKey) {
                // Commit structural DOM outside the 4 Hz telemetry callback.
                // The latest snapshot replaces any earlier pending one; frame
                // timing remains the authoritative end-to-end performance gate.
                scheduleListStructure(list, nextStructureKey, html);
            }
            for (const next of headerUpdates) {
                const count = listHeaders.get(next.key);
                if (count && count.textContent !== next.count) count.textContent = next.count;
            }
            for (const next of rowUpdates) {
                const cached = listRows.get(next.slotKey);
                if (!cached) continue;
                const nextId = String(next.id);
                if (cached.row.dataset.id !== nextId) cached.row.dataset.id = nextId;
                if (cached.row.title !== next.title) cached.row.title = next.title;
                const idLabel = `#${nextId}`;
                if (cached.id.textContent !== idLabel) cached.id.textContent = idLabel;
                if (cached.dotColor !== next.dotCol) {
                    cached.dot.style.color = next.dotCol;
                    cached.dotColor = next.dotCol;
                }
                if (cached.cells.textContent !== next.cells) cached.cells.textContent = next.cells;
                if (cached.age.textContent !== next.age) cached.age.textContent = next.age;
                if (cached.energy && cached.energy.textContent !== next.energy) cached.energy.textContent = next.energy;
                if (cached.flux && cached.flux.textContent !== next.flux) cached.flux.textContent = next.flux;
                if (cached.charge && cached.charge.textContent !== next.charge) cached.charge.textContent = next.charge;
            }
            if (expandedKey) {
                list.querySelectorAll('canvas.kp-chart').forEach((cv) => {
                    const [fk, idStr] = cv.dataset.cid.split(':');
                    drawContribChart(cv, getFieldLineKnotTracker(fk).getKnotHistory(+idStr), historyControl);
                });
            }
        }

        // Event feed — E + B merged, newest first, tagged.
        const feed = el('kp-feed');
        const merged = [];
        for (const fld of FIELDS) {
            const ev = fld.tr.getEvents();
            for (let i = 0; i < (ev.count || 0); i++) merged.push({ tick: ev.tick[i], type: ev.type[i], np: ev.nparents[i], nc: ev.nchildren[i], tag: fld.tag });
        }
        merged.sort((a, b) => b.tick - a.tick);
        let feedHtml;
        if (!merged.length) {
            feedHtml = '<div class="kp-empty">no events yet</div>';
        } else {
            let h = '';
            for (let i = 0; i < Math.min(12, merged.length); i++) {
                const e = merged[i];
                const name = EVENT_NAMES[e.type] ?? 'Event';
                const fieldName = e.tag === 'B' ? 'magnetic' : e.tag === 'J' ? 'flux' : 'electric';
                const before = `${e.np} knot${e.np === 1 ? '' : 's'} before → ${e.nc} after`;
                const title = `${name} · tick ${e.tick} · ${fieldName} field. ${EVENT_DESC[e.type] ?? ''} (${before})`;
                h += `<div title="${title}"><span class="kp-t">t${e.tick}</span> ${e.tag} ${EVENT_GLYPH[e.type] ?? '?'} ${name} <span class="kp-t">(${e.np}→${e.nc})</span></div>`;
            }
            feedHtml = h;
        }
        if (feedHtml !== lastFeedHtml) {
            scheduleFeedRender(feed, feedHtml);
        }
    }

    function clearScientificState() {
        emResetVersion = -1;
        lastAuditStamp = null;
        for (const buffer of Object.values(emHub)) buffer.clear();
        expandedKey = null;
        listStructureKey = '';
        listRows.clear();
        listHeaders.clear();
        if (listRenderRaf) cancelAnimationFrame(listRenderRaf);
        listRenderRaf = 0;
        pendingListRender = null;
        lastFeedHtml = null;
        if (feedRenderRaf) cancelAnimationFrame(feedRenderRaf);
        feedRenderRaf = 0;
        pendingFeedRender = null;
        if (chartRenderRaf) cancelAnimationFrame(chartRenderRaf);
        chartRenderRaf = 0;
        pendingChartRender = null;
        chartTip.hide();
        forEachKnotTracker((tracker) => tracker.reset());
    }

    function setMeasurementActive(on) {
        const next = !!on && !inapplicable && !disposed;
        if (measurementActive === next) return;
        measurementActive = next;
        forEachKnotTracker((tracker) => tracker.setContribEnabled(next));
        // Contribution fields are fetched only for a live Knots panel. Force
        // one fresh sweep when it opens so the first displayed ratios are not
        // retained from a previously hidden panel.
        if (next && isKnotTrackingActive(getScale0State())) markFieldDirty();
    }

    function stopLiveCoordinator() {
        liveSub?.unsubscribe?.();
        liveSub = null;
        setMeasurementActive(false);
    }

    function stopAllCoordinators() {
        armSub?.unsubscribe?.();
        armSub = null;
        stopLiveCoordinator();
    }

    function reconcileVisibility() {
        if (inapplicable || disposed || panel.dataset.applicability !== 'applicable') return;
        if (!isPanelLive(host)) {
            stopLiveCoordinator();
            return;
        }
        setMeasurementActive(true);
        if (!liveSub) {
            update();
            liveSub = rafCoordinator.subscribe(PANEL_ID, { hz: 4, cb: () => {
                if (!isPanelLive(host)) {
                    stopLiveCoordinator();
                    return;
                }
                update();
            } });
        }
    }

    function startVisibilityCoordinator() {
        if (armSub || inapplicable || disposed) return;
        reconcileVisibility();
        armSub = rafCoordinator.subscribe(`${PANEL_ID}-arm`, { hz: 2, cb: reconcileVisibility });
    }

    // Dock/tab/floating visibility changes are explicit synchronous boundaries.
    // The low-rate arm coordinator remains a recovery mechanism, but must not
    // leave contribution field reads enabled for even one in-flight sweep after
    // the panel becomes invisible.
    function onPanelVisibilityChange() {
        if (disposed || inapplicable || panel.dataset.applicability !== 'applicable'
            || !isPanelLive(host)) {
            stopLiveCoordinator();
            return;
        }
        reconcileVisibility();
    }
    window.addEventListener(PANEL_VISIBILITY_CHANGE_EVENT, onPanelVisibilityChange);

    function setControlsDisabled(disabled) {
        applicableContent.querySelectorAll('button, input, select').forEach((control) => {
            control.disabled = !!disabled;
        });
    }

    function setKnotZoneApplicability(on) {
        const effective = setKnotZonesApplicability(on);
        overlayCb.checked = !!getScale0State().knotZonesRequested;
        window.__ftdCtx?.viewport?.toggleKnotZones?.(effective);
        return effective;
    }

    function setEmptyApplicability(nextValue) {
        const next = !!nextValue;
        inapplicable = next;
        panel.dataset.applicability = next ? 'inapplicable-empty' : 'applicable';
        panel.classList.toggle('is-inapplicable', next);
        applicableContent.hidden = next;
        applicableContent.setAttribute('aria-hidden', next ? 'true' : 'false');
        inapplicableMessage.hidden = !next;
        setControlsDisabled(next);

        if (next) {
            stopAllCoordinators();
            setKnotTrackingApplicability(false);
            setKnotZoneApplicability(false);
            clearScientificState();
        } else {
            setKnotTrackingApplicability(true);
            setKnotZoneApplicability(true);
            trackCb.checked = !!getScale0State().knotTracking;
            startVisibilityCoordinator();
        }
    }

    function handleScenarioIntent(scenarioId) {
        if (listRenderRaf) cancelAnimationFrame(listRenderRaf);
        listRenderRaf = 0;
        pendingListRender = null;
        if (feedRenderRaf) cancelAnimationFrame(feedRenderRaf);
        feedRenderRaf = 0;
        pendingFeedRender = null;
        if (chartRenderRaf) cancelAnimationFrame(chartRenderRaf);
        chartRenderRaf = 0;
        pendingChartRender = null;

        // Stop the old generation synchronously on selection intent. This is
        // required even for nonempty→nonempty loads because tracker histories
        // and in-flight streamline jobs belong to one engine generation.
        stopAllCoordinators();
        setKnotTrackingApplicability(false);
        setKnotZoneApplicability(false);
        clearScientificState();

        if (scenarioId === EMPTY_SCENARIO_ID) {
            setEmptyApplicability(true);
            return;
        }

        panel.dataset.applicability = 'pending-scenario';
        applicableContent.hidden = true;
        applicableContent.setAttribute('aria-hidden', 'true');
        inapplicableMessage.hidden = true;
        setControlsDisabled(true);

        scenarioBinding.reconcile({
            scenarioId,
            isReady: () => getScale0State().currentScenarioId === scenarioId
                && isScale0AuthoritativeGenerationReady(getScale0State()),
            onReady: () => setEmptyApplicability(false),
        });
    }

    function rebindScenarioApplicability() {
        scenarioBinding?.bind();
    }

    scenarioBinding = new ScenarioApplicabilityBinding({
        getCurrentScenarioId: () => getScale0State().currentScenarioId,
        onIntent: handleScenarioIntent,
    });
    rebindScenarioApplicability();
    let boundary = null;
    const unsubscribeQualification = subscribeScale0Qualification(q => {
        const next = q.authoritativeLoad
            ? `${q.authoritativeLoad.status}:${q.authoritativeLoad.loadGeneration}`
            : `ready:${q.anchor?.loadGeneration}`;
        if (next === boundary) return;
        boundary = next;
        scenarioBinding.intent(q.scenarioId);
    });

    // Match the sibling singleton panels (e.g. genesis-burst-panel): null the
    // global on dispose, but only if it still points at THIS instance, so a
    // newer mount that already replaced the global isn't clobbered. Without the
    // null-out the stale {dispose} lingered on window after teardown.
    const api = {
        update,
        rebindScenarioApplicability,
        get applicability() { return inapplicable ? 'inapplicable-empty' : panel.dataset.applicability; },
        get coordinatorActive() { return !!armSub || !!liveSub; },
        get measurementActive() { return measurementActive; },
        get updateCount() { return updateCount; },
        get historyLength() { return emHub.emTotal.count; },
        get trackingEffective() { return isKnotTrackingActive(getScale0State()); },
        get knotZonesRequested() { return !!getScale0State().knotZonesRequested; },
        get knotZonesEffective() { return isKnotZonesActive(getScale0State()); },
        get contributionEnabled() {
            return ['e', 'b', 'flux'].some((field) => getFieldLineKnotTracker(field).isContribEnabled());
        },
    };
    api.dispose = () => {
        unsubscribeQualification();
        disposed = true;
        stopAllCoordinators();
        setKnotTrackingApplicability(
            getScale0State().currentScenarioId !== EMPTY_SCENARIO_ID,
        );
        setKnotZoneApplicability(
            getScale0State().currentScenarioId !== EMPTY_SCENARIO_ID,
        );
        scenarioBinding?.dispose();
        scenarioBinding = null;
            window.removeEventListener(PANEL_VISIBILITY_CHANGE_EVENT, onPanelVisibilityChange);
            historyControl.destroy();
        forEachKnotTracker((t) => t.setContribEnabled(false));
        panel.remove();
        if (typeof window !== 'undefined' && window.__ftdKnotsPanel === api) {
            window.__ftdKnotsPanel = null;
        }
    };
    window.__ftdKnotsPanel = api;
    return api;
}

export function initKnotsPanel() {
    if (typeof window === 'undefined') return;
    if (window.__ftdKnotsPanel) return window.__ftdKnotsPanel;
    const host = document.getElementById('panel-knots');
    if (!host) return;
    return mountKnotsPanel(host);
}

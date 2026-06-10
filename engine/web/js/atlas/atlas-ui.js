// FTD Ontology Atlas — UI: layer panel, detail panel, chain stepper, mode switch.
//
//   createUI(dom, ctx) → { showDetail, syncToggles, setMode, getMode }
//   dom = { layerPanel, detailPanel, stepper, modeButtons:[<button data-mode>] }
//   ctx = { layers, LAYERS, GROUPS, STAGES, scene, overlay, api }
//
// The panel re-presents canonical content (atlas-content.js). Epistemic tags
// are shown verbatim and coloured by ontological group — never softened.
// All visibility changes route through ctx.api.setLayerVisible so guided/free
// mode stays consistent; syncToggles() reflects live layer state back into the
// checkboxes (the stepper calls it after each stage).

// hex (#rrggbb or 0xRRGGBB or number) → "r, g, b" for rgba() fills.
function rgbTriplet(hex) {
  let h = hex;
  if (typeof h === 'number') h = '#' + h.toString(16).padStart(6, '0');
  h = String(h).replace('#', '').replace(/^0x/i, '');
  if (h.length === 3) h = h.split('').map((c) => c + c).join('');
  const n = parseInt(h, 16);
  return `${(n >> 16) & 255}, ${(n >> 8) & 255}, ${n & 255}`;
}

export function createUI(dom, ctx) {
  const { layers, LAYERS, GROUPS, STAGES, api } = ctx;
  const groupColor = new Map(GROUPS.map((g) => [g.id, g.color]));

  // Layer ids that belong to each group, in declaration order of LAYERS.
  const layerIdsByGroup = new Map(GROUPS.map((g) => [g.id, []]));
  for (const id of Object.keys(LAYERS)) {
    const g = LAYERS[id].group;
    if (layerIdsByGroup.has(g)) layerIdsByGroup.get(g).push(id);
  }

  // ── Left: grouped layer toggles ─────────────────────────────────────────
  const rowCheckbox = new Map();   // layerId → <input type=checkbox>
  const groupCheckbox = new Map(); // groupId → <input type=checkbox> (master)

  function buildLayerPanel() {
    const panel = dom.layerPanel;
    panel.innerHTML = '';
    panel.classList.add('atlas-layer-panel');

    for (const grp of GROUPS) {
      const ids = layerIdsByGroup.get(grp.id) || [];
      if (!ids.length) continue;

      const section = document.createElement('section');
      section.className = 'atlas-group';

      // Group header: colour dot + master checkbox + label.
      const header = document.createElement('label');
      header.className = 'atlas-group-head';
      const master = document.createElement('input');
      master.type = 'checkbox';
      master.className = 'atlas-master';
      master.addEventListener('change', () => {
        for (const id of ids) api.setLayerVisible(id, master.checked);
        ctx.onUserInteract?.();
        syncToggles();
      });
      groupCheckbox.set(grp.id, master);
      const dot = document.createElement('span');
      dot.className = 'atlas-dot';
      dot.style.background = grp.color;
      const lbl = document.createElement('span');
      lbl.className = 'atlas-group-label';
      lbl.textContent = grp.label;
      lbl.style.color = grp.color;
      header.append(master, dot, lbl);
      section.appendChild(header);

      // One sub-row per layer in the group.
      for (const id of ids) {
        const L = LAYERS[id];
        const row = document.createElement('div');
        row.className = 'atlas-row';

        const cb = document.createElement('input');
        cb.type = 'checkbox';
        cb.className = 'atlas-row-cb';
        cb.addEventListener('change', () => {
          api.setLayerVisible(id, cb.checked);
          ctx.onUserInteract?.();
          syncToggles();
        });
        rowCheckbox.set(id, cb);

        const sym = document.createElement('span');
        sym.className = 'atlas-row-sym';
        sym.textContent = L.symbol;
        sym.style.color = grp.color;

        const name = document.createElement('button');
        name.type = 'button';
        name.className = 'atlas-row-name';
        name.textContent = L.name;
        name.title = `${L.name} — ${L.tag}`;
        name.addEventListener('click', () => showDetail(id));

        row.append(cb, sym, name);
        section.appendChild(row);
      }
      panel.appendChild(section);
    }
  }

  // Reflect live layer visibility into every checkbox + group master state.
  function syncToggles() {
    for (const [id, cb] of rowCheckbox) {
      const L = layers.get(id);
      cb.checked = !!(L && L.root.visible);
    }
    for (const grp of GROUPS) {
      const ids = layerIdsByGroup.get(grp.id) || [];
      const on = ids.filter((id) => layers.get(id)?.root.visible).length;
      const master = groupCheckbox.get(grp.id);
      if (!master) continue;
      master.checked = on > 0 && on === ids.length;
      master.indeterminate = on > 0 && on < ids.length;
    }
  }

  // ── Right: detail panel ─────────────────────────────────────────────────
  let selectedKey = null;

  function showDetail(contentKey) {
    selectedKey = contentKey;
    const panel = dom.detailPanel;
    panel.innerHTML = '';
    panel.classList.add('atlas-detail');

    const body = document.createElement('div');
    body.className = 'atlas-detail-body';

    if (contentKey === 'lattice' || !LAYERS[contentKey]) {
      // The opening "substrate" card (chain stage 0).
      const h = document.createElement('div');
      h.className = 'atlas-detail-title';
      h.textContent = 'The substrate';
      const def = document.createElement('p');
      def.className = 'atlas-detail-def';
      def.textContent = 'A Moore-neighbourhood lattice (postulates P1–P5): discrete space, discrete time, ternary states, local causality, determinism.';
      const tag = document.createElement('div');
      tag.className = 'atlas-detail-tag';
      tag.textContent = '[AXIOM · P1–P5]';
      tag.style.color = 'var(--text-dim)';
      body.append(h, def, tag);
    } else {
      const L = LAYERS[contentKey];
      const color = groupColor.get(L.group) || 'var(--text)';
      const rgb = rgbTriplet(color);

      // symbol badge — filled at ~16% alpha, text in the group colour.
      const badge = document.createElement('div');
      badge.className = 'atlas-badge';
      badge.textContent = L.symbol;
      badge.style.background = `rgba(${rgb}, 0.16)`;
      badge.style.color = color;
      badge.style.borderColor = `rgba(${rgb}, 0.5)`;

      const title = document.createElement('div');
      title.className = 'atlas-detail-title';
      title.textContent = L.name;

      const def = document.createElement('p');
      def.className = 'atlas-detail-def';
      def.textContent = L.definition;

      const math = document.createElement('div');
      math.className = 'atlas-detail-math';
      math.textContent = L.math;

      const tag = document.createElement('div');
      tag.className = 'atlas-detail-tag';
      tag.textContent = L.tag;
      tag.style.color = color;

      const doc = document.createElement('div');
      doc.className = 'atlas-detail-doc';
      doc.textContent = L.doc;

      const head = document.createElement('div');
      head.className = 'atlas-detail-head';
      head.append(badge, title);
      body.append(head, def, math, tag, doc);

      // flows-to → neighbour names.
      if (Array.isArray(L.flowsTo) && L.flowsTo.length) {
        const flows = document.createElement('div');
        flows.className = 'atlas-detail-flows';
        const names = L.flowsTo.map((fid) => LAYERS[fid]?.name || fid).join(' · ');
        flows.textContent = `flows to → ${names}`;
        body.append(flows);
      }
    }

    panel.appendChild(body);

    // persistent honesty footer.
    const footer = document.createElement('div');
    footer.className = 'atlas-detail-footer';
    footer.textContent = 'Teaching diagram — tags are canonical (LEDGER). J is real; Ψ is bookkeeping; M is declined.';
    panel.appendChild(footer);
  }

  // ── Bottom: chain stepper ───────────────────────────────────────────────
  let playing = false;
  let playTimer = null;
  const PLAY_MS = 2200;
  const nodeEls = [];
  let playBtn = null;

  // Primary colour of a stage = the group of its last-added (newest) layer,
  // else neutral. We infer "newest" by diffing layersOn vs the previous stage.
  function stagePrimaryColor(i) {
    const cur = STAGES[i].layersOn;
    const prev = i > 0 ? STAGES[i - 1].layersOn : ['lattice'];
    const added = cur.filter((id) => !prev.includes(id) && id !== 'lattice');
    const newest = added[added.length - 1];
    const grp = newest && LAYERS[newest] ? LAYERS[newest].group : null;
    return grp ? (groupColor.get(grp) || 'var(--text-dim)') : 'var(--text-dim)';
  }

  function buildStepper() {
    const bar = dom.stepper;
    bar.innerHTML = '';
    bar.classList.add('atlas-stepper');

    playBtn = document.createElement('button');
    playBtn.type = 'button';
    playBtn.className = 'atlas-play';
    playBtn.setAttribute('aria-label', 'Play chain');
    playBtn.textContent = '▶';
    playBtn.addEventListener('click', () => (playing ? pause() : play()));
    bar.appendChild(playBtn);

    const track = document.createElement('div');
    track.className = 'atlas-track';
    nodeEls.length = 0;
    for (let i = 0; i < STAGES.length; i++) {
      const node = document.createElement('button');
      node.type = 'button';
      node.className = 'atlas-node';
      node.dataset.stage = String(i);
      node.title = `${i}. ${STAGES[i].title}`;
      const dot = document.createElement('span');
      dot.className = 'atlas-node-dot';
      dot.style.background = stagePrimaryColor(i);
      const cap = document.createElement('span');
      cap.className = 'atlas-node-cap';
      cap.textContent = STAGES[i].title;
      node.append(dot, cap);
      node.addEventListener('click', () => {
        pause();
        ctx.onUserInteract?.();
        api.setStage(i);
      });
      track.appendChild(node);
      nodeEls.push(node);
    }
    bar.appendChild(track);
  }

  // Mark stepper node n active (called by atlas-main's setStage).
  function markActive(n) {
    for (let i = 0; i < nodeEls.length; i++) {
      nodeEls[i].classList.toggle('active', i === n);
    }
    const active = nodeEls[n];
    if (active && active.scrollIntoView) {
      active.scrollIntoView({ block: 'nearest', inline: 'nearest' });
    }
  }

  function play() {
    if (playing) return;
    playing = true;
    if (playBtn) { playBtn.textContent = '❚❚'; playBtn.setAttribute('aria-label', 'Pause chain'); }
    playTimer = setInterval(() => {
      api.nextStage();
    }, PLAY_MS);
  }
  function pause() {
    playing = false;
    if (playBtn) { playBtn.textContent = '▶'; playBtn.setAttribute('aria-label', 'Play chain'); }
    if (playTimer) { clearInterval(playTimer); playTimer = null; }
  }

  // ── Mode switch (guided ⇆ free) ─────────────────────────────────────────
  let mode = 'guided';

  function setMode(next) {
    mode = next === 'free' ? 'free' : 'guided';
    for (const btn of dom.modeButtons) {
      const on = btn.dataset.mode === mode;
      btn.classList.toggle('active', on);
      btn.setAttribute('aria-selected', on ? 'true' : 'false');
    }
    // Re-apply the current stage so guided/free visibility rules take effect.
    api.setStage(api._stage ?? 0);
  }
  function getMode() { return mode; }

  for (const btn of dom.modeButtons) {
    btn.addEventListener('click', () => {
      pause();
      ctx.onUserInteract?.();
      setMode(btn.dataset.mode);
    });
  }

  // ── build everything ────────────────────────────────────────────────────
  buildLayerPanel();
  buildStepper();
  syncToggles();

  return { showDetail, syncToggles, setMode, getMode, markActive, selected: () => selectedKey, isPlaying: () => playing, pause };
}

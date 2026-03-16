/**
 * FTD Test Dashboard — Live streaming + static JSON modes.
 *
 * Live mode: connects to SSE at /api/run, shows real-time test execution.
 * Static mode: loads test_results.json for post-run viewing.
 */

// ── State ──────────────────────────────────────────────────────────────

let mode = "idle";           // "idle" | "live" | "static" | "done"
let tests = [];              // [{index, name, category, status, checks, lines, duration}]
let activeCategory = null;
let filterMode = "all";
let eventSource = null;
let liveStats = { total: 0, passed: 0, failed: 0, running: -1, progress: 0 };

// ── DOM refs ───────────────────────────────────────────────────────────

const sidebar = document.getElementById("sidebar");
const main = document.getElementById("main");
const btnRun = document.getElementById("btn-run");
const btnLoad = document.getElementById("btn-load");
const btnStop = document.getElementById("btn-stop");

// ── Init ───────────────────────────────────────────────────────────────

btnRun.addEventListener("click", startLiveRun);
btnLoad.addEventListener("click", loadStatic);
btnStop.addEventListener("click", forceStop);

// Auto-detect: try loading static JSON, show empty state if not found
loadStatic();

// ── Live mode: SSE connection ───────────────────────────────────────────

function startLiveRun() {
  if (mode === "live") return;

  // Reset state
  tests = [];
  liveStats = { total: 0, passed: 0, failed: 0, running: -1, progress: 0 };
  mode = "live";
  activeCategory = null;
  filterMode = "all";

  btnRun.textContent = "Running...";
  btnRun.classList.add("running");
  btnStop.style.display = "";

  eventSource = new EventSource("/api/run");

  eventSource.addEventListener("init", (e) => {
    const data = JSON.parse(e.data);
    liveStats.total = data.total;
    tests = data.tests.map(t => ({
      index: t.index,
      name: t.name,
      category: t.category,
      status: "pending",
      checks: [],
      lines: [],
      duration: null,
    }));
    renderAll();
  });

  eventSource.addEventListener("test_start", (e) => {
    const data = JSON.parse(e.data);
    liveStats.running = data.index;
    liveStats.progress = data.progress;
    const t = findTest(data.index);
    if (t) {
      t.status = "running";
      t.checks = [];
      t.lines = [];
    }
    renderAll();
    scrollToTest(data.index);
  });

  eventSource.addEventListener("check", (e) => {
    const data = JSON.parse(e.data);
    const t = findTest(data.index);
    if (t) {
      t.checks.push({ name: data.name, status: data.status });
      updateTestRow(data.index);
      appendCheckLine(data.index, data.name, data.status);
    }
  });

  eventSource.addEventListener("line", (e) => {
    const data = JSON.parse(e.data);
    const t = findTest(data.index);
    if (t) {
      t.lines.push(data.text);
      appendLiveLine(data.index, data.text);
    }
  });

  eventSource.addEventListener("test_end", (e) => {
    const data = JSON.parse(e.data);
    const t = findTest(data.index);
    if (t) {
      t.status = data.status;
      t.duration = data.duration;
    }
    if (data.status === "passed") liveStats.passed++;
    else liveStats.failed++;
    liveStats.progress++;
    updateTestRow(data.index);
    renderSidebar();
  });

  eventSource.addEventListener("done", (e) => {
    const data = JSON.parse(e.data);
    finishLiveRun();
  });

  eventSource.addEventListener("stopped", (e) => {
    const data = JSON.parse(e.data);
    // Mark remaining pending tests as stopped
    for (const t of tests) {
      if (t.status === "pending" || t.status === "running") {
        t.status = "stopped";
      }
    }
    finishLiveRun();
  });

  eventSource.onerror = () => {
    if (mode === "live") {
      finishLiveRun();
    }
  };
}

function finishLiveRun() {
  mode = "done";
  liveStats.running = -1;
  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }
  btnRun.textContent = "Run Live";
  btnRun.classList.remove("running");
  btnStop.style.display = "none";
  renderAll();
}

async function forceStop() {
  btnStop.textContent = "Stopping...";
  btnStop.style.pointerEvents = "none";
  try {
    await fetch("/api/stop", { method: "POST" });
  } catch (e) {
    // Server might be busy — the SSE connection closing will handle cleanup
  }
  // The "stopped" or "done" SSE event will call finishLiveRun()
  // But if the SSE connection is already dead, force cleanup after a short delay
  setTimeout(() => {
    if (mode === "live") {
      finishLiveRun();
    }
    btnStop.textContent = "Stop";
    btnStop.style.pointerEvents = "";
  }, 2000);
}

function findTest(index) {
  return tests.find(t => t.index === index);
}

// ── Static mode: load JSON ──────────────────────────────────────────────

async function loadStatic() {
  try {
    const resp = await fetch("test_results.json?" + Date.now());
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    const data = await resp.json();
    mode = "static";

    // Convert static data to test array
    tests = [];
    let idx = 0;
    for (const [cat, info] of Object.entries(data.categories)) {
      for (const t of info.tests) {
        idx++;
        tests.push({
          index: idx,
          name: t.name,
          category: cat,
          status: t.status,
          checks: t.checks || [],
          lines: (t.stdout || "").split("\n"),
          duration: t.duration_sec,
        });
      }
    }
    liveStats = {
      total: data.total,
      passed: data.passed,
      failed: data.failed,
      running: -1,
      progress: data.total,
    };
    renderAll();
  } catch (e) {
    if (mode !== "live" && mode !== "done") {
      mode = "idle";
      renderEmpty(e.message);
    }
  }
}

// ── Render orchestration ────────────────────────────────────────────────

function renderAll() {
  renderSidebar();
  renderMain();
}

function renderEmpty(reason) {
  sidebar.innerHTML = "";
  main.innerHTML = `
    <div class="empty-state">
      <h2>No Test Results</h2>
      <p>Click <strong>Run Live</strong> to execute tests with live streaming, or generate static results:</p>
      <code>python engine/run_tests_json.py</code>
      <p style="margin-top:8px;font-size:12px;color:var(--text-dim)">
        Live mode requires: <code style="display:inline;padding:2px 6px;margin:0">python engine/run_tests_live.py</code>
      </p>
    </div>`;
}

// ── Sidebar ─────────────────────────────────────────────────────────────

function renderSidebar() {
  const total = liveStats.total || tests.length;
  const passed = liveStats.passed;
  const failed = liveStats.failed;
  const done = passed + failed;
  const passRate = done > 0 ? (passed / done * 100) : 0;
  const isRunning = mode === "live";

  const barColor = failed > 0 ? "var(--negative)" : "var(--positive)";
  const pctPass = total > 0 ? (passed / total * 100) : 0;
  const pctFail = total > 0 ? (failed / total * 100) : 0;
  const pctActive = isRunning && total > 0 ? (1 / total * 100) : 0;

  let html = `
    <div class="summary-cards">
      <div class="summary-card">
        <div class="value" style="color:var(--positive)">${passed}</div>
        <div class="label">Passed</div>
      </div>
      <div class="summary-card">
        <div class="value" style="color:${failed > 0 ? 'var(--negative)' : 'var(--text-dim)'}">${failed}</div>
        <div class="label">Failed</div>
      </div>
      <div class="summary-card">
        <div class="value">${total}</div>
        <div class="label">Total</div>
      </div>
      <div class="summary-card">
        <div class="value">${done > 0 ? passRate.toFixed(1) + '%' : '--'}</div>
        <div class="label">Pass Rate</div>
      </div>
    </div>

    <div class="progress-bar">
      <div class="fill-pass" style="width:${pctPass}%"></div>
      <div class="fill-fail" style="left:${pctPass}%;width:${pctFail}%"></div>
      ${isRunning ? `<div class="fill-active" style="left:${pctPass + pctFail}%;width:${pctActive}%"></div>` : ''}
    </div>
    <div class="progress-text">
      <span>${done}/${total} ${isRunning ? 'running' : 'complete'}</span>
      <span>${isRunning ? '...' : (done > 0 ? passRate.toFixed(1) + '% pass' : '')}</span>
    </div>`;

  // Category summary
  const cats = buildCategoryMap();
  html += `<h3>Categories</h3><div class="cat-list">`;
  html += `<div class="cat-item ${activeCategory === null ? 'active' : ''}" data-cat="">
    <span class="cat-dot" style="background:var(--accent)"></span>
    <span>All Tests</span>
    <span class="cat-count">${passed}/${total}</span>
  </div>`;

  for (const [cat, info] of Object.entries(cats)) {
    const dotColor = info.failed > 0 ? "var(--negative)"
      : info.running ? "var(--accent)"
      : info.passed > 0 ? "var(--positive)"
      : "var(--text-dim)";
    html += `<div class="cat-item ${activeCategory === cat ? 'active' : ''}" data-cat="${esc(cat)}">
      <span class="cat-dot" style="background:${dotColor}"></span>
      <span>${esc(cat)}</span>
      <span class="cat-count">${info.passed}/${info.total}</span>
    </div>`;
  }
  html += `</div>`;

  sidebar.innerHTML = html;

  sidebar.querySelectorAll(".cat-item").forEach(el => {
    el.addEventListener("click", () => {
      activeCategory = el.dataset.cat || null;
      renderAll();
    });
  });
}

function buildCategoryMap() {
  const cats = {};
  for (const t of tests) {
    if (!cats[t.category]) cats[t.category] = { passed: 0, failed: 0, running: false, total: 0 };
    cats[t.category].total++;
    if (t.status === "passed") cats[t.category].passed++;
    else if (t.status === "failed") cats[t.category].failed++;
    else if (t.status === "running") cats[t.category].running = true;
  }
  return cats;
}

// ── Main content ────────────────────────────────────────────────────────

function renderMain() {
  const filtered = getFilteredTests();

  let html = `
    <div class="toolbar">
      <div class="filter-group">
        <button class="btn ${filterMode === 'all' ? 'active' : ''}" data-filter="all">All</button>
        <button class="btn ${filterMode === 'failed' ? 'active' : ''}" data-filter="failed">Failed</button>
        <button class="btn ${filterMode === 'passed' ? 'active' : ''}" data-filter="passed">Passed</button>
      </div>
    </div>
    <div class="test-list" id="test-list">`;

  for (const t of filtered) {
    html += renderTestRow(t);
  }

  html += `</div>`;
  main.innerHTML = html;

  // Wire filter buttons
  main.querySelectorAll("[data-filter]").forEach(btn => {
    btn.addEventListener("click", () => {
      filterMode = btn.dataset.filter;
      renderMain();
    });
  });

  // Wire expand/collapse
  wireTestClicks();
}

function getFilteredTests() {
  let list = tests;
  if (activeCategory) {
    list = list.filter(t => t.category === activeCategory);
  }
  if (filterMode === "failed") {
    list = list.filter(t => t.status === "failed");
  } else if (filterMode === "passed") {
    list = list.filter(t => t.status === "passed");
  }
  return list;
}

function renderTestRow(t) {
  const iconClass = t.status === "passed" ? "pass"
    : t.status === "failed" ? "fail"
    : t.status === "running" ? "running"
    : t.status === "stopped" ? "skipped"
    : "pending";
  const iconChar = t.status === "passed" ? "\u2713"
    : t.status === "failed" ? "\u2717"
    : t.status === "running" ? "\u25B6"
    : t.status === "stopped" ? "\u25A0"
    : "\u25CB";
  const checkSummary = t.checks.length > 0
    ? `${t.checks.filter(c => c.status === "pass").length}/${t.checks.length}`
    : "";
  const durText = t.duration != null ? `${t.duration.toFixed(2)}s` : "";
  const isActive = t.status === "running";
  const isOpen = isActive; // auto-expand running test

  return `
    <div class="test-row ${isActive ? 'active-test' : ''}" id="test-${t.index}" data-index="${t.index}">
      <div class="test-header">
        <span class="test-num">${t.index}.</span>
        <div class="test-icon ${iconClass}">${iconChar}</div>
        <div class="test-name">${esc(t.name)}<span class="cat-tag">${esc(t.category)}</span></div>
        <div class="test-checks">${checkSummary}</div>
        <div class="test-duration">${durText}</div>
      </div>
      <div class="test-details ${isOpen ? 'open' : ''}" id="details-${t.index}">
        <div class="live-output" id="output-${t.index}">${renderExistingOutput(t)}</div>
      </div>
    </div>`;
}

function renderExistingOutput(t) {
  let html = "";
  for (const c of t.checks) {
    const cls = c.status === "pass" ? "hl-pass" : "hl-fail";
    const label = c.status === "pass" ? "PASS" : "FAIL";
    html += `<span class="${cls}">  ${label}  ${esc(c.name)}</span>\n`;
  }
  // Only show raw lines if no checks (avoids duplication)
  if (t.checks.length === 0 && t.lines.length > 0) {
    for (const line of t.lines) {
      html += highlightLine(line) + "\n";
    }
  }
  return html;
}

function highlightLine(text) {
  const escaped = esc(text);
  if (/^\s*PASS\s{2}/.test(text)) return `<span class="hl-pass">${escaped}</span>`;
  if (/^\s*FAIL\s{2}/.test(text)) return `<span class="hl-fail">${escaped}</span>`;
  if (/^=+\s.*\s=+$/.test(text)) return `<span class="hl-section">${escaped}</span>`;
  return escaped;
}

// ── Incremental live updates (avoid full re-render) ─────────────────────

function updateTestRow(index) {
  const t = findTest(index);
  if (!t) return;

  const row = document.getElementById(`test-${index}`);
  if (!row) return;

  // Update icon
  const icon = row.querySelector(".test-icon");
  if (icon) {
    icon.className = "test-icon " + (t.status === "passed" ? "pass"
      : t.status === "failed" ? "fail"
      : t.status === "running" ? "running" : "pending");
    icon.textContent = t.status === "passed" ? "\u2713"
      : t.status === "failed" ? "\u2717"
      : t.status === "running" ? "\u25B6" : "\u25CB";
  }

  // Update checks count
  const checksEl = row.querySelector(".test-checks");
  if (checksEl && t.checks.length > 0) {
    checksEl.textContent = `${t.checks.filter(c => c.status === "pass").length}/${t.checks.length}`;
  }

  // Update duration
  const durEl = row.querySelector(".test-duration");
  if (durEl && t.duration != null) {
    durEl.textContent = `${t.duration.toFixed(2)}s`;
  }

  // Update active state
  row.classList.toggle("active-test", t.status === "running");

  // Collapse details when test completes (unless failed)
  if (t.status === "passed") {
    const details = document.getElementById(`details-${index}`);
    if (details) details.classList.remove("open");
  }
}

function appendCheckLine(index, name, status) {
  const output = document.getElementById(`output-${index}`);
  if (!output) return;

  const details = document.getElementById(`details-${index}`);
  if (details && !details.classList.contains("open")) {
    details.classList.add("open");
  }

  const cls = status === "pass" ? "hl-pass" : "hl-fail";
  const label = status === "pass" ? "PASS" : "FAIL";
  output.insertAdjacentHTML("beforeend", `<span class="${cls}">  ${label}  ${esc(name)}</span>\n`);

  if (details) details.scrollTop = details.scrollHeight;
}

function appendLiveLine(index, text) {
  const output = document.getElementById(`output-${index}`);
  if (!output) return;

  const details = document.getElementById(`details-${index}`);
  if (details && !details.classList.contains("open")) {
    details.classList.add("open");
  }

  // Check if this is a PASS/FAIL line (already handled by check event)
  if (/^\s{2,}(PASS|FAIL)\s{2}/.test(text)) {
    // Rendered by check event instead
    return;
  }

  const highlighted = highlightLine(text);
  output.insertAdjacentHTML("beforeend", highlighted + "\n");

  // Auto-scroll the output panel
  details.scrollTop = details.scrollHeight;
}

function scrollToTest(index) {
  const row = document.getElementById(`test-${index}`);
  if (row) {
    row.scrollIntoView({ behavior: "smooth", block: "center" });
  }
}

function wireTestClicks() {
  main.querySelectorAll(".test-header").forEach(header => {
    header.addEventListener("click", () => {
      const row = header.closest(".test-row");
      const index = row.dataset.index;
      const details = document.getElementById(`details-${index}`);
      if (details) details.classList.toggle("open");
    });
  });
}

// ── Util ────────────────────────────────────────────────────────────────

function esc(s) {
  if (s == null) return "";
  return String(s)
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;");
}

/**
 * GPU Acceleration splash card (local dev only).
 *
 * Talks to serve.py's /api/gpu-server/{status,start,download,stop-all} routes
 * and /api/idle-policy so the user can start or download the CUDA WebSocket
 * server (ws_server.exe), stop every engine server on the machine, and set
 * the dev server's idle auto-stop policy straight from the dashboard splash
 * instead of the terminal. On GitHub Pages — or any host without serve.py —
 * those routes are absent, so the card stays hidden (and, if the user opens
 * it explicitly, explains how to enable it locally).
 *
 * The idle-policy and stop-all routes were added after /status/start/download
 * (spec docs/superpowers/specs/2026-09-16-idle-shutdown-and-kill-all.md); an
 * older dev server may still be running without them. Those two controls
 * degrade independently and honestly (see idleApiUnavailable/stopAllApiUnavailable
 * below) instead of failing silently when they 404.
 *
 * Deliberately a CLASSIC (non-module) script with no imports so it runs during
 * the splash, before the ES-module app boot. Its idle-settings clamp/storage
 * logic is therefore a small inline copy of
 * `engine/web/js/lib/idle-policy-settings.js` (the canonical, Node-tested
 * definition) — keep the two in sync when either changes.
 */
(function () {
  'use strict';

  var API = '/api/gpu-server';
  var DISMISS_KEY = 'ftd-gpu-card-dismissed';

  // ── Idle-policy settings (inline twin of js/lib/idle-policy-settings.js) ──
  var IDLE_MINUTES_MIN = 1;
  var IDLE_MINUTES_MAX = 1440;
  var IDLE_MINUTES_DEFAULT = 30;
  var IDLE_ENABLED_KEY = 'ftd-gpu-idle-enabled';
  var IDLE_MINUTES_KEY = 'ftd-gpu-idle-minutes';

  // Once either new route 404s, stop asking — a restarted-without-rebuild
  // page keeps re-probing, but within one page life we take the answer once.
  var idleApiUnavailable = false;
  var stopAllApiUnavailable = false;

  function clampIdleMinutes(value) {
    var n = Math.trunc(Number(value));
    if (!isFinite(n)) return IDLE_MINUTES_DEFAULT;
    return Math.min(IDLE_MINUTES_MAX, Math.max(IDLE_MINUTES_MIN, n));
  }
  function readIdleSettings() {
    var enabled = true, minutes = IDLE_MINUTES_DEFAULT;
    try {
      var e = localStorage.getItem(IDLE_ENABLED_KEY);
      if (e !== null) enabled = e === '1';
      var m = localStorage.getItem(IDLE_MINUTES_KEY);
      if (m !== null) minutes = clampIdleMinutes(m);
    } catch (ex) { /* storage unavailable — defaults stand */ }
    return { enabled: enabled, minutes: minutes };
  }
  function writeIdleSettings(settings) {
    var normalized = { enabled: !!settings.enabled, minutes: clampIdleMinutes(settings.minutes) };
    try {
      localStorage.setItem(IDLE_ENABLED_KEY, normalized.enabled ? '1' : '0');
      localStorage.setItem(IDLE_MINUTES_KEY, String(normalized.minutes));
    } catch (ex) { /* best-effort only */ }
    return normalized;
  }

  function isDismissed() {
    try { return localStorage.getItem(DISMISS_KEY) === '1'; } catch (e) { return false; }
  }
  function setDismissed(on) {
    try { on ? localStorage.setItem(DISMISS_KEY, '1') : localStorage.removeItem(DISMISS_KEY); } catch (e) {}
  }
  function fmtSize(b) {
    if (!b) return '';
    return b >= 1048576 ? (b / 1048576).toFixed(1) + ' MB' : Math.round(b / 1024) + ' KB';
  }
  function getStatus() {
    return fetch(API + '/status', { cache: 'no-store' })
      .then(function (r) { return r.ok ? r.json() : null; })
      .catch(function () { return null; });
  }
  function policyWords(idle) {
    return idle.enabled ? ('stops after ' + idle.minutes + ' min idle') : 'auto-stop off';
  }

  function init() {
    var card = document.getElementById('gpu-server-card');
    if (!card) return;
    var bodyEl = document.getElementById('gpu-card-body');
    var statusEl = document.getElementById('gpu-card-status');
    var dotEl = document.getElementById('gpu-card-dot');
    var closeBtn = document.getElementById('gpu-card-close');

    function show() { card.hidden = false; }
    function hide() { card.hidden = true; }
    function setDot(cls) { dotEl.className = 'gpu-card-dot' + (cls ? ' ' + cls : ''); }

    // ── Idle-policy + stop-all controls, shared by the running/not-running
    // branches of render() below. Rendered as long as the gpu-server API
    // exists at all (s !== null); the two sub-features degrade on their own
    // if their specific route 404s (older dev server, hook not yet
    // committed — see the module header).
    function idleControlsHTML(idle) {
      var toggleDisabled = idleApiUnavailable ? ' disabled' : '';
      var stopAllDisabled = stopAllApiUnavailable ? ' disabled' : '';
      var html = ''
        + '<div class="gpu-row gpu-idle-row">'
        + '<label class="gpu-idle-toggle"><input type="checkbox" id="gpu-idle-enabled"'
        + (idle.enabled ? ' checked' : '') + toggleDisabled + '> Stop servers when idle</label>'
        + '<label class="gpu-idle-minutes-lbl">after <input type="number" id="gpu-idle-minutes" '
        + 'min="' + IDLE_MINUTES_MIN + '" max="' + IDLE_MINUTES_MAX + '" value="' + idle.minutes + '"'
        + (idle.enabled ? '' : ' disabled') + toggleDisabled + '> min</label>'
        + '</div>'
        + '<div class="gpu-note gpu-idle-hint">A change here applies to servers started afterwards, '
        + 'not to one already running.</div>'
        + '<div class="gpu-idle-msg" id="gpu-idle-msg"' + (idleApiUnavailable ? '' : ' hidden') + '>'
        + (idleApiUnavailable
          ? 'Dev-server idle controls are unavailable — restart the dev server to pick up this feature.'
          : '') + '</div>'
        + '<div class="gpu-row"><button id="gpu-stop-all" class="gpu-btn danger"' + stopAllDisabled + '>'
        + 'Stop all engine servers</button></div>'
        + '<div class="gpu-stopall-msg" id="gpu-stopall-msg"' + (stopAllApiUnavailable ? '' : ' hidden') + '>'
        + (stopAllApiUnavailable
          ? 'Stop-all is unavailable — restart the dev server to pick up this feature.'
          : '') + '</div>';
      return html;
    }

    function pushIdlePolicy(idle) {
      if (idleApiUnavailable) return;
      fetch('/api/idle-policy', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ enabled: idle.enabled, minutes: idle.minutes }),
      }).then(function (r) {
        if (r.status === 404) {
          idleApiUnavailable = true;
          var msg = document.getElementById('gpu-idle-msg');
          if (msg) {
            msg.hidden = false;
            msg.textContent = 'Dev-server idle controls are unavailable — restart the dev server to pick up this feature.';
          }
        }
      }).catch(function () { /* transient network error — keep the optimistic local setting */ });
    }

    function wireIdleControls() {
      var enabledEl = document.getElementById('gpu-idle-enabled');
      var minutesEl = document.getElementById('gpu-idle-minutes');
      var stopAllBtn = document.getElementById('gpu-stop-all');
      var stopAllMsg = document.getElementById('gpu-stopall-msg');

      if (enabledEl && !idleApiUnavailable) {
        enabledEl.onchange = function () {
          var current = readIdleSettings();
          var next = writeIdleSettings({ enabled: enabledEl.checked, minutes: current.minutes });
          if (minutesEl) minutesEl.disabled = !next.enabled;
          if (statusEl && statusEl.dataset.running === '1') statusEl.textContent = statusEl.dataset.port
            ? ('running · :' + statusEl.dataset.port + ' · ' + policyWords(next)) : statusEl.textContent;
          pushIdlePolicy(next);
        };
      }
      if (minutesEl && !idleApiUnavailable) {
        minutesEl.onchange = function () {
          var current = readIdleSettings();
          var next = writeIdleSettings({ enabled: current.enabled, minutes: minutesEl.value });
          minutesEl.value = next.minutes;
          if (statusEl && statusEl.dataset.running === '1') statusEl.textContent = statusEl.dataset.port
            ? ('running · :' + statusEl.dataset.port + ' · ' + policyWords(next)) : statusEl.textContent;
          pushIdlePolicy(next);
        };
      }
      if (stopAllBtn && !stopAllApiUnavailable) {
        stopAllBtn.onclick = function () {
          var confirmed = window.confirm(
            'Stop every ws_server.exe engine server on this machine, including ones started outside this dashboard?');
          if (!confirmed) return;
          stopAllBtn.disabled = true;
          fetch(API + '/stop-all', { method: 'POST' }).then(function (r) {
            if (r.status === 404) { stopAllApiUnavailable = true; return null; }
            return r.json();
          }).then(function (j) {
            stopAllBtn.disabled = false;
            if (j === null) {
              if (stopAllMsg) {
                stopAllMsg.hidden = false;
                stopAllMsg.textContent = 'Stop-all is unavailable — restart the dev server to pick up this feature.';
              }
              return;
            }
            getStatus().then(function (s) { render(s, true); });
          }).catch(function () {
            stopAllBtn.disabled = false;
          });
        };
      }
    }

    function render(s, fromClick) {
      // No launcher API → not served by serve.py (e.g. GitHub Pages).
      if (s === null) {
        if (!fromClick) { hide(); return; }
        setDot('off');
        statusEl.textContent = 'unavailable';
        bodyEl.innerHTML = '<div class="gpu-note">GPU acceleration is available when you run the dashboard '
          + 'locally: <code>python engine/web/serve.py</code>.</div>';
        show();
        return;
      }
      var idle = readIdleSettings();
      if (s.running) {
        setDot('on');
        statusEl.dataset.running = '1';
        statusEl.dataset.port = String(s.port);
        statusEl.textContent = 'running · :' + s.port + ' · ' + policyWords(idle);
        bodyEl.innerHTML = '<div class="gpu-note">GPU engine is live — the dashboard uses it automatically.</div>'
          + '<div class="gpu-row"><button id="gpu-reload" class="gpu-btn primary">Reload &amp; connect</button></div>'
          + idleControlsHTML(idle);
        var rl = document.getElementById('gpu-reload');
        if (rl) rl.onclick = function () { window.location.reload(); };
        wireIdleControls();
        if (fromClick) show(); else hide();  // don't nag an already-GPU session on load
        return;
      }
      statusEl.dataset.running = '0';
      setDot('off');
      statusEl.textContent = 'not running';
      if (!s.exeExists) {
        bodyEl.innerHTML = '<div class="gpu-note">The GPU server (<code>ws_server.exe</code>) isn’t built yet. '
          + 'Build it with <code>engine\\build_native.bat</code>, then reload.</div>'
          + idleControlsHTML(idle);
        wireIdleControls();
        show();
        return;
      }
      bodyEl.innerHTML =
        '<div class="gpu-note">Run the simulation on your GPU (CUDA) for large lattices.</div>'
        + '<div class="gpu-row"><label class="gpu-lattice-lbl">Lattice '
        + '<select id="gpu-lattice">'
        + '<option value="0">default (32)</option>'
        + '<option value="65">65</option>'
        + '<option value="97">97</option>'
        + '<option value="129">129</option>'
        + '<option value="181">181</option>'
        + '</select></label></div>'
        + '<div class="gpu-row">'
        + '<button id="gpu-start" class="gpu-btn primary">Start GPU Server</button>'
        + '<a class="gpu-btn" href="' + API + '/download" download="ws_server.exe">Download exe'
        + (s.exeSize ? ' · ' + fmtSize(s.exeSize) : '') + '</a>'
        + '</div>'
        + '<div class="gpu-msg" id="gpu-msg"></div>'
        + idleControlsHTML(idle);
      document.getElementById('gpu-start').onclick = onStart;
      wireIdleControls();
      show();
    }

    function onStart() {
      var sel = document.getElementById('gpu-lattice');
      var lattice = sel ? (parseInt(sel.value, 10) || 0) : 0;
      var idle = readIdleSettings();
      var msg = document.getElementById('gpu-msg');
      var btn = document.getElementById('gpu-start');
      btn.disabled = true;
      msg.textContent = 'Launching GPU server…';
      fetch(API + '/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        // idleMinutes/autoStop are sent with every start request (spec §4) so
        // the launched server picks up this tab's idle-policy setting.
        body: JSON.stringify({ lattice: lattice, idleMinutes: idle.minutes, autoStop: idle.enabled }),
      }).then(function (r) { return r.json(); }).then(function (j) {
        if (j && j.error) { msg.textContent = 'Error: ' + j.error; btn.disabled = false; return; }
        msg.textContent = 'Started' + (j && j.pid ? ' (pid ' + j.pid + ')' : '') + ' — waiting for it to come online…';
        var tries = 0;
        var poll = setInterval(function () {
          tries++;
          getStatus().then(function (s) {
            if (s && s.running) {
              clearInterval(poll);
              msg.textContent = 'GPU server online — connecting…';
              setTimeout(function () { window.location.reload(); }, 800);
            } else if (tries > 40) {  // ~20s
              clearInterval(poll);
              msg.textContent = 'Did not come online in time — check the terminal.';
              btn.disabled = false;
            }
          });
        }, 500);
      }).catch(function (e) {
        msg.textContent = 'Error: ' + e.message; btn.disabled = false;
      });
    }

    if (closeBtn) closeBtn.onclick = function () { setDismissed(true); hide(); };

    // The compute-status chip reopens the card on demand, any time after boot.
    // bridge-boot.js already gives it the "Start ws_server.exe for GPU" title on
    // the WASM path, so we only add the cursor + click behaviour here.
    var compute = document.getElementById('status-compute');
    if (compute) {
      compute.style.cursor = 'pointer';
      compute.addEventListener('click', function () {
        setDismissed(false);
        getStatus().then(function (s) { render(s, true); });
      });
    }

    // On load, auto-surface the card only when there is something to do (GPU not
    // running but launchable) and the user hasn't dismissed it before.
    getStatus().then(function (s) {
      if (isDismissed()) { hide(); return; }
      render(s, false);
    });
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init);
  else init();
})();

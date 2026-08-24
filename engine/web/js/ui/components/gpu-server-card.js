/**
 * GPU Acceleration splash card (local dev only).
 *
 * Talks to serve.py's /api/gpu-server/{status,start,download} routes so the user
 * can start or download the CUDA WebSocket server (ws_server.exe) straight from
 * the dashboard splash instead of the terminal. On GitHub Pages — or any host
 * without serve.py — those routes are absent, so the card stays hidden (and, if
 * the user opens it explicitly, explains how to enable it locally).
 *
 * Deliberately a CLASSIC (non-module) script with no imports so it runs during
 * the splash, before the ES-module app boot.
 */
(function () {
  'use strict';

  var API = '/api/gpu-server';
  var DISMISS_KEY = 'ftd-gpu-card-dismissed';

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
      if (s.running) {
        setDot('on');
        statusEl.textContent = 'running · :' + s.port;
        bodyEl.innerHTML = '<div class="gpu-note">GPU engine is live — the dashboard uses it automatically.</div>'
          + '<div class="gpu-row"><button id="gpu-reload" class="gpu-btn primary">Reload &amp; connect</button></div>';
        var rl = document.getElementById('gpu-reload');
        if (rl) rl.onclick = function () { window.location.reload(); };
        if (fromClick) show(); else hide();  // don't nag an already-GPU session on load
        return;
      }
      setDot('off');
      statusEl.textContent = 'not running';
      if (!s.exeExists) {
        bodyEl.innerHTML = '<div class="gpu-note">The GPU server (<code>ws_server.exe</code>) isn’t built yet. '
          + 'Build it with <code>engine\\build_native.bat</code>, then reload.</div>';
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
        + '<div class="gpu-msg" id="gpu-msg"></div>';
      document.getElementById('gpu-start').onclick = onStart;
      show();
    }

    function onStart() {
      var sel = document.getElementById('gpu-lattice');
      var lattice = sel ? (parseInt(sel.value, 10) || 0) : 0;
      var msg = document.getElementById('gpu-msg');
      var btn = document.getElementById('gpu-start');
      btn.disabled = true;
      msg.textContent = 'Launching GPU server…';
      fetch(API + '/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ lattice: lattice }),
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

/**
 * app-wire/status.js — toast + loading-bar helpers for the dashboard.
 * Extracted from app.js (behavior-preserving).
 */

export function showToast(msg, severity = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const toast = document.createElement('div');
    toast.className = `toast toast-${severity}`;
    const span = document.createElement('span');
    span.textContent = msg;
    const btn = document.createElement('button');
    btn.textContent = '\u00d7';
    btn.addEventListener('click', () => toast.remove());
    toast.appendChild(span);
    toast.appendChild(btn);
    container.appendChild(toast);
    setTimeout(() => { if (toast.parentElement) toast.remove(); }, 8000);
}

export function loadProgress(pct, msg) {
    const bar = document.getElementById('load-bar');
    const status = document.getElementById('load-status');
    if (bar) bar.style.width = pct + '%';
    if (status) status.textContent = msg;
}

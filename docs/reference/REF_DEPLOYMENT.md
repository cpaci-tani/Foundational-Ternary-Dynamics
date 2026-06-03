# FTD Web Deployment Guide

Deploy the FTD interactive web experience via FTP to any static hosting server.

---

## What to Upload

| Source Directory | Deploy To | Size | Description |
|-----------------|-----------|------|-------------|
| `engine/web/` | `/ftd/` (root) | ~1.1 MB | WASM simulation dashboard |
| `dissemination/interactive/` | `/ftd/interactive/` | ~176 KB | 5 standalone force simulations |
| `dissemination/manuscript/_webbook/` | `/ftd/book/` | ~12 MB | 119-chapter HTML textbook |
| `dissemination/whitepaper/FTD_Whitepaper.pdf` | `/ftd/whitepaper/` | ~652 KB | Academic whitepaper |

**Total: ~14 MB**

---

## Recommended Directory Structure

```
public_html/ftd/
├── index.html                  ← engine/web/index.html
├── js/                         ← engine/web/js/ (28 ES6 modules)
├── wasm/                       ← engine/web/wasm/ (ftd_core.js + ftd_core.wasm)
├── .htaccess                   ← engine/web/.htaccess (MIME types)
├── interactive/                ← dissemination/interactive/
│   ├── gravity_simulation.html
│   ├── electromagnetic_simulation.html
│   ├── strong_force_simulation.html
│   ├── weak_force_simulation.html
│   └── unified_forces_simulation.html
├── book/                       ← dissemination/manuscript/_webbook/
│   ├── index.html
│   ├── chapters/
│   ├── site_libs/
│   └── ...
└── whitepaper/
    └── FTD_Whitepaper.pdf
```

---

## Server Requirements

### MIME Types (Critical)

The `.wasm` file **must** be served with the correct MIME type or the browser will reject it.

**Apache** (`.htaccess` included in `engine/web/`):
```apache
AddType application/wasm .wasm
```

**Nginx** (add to `mime.types` or server block):
```nginx
types {
    application/wasm wasm;
}
```

**IIS** (`web.config`):
```xml
<configuration>
  <system.webServer>
    <staticContent>
      <mimeMap fileExtension=".wasm" mimeType="application/wasm" />
    </staticContent>
  </system.webServer>
</configuration>
```

### CDN Dependencies

The following resources load from CDNs (internet access required):

| Resource | CDN | Used By |
|----------|-----|---------|
| Three.js v0.169.0 | esm.sh | Dashboard (engine/web) |
| p5.js v1.7.0 | CloudFlare | Interactive simulations |
| KaTeX v0.16.9 | jsDelivr | Interactive simulations |
| MathJax v3 | jsDelivr | Webbook (manuscript) |
| Google Fonts (Inter, JetBrains Mono) | Google | Dashboard |

If your server has no internet access, these must be bundled locally.

---

## FTP Upload Steps

1. **Connect** to your FTP server and navigate to the web root (e.g., `public_html/`)

2. **Create** the `/ftd/` directory

3. **Upload** in this order:
   ```
   engine/web/.htaccess          → /ftd/.htaccess
   engine/web/index.html         → /ftd/index.html
   engine/web/js/                → /ftd/js/         (entire directory)
   engine/web/wasm/              → /ftd/wasm/       (entire directory)
   dissemination/interactive/    → /ftd/interactive/ (5 HTML files)
   dissemination/manuscript/_webbook/ → /ftd/book/   (entire directory)
   dissemination/whitepaper/FTD_Whitepaper.pdf → /ftd/whitepaper/FTD_Whitepaper.pdf
   ```

4. **Verify** file transfer mode: `.wasm` files must be transferred in **binary mode** (not ASCII)

---

## Post-Upload Checklist

After uploading, verify in a browser:

- [ ] `/ftd/index.html` loads the simulation dashboard
- [ ] Dashboard shows "WASM loaded" (or falls back to MockBridge gracefully)
- [ ] Scale 0 scenarios run (particles appear, energy chart updates)
- [ ] Scale 1 hydrogen scenario shows orbital motion
- [ ] `/ftd/interactive/gravity_simulation.html` renders with p5.js canvas
- [ ] `/ftd/interactive/electromagnetic_simulation.html` renders
- [ ] `/ftd/book/index.html` loads the webbook with navigation
- [ ] `/ftd/book/chapters/1.0-before-the-void.html` loads with MathJax equations
- [ ] `/ftd/whitepaper/FTD_Whitepaper.pdf` downloads correctly

### Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| WASM fails to load | Wrong MIME type | Add `.htaccess` or configure server MIME types |
| Blank dashboard | JS module import error | Check browser console; ensure `js/` directory uploaded |
| p5.js simulations blank | CDN blocked | Check network access to `cdnjs.cloudflare.com` |
| Webbook unstyled | Missing `site_libs/` | Ensure entire `_webbook/site_libs/` was uploaded |
| Math equations missing | MathJax CDN blocked | Check network access to `cdn.jsdelivr.net` |

---

## MockBridge Fallback

If WASM fails to load (wrong MIME type, unsupported browser, etc.), the dashboard automatically falls back to a JavaScript MockBridge that provides:
- Scale 0: Zeroed diagnostic data (no simulation)
- Scale 1: Full JavaScript Velocity Verlet physics (particle scenarios work)
- Scale 2: Full JavaScript atom engine (molecule scenarios work)

The interactive simulations (`/ftd/interactive/`) are pure JavaScript and have no WASM dependency.

---

## Optional: Landing Page

To create a portal linking all sections, add an `index.html` at `/ftd/` root level that links to:
- `/ftd/index.html` (or rename dashboard to `dashboard.html`) — Simulation Dashboard
- `/ftd/interactive/` — Force Simulations
- `/ftd/book/` — Online Textbook
- `/ftd/whitepaper/FTD_Whitepaper.pdf` — Academic Paper

The current `engine/web/index.html` serves as both the dashboard and de-facto landing page.

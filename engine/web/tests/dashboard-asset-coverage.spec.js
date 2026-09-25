// @ts-check
import { test, expect } from '@playwright/test';
import { createServer } from 'node:http';
import { existsSync } from 'node:fs';
import { readFile, readdir } from 'node:fs/promises';
import { extname, join, relative, resolve, sep } from 'node:path';
import { fileURLToPath } from 'node:url';

const webRoot = fileURLToPath(new URL('../', import.meta.url));
const distRoot = resolve(webRoot, '../../dist');
const lazySheets = new Map([
    ['css/atlas.css', 'fields-atlas.html'],
    ['css/ui/components/jev-console.css', 'js/assistant/console.js'],
    ['css/ui/components/fluid-panel.css', 'js/scales/scale0/ui/overlays/fluid-panel.js'],
    ['css/ui/components/time-panel.css', 'js/scales/scale0/ui/overlays/time-panel.js'],
    ['css/ui/components/gravity-panel.css', 'js/scales/scale0/ui/overlays/gravity-panel.js'],
]);

async function listCss(directory) {
    const files = [];
    for (const entry of await readdir(directory, { withFileTypes: true })) {
        const path = join(directory, entry.name);
        if (entry.isDirectory()) files.push(...await listCss(path));
        else if (entry.name.endsWith('.css')) files.push(relative(webRoot, path).replaceAll('\\', '/'));
    }
    return files;
}

function localPath(href, base) {
    const url = new URL(href, base);
    return url.origin === new URL(base).origin ? decodeURIComponent(url.pathname).replace(/^\//, '') : null;
}

async function readStyles(page) {
    return page.evaluate(() => ({
        links: Array.from(document.querySelectorAll('head link[rel="stylesheet"]'), (link) => link.getAttribute('href')),
        deferred: window.FTD_DEFER_CSS.map((spec) => spec.href),
    }));
}

async function startDistServer() {
    const types = { '.html': 'text/html', '.css': 'text/css', '.js': 'text/javascript', '.json': 'application/json', '.wasm': 'application/wasm' };
    const server = createServer(async (request, response) => {
        const url = new URL(request.url, 'http://localhost');
        const target = resolve(distRoot, `.${url.pathname === '/' ? '/index.html' : decodeURIComponent(url.pathname)}`);
        if (!target.startsWith(distRoot + sep)) {
            response.writeHead(403).end();
            return;
        }
        try {
            const body = await readFile(target);
            response.setHeader('Content-Type', types[extname(target)] || 'application/octet-stream');
            response.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
            response.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
            response.writeHead(200).end(body);
        } catch (_err) {
            response.writeHead(404).end();
        }
    });
    await new Promise((resolveListen) => server.listen(0, '127.0.0.1', resolveListen));
    return { server, origin: `http://127.0.0.1:${server.address().port}` };
}

test('direct-serving stylesheet lists cover every non-lazy CSS asset', async ({ page }) => {
    await page.goto('/', { waitUntil: 'domcontentloaded' });
    const styles = await readStyles(page);
    const listed = new Set([...styles.links, ...styles.deferred]
        .map((href) => localPath(href, page.url())).filter(Boolean));
    const cssFiles = await listCss(join(webRoot, 'css'));
    expect(cssFiles.filter((path) => !listed.has(path) && !lazySheets.has(path))).toEqual([]);
    for (const [path, owner] of lazySheets) {
        expect(cssFiles).toContain(path);
        expect(await readFile(join(webRoot, owner), 'utf8')).toContain(path.split('/').at(-1));
    }
    for (const href of [...styles.links, ...styles.deferred]) {
        const path = localPath(href, page.url());
        if (path) expect(existsSync(join(webRoot, path)), path).toBe(true);
    }
});

test('Vite output resolves its local styles and direct script references', async ({ browser }) => {
    test.skip(!existsSync(join(distRoot, 'index.html')), 'Run npm run build before checking Vite output');
    const { server, origin } = await startDistServer();
    const context = await browser.newContext();
    const page = await context.newPage();
    const failed = [];
    page.on('response', (response) => {
        const pathname = new URL(response.url()).pathname;
        const stagedOnlyManifest = pathname === '/data/assistant/knowledge/manifest.json';
        if (response.url().startsWith(origin) && response.status() >= 400
            && !pathname.startsWith('/api/') && !stagedOnlyManifest) {
            failed.push(`${response.status()} ${pathname}`);
        }
    });
    try {
        await page.goto(origin, { waitUntil: 'domcontentloaded' });
        const styles = await readStyles(page);
        const direct = await page.evaluate(() => Array.from(
            document.querySelectorAll('script[src], head link[rel="stylesheet"]'),
            (element) => element.getAttribute('src') || element.getAttribute('href'),
        ));
        for (const href of [...styles.deferred, ...direct]) {
            const path = localPath(href, origin);
            if (path) expect(existsSync(join(distRoot, path)), path).toBe(true);
        }
        await page.waitForFunction(() => !!document.querySelector('#tab-bar .tab'), null, { timeout: 30_000 });
        await page.waitForLoadState('load');
        await page.getByRole('tab', { name: 'JEV', exact: true }).click();
        await expect(page.locator('#panel-jev.active .jev-console')).toBeVisible();
        expect(failed).toEqual([]);
    } finally {
        await context.close();
        await new Promise((resolveClose) => server.close(resolveClose));
    }
});

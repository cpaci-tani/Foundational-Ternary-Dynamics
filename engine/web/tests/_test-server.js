// @ts-check
/**
 * Test-server selection shared by the playwright.*.config.js files, plus the
 * globalSetup guard (default export) that refuses to test a server serving a
 * different checkout. The variables are documented at the top of
 * playwright.config.js.
 */
import { realpathSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const WEB_ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');

/**
 * Port for a config's web server: FTD_TEST_PORT when set, else the config's
 * own default.
 * @param {number} defaultPort
 */
export function testPort(defaultPort) {
  const raw = process.env.FTD_TEST_PORT;
  if (!raw) return defaultPort;
  const port = Number(raw);
  if (!Number.isInteger(port) || port < 1024 || port > 65535) {
    throw new Error(`FTD_TEST_PORT must be an integer in 1024..65535, got "${raw}"`);
  }
  return port;
}

/**
 * Whether the default config may adopt a server it did not start. A chosen
 * port is a private port, so FTD_TEST_PORT turns reuse off unless
 * FTD_TEST_REUSE_SERVER says otherwise.
 */
export function reuseExistingServer() {
  const raw = process.env.FTD_TEST_REUSE_SERVER;
  if (raw === '0' || raw === '1') return raw === '1';
  if (raw) throw new Error(`FTD_TEST_REUSE_SERVER must be 0 or 1, got "${raw}"`);
  return !process.env.FTD_TEST_PORT && !process.env.CI;
}

/** @param {string} path */
function canonical(path) {
  let real = path;
  try {
    real = realpathSync.native(path);
  } catch {
    // Not a path on this machine; compare it as reported.
  }
  return process.platform === 'win32' ? real.toLowerCase() : real;
}

/**
 * Ask the server which tree it serves. serve.py binds 127.0.0.1 only, so a
 * `localhost` that resolves to ::1 first is retried over IPv4.
 * @param {string} baseURL
 * @returns {Promise<{ info: { webRoot: string, pid?: number } | null, failure: string }>}
 */
async function serverInfo(baseURL) {
  const url = new URL('/api/server-info', baseURL);
  const candidates = [url.href];
  if (url.hostname === 'localhost') {
    url.hostname = '127.0.0.1';
    candidates.push(url.href);
  }
  let failure = 'no response';
  for (const candidate of candidates) {
    try {
      const response = await fetch(candidate, { signal: AbortSignal.timeout(5000) });
      if (!response.ok) return { info: null, failure: `HTTP ${response.status}` };
      const info = await response.json();
      if (typeof info?.webRoot === 'string') return { info, failure: '' };
      return { info: null, failure: 'a reply without webRoot' };
    } catch (error) {
      failure = error instanceof Error ? error.message : String(error);
    }
  }
  return { info: null, failure };
}

/**
 * globalSetup: runs once the webServer is up and before any test. Reuse is the
 * usual way onto a foreign server. The other is a bind race: Playwright only
 * waits for the port to answer, so if another run's server takes it while ours
 * is still starting, ours exits and theirs is tested. Hence no reuse condition.
 * @param {import('@playwright/test').FullConfig} config
 */
export default async function verifyServedTree(config) {
  const baseURL = config.projects[0]?.use?.baseURL;
  if (!baseURL) return;
  const { info, failure } = await serverInfo(baseURL);
  if (info && canonical(info.webRoot) === canonical(WEB_ROOT)) return;
  const serving = info
    ? `${info.webRoot} (pid ${info.pid})`
    : `unknown: GET /api/server-info gave ${failure}, so it is not this checkout's serve.py`;
  throw new Error([
    `The server at ${baseURL} does not serve this checkout, so no spec was run.`,
    `  expected: ${WEB_ROOT}`,
    `  serving:  ${serving}`,
    'It belongs to another run, most likely another session\'s: reused, or it took the port first.',
    'Leave that server running and re-run on a private port, e.g. FTD_TEST_PORT=8091.',
  ].join('\n'));
}

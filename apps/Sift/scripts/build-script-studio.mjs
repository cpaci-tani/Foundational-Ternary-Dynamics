import { build } from 'esbuild';
import { createHash } from 'node:crypto';
import { copyFile, mkdir, readFile, readdir, writeFile } from 'node:fs/promises';
import { relative, resolve } from 'node:path';

const root = resolve(import.meta.dirname, '..');
const output = resolve(root, 'WebAssets', 'dist');

await mkdir(output, { recursive: true });
await build({
  absWorkingDir: root,
  entryPoints: {
    'script-studio': 'WebAssets/src/script-studio.js',
    'editor.worker': 'node_modules/monaco-editor/esm/vs/editor/editor.worker.js',
    'typescript.worker': 'node_modules/monaco-editor/esm/vs/language/typescript/ts.worker.js'
  },
  outdir: output,
  bundle: true,
  splitting: true,
  format: 'esm',
  platform: 'browser',
  target: ['chrome120'],
  minify: true,
  sourcemap: false,
  legalComments: 'eof',
  chunkNames: 'chunks/[name]-[hash]',
  assetNames: 'assets/[name]-[hash]',
  loader: {
    '.ttf': 'file'
  }
});
await copyFile(resolve(root, 'WebAssets', 'src', 'script-studio.html'), resolve(output, 'index.html'));
await copyFile(resolve(root, 'WebAssets', 'THIRD_PARTY_NOTICES.md'), resolve(output, 'THIRD_PARTY_NOTICES.md'));

const inputFiles = [
  'package.json',
  'package-lock.json',
  'scripts/build-script-studio.mjs',
  'scripts/verify-script-studio.ps1',
  'WebAssets/THIRD_PARTY_NOTICES.md',
  'WebAssets/src/script-studio.css',
  'WebAssets/src/script-studio.html',
  'WebAssets/src/script-studio.js'
];

async function sha256(path) {
  return createHash('sha256').update(await readFile(path)).digest('hex');
}

async function filesUnder(directory) {
  const values = [];
  for (const item of await readdir(directory, { withFileTypes: true })) {
    const path = resolve(directory, item.name);
    if (item.isDirectory()) values.push(...await filesUnder(path));
    else if (item.name !== 'asset-manifest.json') values.push(path);
  }
  return values;
}

const inputs = {};
for (const file of inputFiles) inputs[file] = await sha256(resolve(root, file));
const outputs = {};
for (const file of (await filesUnder(output)).sort()) {
  outputs[relative(output, file).replaceAll('\\', '/')] = await sha256(file);
}
await writeFile(resolve(output, 'asset-manifest.json'), `${JSON.stringify({ schemaVersion: 1, inputs, outputs }, null, 2)}\n`);

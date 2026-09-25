import { randomBytes } from 'node:crypto';
import { lstatSync, mkdirSync, writeFileSync } from 'node:fs';
import { basename, join } from 'node:path';
import { fileURLToPath } from 'node:url';

export const HARDWARE_SCRATCH_ROOT = fileURLToPath(new URL('../../build/scale0-hardware/', import.meta.url));
const ENGINE_BUILD_ROOT = fileURLToPath(new URL('../../build/', import.meta.url));

export function hardwareRunId(requested = process.env.FTD_AUDIT_RUN_ID) {
    const runId = requested === undefined
        ? `${new Date().toISOString().replace(/[-:.]/g, '')}-${randomBytes(4).toString('hex')}`
        : requested;
    if (!/^[A-Za-z0-9][A-Za-z0-9_-]{0,63}$/.test(runId)
        || /^(con|prn|aux|nul|com[1-9]|lpt[1-9])$/i.test(runId)) {
        throw new Error('FTD_AUDIT_RUN_ID must be 1-64 letters, digits, underscores or hyphens, starting with a letter or digit');
    }
    return runId;
}

export function reserveHardwareReport({ runId, filename, report }) {
    const validatedRunId = hardwareRunId(runId);
    if (basename(filename) !== filename || !/^scale0-hardware-[A-Za-z0-9-]+\.json$/.test(filename)) {
        throw new Error('Unsafe hardware report filename');
    }
    mkdirSync(ENGINE_BUILD_ROOT, { recursive: true });
    if (lstatSync(ENGINE_BUILD_ROOT).isSymbolicLink()) throw new Error('Refusing linked engine build root');
    mkdirSync(HARDWARE_SCRATCH_ROOT, { recursive: true });
    if (lstatSync(HARDWARE_SCRATCH_ROOT).isSymbolicLink()) throw new Error('Refusing linked hardware scratch root');
    const directory = join(HARDWARE_SCRATCH_ROOT, validatedRunId);
    mkdirSync(directory, { recursive: true });
    if (lstatSync(directory).isSymbolicLink()) throw new Error(`Refusing linked run directory: ${directory}`);
    const output = join(directory, filename);
    report.provenance = {
        runId: validatedRunId,
        scratchPath: `engine/build/scale0-hardware/${validatedRunId}/${filename}`,
        evidenceStatus: 'scratch-unpromoted',
        startedAt: new Date().toISOString(),
    };
    const serialize = () => JSON.stringify(report, null, 2) + '\n';
    try {
        writeFileSync(output, serialize(), { flag: 'wx' });
    } catch (error) {
        if (error?.code === 'EEXIST') throw new Error(`Refusing to overwrite previous campaign output: ${output}`, { cause: error });
        throw error;
    }
    return { output, save: () => writeFileSync(output, serialize()) };
}

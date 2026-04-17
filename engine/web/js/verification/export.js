/**
 * Verification Lab — export helpers.
 * CSV / JSON / clipboard for a single experiment's results.
 */

export function toCSV(experiment, results, aggregate) {
    const lines = [];
    lines.push('# FTD Verification Lab');
    lines.push(`# Experiment: ${experiment.id} — ${experiment.name}`);
    lines.push(`# Category: ${experiment.category} · Tag: ${experiment.epistemicTag}`);
    lines.push(`# Theory: ${experiment.theoryFn?.().value ?? '?'} ${experiment.theoryFn?.().units ?? ''}`);
    lines.push(`# Trials: ${results.length}`);
    lines.push(`# Mean: ${aggregate?.mean ?? ''}  Stddev: ${aggregate?.stddev ?? ''}`);
    lines.push('');
    lines.push('trial,value');
    results.forEach((r, i) => {
        const v = typeof r === 'number' ? r : r?.value ?? '';
        lines.push(`${i},${v}`);
    });
    return lines.join('\n');
}

export function toJSON(experiment, results, aggregate) {
    return JSON.stringify({
        id: experiment.id,
        name: experiment.name,
        category: experiment.category,
        epistemicTag: experiment.epistemicTag,
        theory: experiment.theoryFn?.(),
        tolerance: experiment.tolerance,
        trials: results.length,
        aggregate,
        results,
        timestamp: new Date().toISOString(),
    }, null, 2);
}

export function downloadBlob(filename, text, mime = 'text/plain') {
    const blob = new Blob([text], { type: mime });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(() => URL.revokeObjectURL(url), 1000);
}

export async function copyToClipboard(text) {
    try {
        await navigator.clipboard.writeText(text);
        return true;
    } catch (err) {
        // Fallback: textarea + execCommand
        const ta = document.createElement('textarea');
        ta.value = text;
        ta.style.position = 'fixed';
        ta.style.left = '-9999px';
        document.body.appendChild(ta);
        ta.select();
        try { document.execCommand('copy'); } catch (_) {}
        document.body.removeChild(ta);
        return false;
    }
}

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// 1. Clean JS Helper: strip comments and string literals
function cleanJs(content) {
    // Strip block comments
    let cleaned = content.replace(/\/\*[\s\S]*?\*\//g, '');
    // Strip single-line comments
    cleaned = cleaned.replace(/\/\/.*/g, '');
    // Strip string literals (single quoted, double quoted, template literals)
    cleaned = cleaned.replace(/'(?:\\'|[^'])*'/g, '');
    cleaned = cleaned.replace(/"(?:\\"|[^"])*"/g, '');
    cleaned = cleaned.replace(/`(?:\\`|[^`])*`/g, '');
    return cleaned;
}

// 2. Scan engine/web/js directory recursively
const jsDir = path.resolve(__dirname, '../js');
const errors = [];

function scanDir(dir) {
    const files = fs.readdirSync(dir);
    for (const file of files) {
        const fullPath = path.join(dir, file);
        const stat = fs.statSync(fullPath);
        if (stat.isDirectory()) {
            scanDir(fullPath);
        } else if (file.endsWith('.js')) {
            // Skip constants.js itself
            if (file === 'constants.js') continue;
            
            const relativePath = path.relative(jsDir, fullPath);
            const content = fs.readFileSync(fullPath, 'utf8');
            const cleaned = cleanJs(content);

            // Check for hardcoded constants
            if (cleaned.includes('137.036')) {
                errors.push(`File "${relativePath}" contains hardcoded literal x_plus root: "137.036"`);
            }
            if (cleaned.includes('0.511')) {
                errors.push(`File "${relativePath}" contains hardcoded literal K_B constant: "0.511"`);
            }
            if (/2\.9586[78]/.test(cleaned)) {
                errors.push(`File "${relativePath}" contains hardcoded literal G_STAR constant`);
            }
            // Check for hardcoded gravity/G_N 0.01 in code assignments or exports
            if (/(?:\b(G|G_N|gravity|grav)\b\s*[:=]\s*0\.01\b)/i.test(cleaned)) {
                errors.push(`File "${relativePath}" contains hardcoded literal gravity assignment "0.01"`);
            }
        }
    }
}

// Start scanning
scanDir(jsDir);

// 3. Verify telemetry buffer properties against telemetry-grid/component.js
try {
    const hubPath = path.resolve(jsDir, 'telemetry-hub.js');
    const compPath = path.resolve(jsDir, 'ui/panels/telemetry-grid/component.js');

    if (!fs.existsSync(hubPath)) {
        errors.push(`telemetry-hub.js not found at: ${hubPath}`);
    }
    if (!fs.existsSync(compPath)) {
        errors.push(`component.js not found at: ${compPath}`);
    }

    if (fs.existsSync(hubPath) && fs.existsSync(compPath)) {
        const hubContent = fs.readFileSync(hubPath, 'utf8');
        const compContent = fs.readFileSync(compPath, 'utf8');

        // Extract all buffer values from component.js
        // Matches e.g., buffer: 'plTotal' or buffer: "lag.total"
        const bufferMatches = [...compContent.matchAll(/buffer:\s*['"]([^'"]+)['"]/g)];
        const bufferPaths = bufferMatches.map(m => m[1]);

        if (bufferPaths.length === 0) {
            errors.push('No telemetry buffer paths found in component.js');
        }

        // Parse and mock evaluate telemetry-hub.js
        const hubCleaned = hubContent
            .replace(/export class/g, 'class')
            .replace(/export const/g, 'const');

        // Create constructor and execute inside a function block to avoid scope issues
        const factory = new Function(`${hubCleaned}\nreturn new TelemetryHub();`);
        const hubInstance = factory();

        // Check if each buffer path is resolvable on the TelemetryHub instance
        for (const p of bufferPaths) {
            const parts = p.split('.');
            let current = hubInstance;
            for (const part of parts) {
                if (current) current = current[part];
            }
            if (!current || typeof current.get !== 'function') {
                errors.push(`Telemetry buffer path "${p}" defined in component.js is missing or invalid in TelemetryHub`);
            }
        }
    }
} catch (err) {
    errors.push(`Telemetry grid verification threw exception: ${err.message}\n${err.stack}`);
}

// 4. Output results
if (errors.length > 0) {
    console.error('Consistency verification failed:');
    errors.forEach(e => console.error(`  - ${e}`));
    process.exit(1);
} else {
    console.log('Consistency verification succeeded! All constants and telemetry channels are aligned.');
    process.exit(0);
}

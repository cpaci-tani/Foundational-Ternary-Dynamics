#!/usr/bin/env node
import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import { CallToolRequestSchema, ListToolsRequestSchema, ListResourcesRequestSchema,
    ReadResourceRequestSchema, ListPromptsRequestSchema, GetPromptRequestSchema } from '@modelcontextprotocol/sdk/types.js';
import { TOOLS, validateToolArguments } from './tools.js';
import { readConfig, RelayClient } from './relay.js';
import { SCENARIO_LIMITS } from '../web/js/assistant/scenario-limits.js';

const WORKFLOW = `FTD local experiment workflow
1. Use ftd_observe to obtain the active workspace, ownerId, preparationVersion, supported actions and actual measurements.
2. Use ftd_list_scenarios and ftd_describe_scenario. The complete registered catalog and constructor-owned settings are authoritative. Check coverage and backend limitations.
3. Select a canned preparation or construct a data-only draft using that template's admitted scalar settings. Include name, goal, settings, ticks, sampleEvery and measurements. Draft goal/ticks/sampleEvery must match the request. No scripts, invented settings or new physics mappings.
4. Use ftd_run_scenario with fresh expected authority. The browser begins from an explicitly empty lattice, obtains one JEV approval for the complete fixed protocol, installs the chosen preparation, and records measurements. Fixed protocols support up to ${SCENARIO_LIMITS.ticks} ticks, ${SCENARIO_LIMITS.intervals} measured intervals plus the baseline, and ${SCENARIO_LIMITS.durationMs/60000} minutes. A supplied draft needs no embedded browser LLM. Poll ftd_run_status for progress by runId. Adaptive autonomous AI requests retain their separate five-minute/50-action limit; ordinary lattice.step remains capped at 4096 ticks.
5. Interpret recorded observations, compare them to the declared reference model, then form another bounded hypothesis. Distinguish measured values, adopted reference physics, documented claims and generated interpretations. Counts are not physical energy; a completed experiment does not certify a theory. Unavailable measurements stay unavailable. Current ledger and constitution status govern historical claims.
6. ftd_execute_plan supports at most eight typed operations; JEV is required for mutations. Browser ownership and preparation checks remain authoritative. Never execute JavaScript or bypass the active owner.
7. ftd_stop cancels pending work and prevents subsequent MCP writes until the user reconnects. It preserves playback at the stop instant. A timeout/cancellation does not roll back committed actions; inspect receipts before retrying.
This gateway requires an MCP-capable client. It does not turn a bare language model into an MCP client and does not provide an external JEV key. The local dashboard tab and enabled MCP session must remain open.`;

async function main() {
    const relay = new RelayClient(readConfig());
    // Explicit JSON Schema is shared with clients; this low-level SDK server
    // avoids translating permissive action payloads through generated code.
    const server = new Server({ name: 'ftd-local-simulator', version: '1.0.0' }, {
        capabilities: { tools: {}, resources: {}, prompts: {} }, instructions: WORKFLOW,
    });
    server.setRequestHandler(ListToolsRequestSchema, async () => ({ tools: TOOLS.map(({ method: _method, ...spec }) => spec) }));
    server.setRequestHandler(CallToolRequestSchema, async (request, extra) => {
        const spec = TOOLS.find(row => row.name === request.params.name);
        let payload;
        try {
            if (!spec) throw new Error('Unknown FTD tool.');
            const args = request.params.arguments ?? {};
            validateToolArguments(spec, args);
            payload = await relay.call(spec.method, args, extra.signal);
        } catch (error) {
            payload = { error: { code: 'INVALID_ARGUMENTS', message: error instanceof Error ? error.message : 'Invalid tool arguments.' } };
        }
        return { content: [{ type: 'text', text: JSON.stringify(payload) }], structuredContent: payload, ...(payload.error ? { isError: true } : {}) };
    });
    server.setRequestHandler(ListResourcesRequestSchema, async () => ({ resources: [{ uri: 'ftd://workflow', name: 'FTD experiment workflow',
        description: 'Ownership, JEV approval, measured experiments and scientific interpretation.', mimeType: 'text/plain' }] }));
    server.setRequestHandler(ReadResourceRequestSchema, async request => {
        if (request.params.uri !== 'ftd://workflow') throw new Error('Unknown FTD resource.');
        return { contents: [{ uri: 'ftd://workflow', mimeType: 'text/plain', text: WORKFLOW }] };
    });
    server.setRequestHandler(ListPromptsRequestSchema, async () => ({ prompts: [{ name: 'ftd_live_experiment',
        description: 'Plan a bounded experiment using actual observations and constructor templates.',
        arguments: [{ name: 'goal', description: 'The scientific or simulator question to investigate.', required: true }] }] }));
    server.setRequestHandler(GetPromptRequestSchema, async request => {
        const goal = request.params.arguments?.goal;
        if (request.params.name !== 'ftd_live_experiment' || typeof goal !== 'string' || !goal.trim() || goal.length > 2000)
            throw new Error('Provide a goal of 1 to 2000 characters for ftd_live_experiment.');
        return { description: 'FTD live experiment', messages: [{ role: 'user', content: { type: 'text', text: `${WORKFLOW}\n\nExperiment goal (user request):\n${goal}` } }] };
    });
    // stdout is reserved for MCP protocol messages. No credentials or request
    // bodies are logged to either stream.
    await server.connect(new StdioServerTransport(process.stdin, process.stdout, { maxBufferSize: 256 * 1024 }));
    process.stdin.once('end', () => { void server.close(); });
}

main().catch(() => { process.stderr.write('FTD MCP could not start. Check the loopback URL and required session/token environment variables.\n'); process.exitCode = 1; });

import { rafCoordinator } from '../lib/raf-coordinator.js';

/** Drain already requested work before lending presentation to another world. */
export async function suspendDashboardWork(owners, { timeoutMs = 15000, settleAnalysis = async () => {} } = {}) {
    const resumePanels = rafCoordinator.suspend();
    const resumes = [];
    let restored = false;
    const restore = () => {
        if (restored) return;
        restored = true;
        for (const resume of resumes.reverse()) resume();
        resumePanels();
    };
    try {
        // These optional experiments can own independent analysis workers or
        // explicitly request steps; cancellation retains their panel/history.
        globalThis.window?.__ftdSpectrumPanel?.suspendBackgroundWork?.();
        globalThis.window?.__ftdGenesisBurstPanel?.suspendBackgroundWork?.();
        for (const owner of new Set(owners.filter(Boolean))) {
            owner.cancelQueuedTicks?.();
            owner.setRunning?.(false);
            const deadline = performance.now() + timeoutMs;
            while (owner.runningStateSettled === false || owner.ready === false && !owner.failed
                || owner._hasPendingScenarioWork?.()) {
                if (performance.now() > deadline) throw new Error('Lattice did not settle; switch cancelled');
                await new Promise(resolve => setTimeout(resolve, 16));
            }
            if (owner.queue) await owner.queue;
            if (owner.suspendBackgroundWork) resumes.push(await owner.suspendBackgroundWork());
        }
        await settleAnalysis();
        return restore;
    } catch (error) { restore(); throw error; }
}

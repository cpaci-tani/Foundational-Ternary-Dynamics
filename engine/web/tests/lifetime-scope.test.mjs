import test from 'node:test';
import assert from 'node:assert/strict';
import { LifetimeScope } from '../js/ui/utils/lifetime-scope.js';

test('remount releases handlers and each resource exactly once', () => {
    const target = new EventTarget();
    let hits = 0;
    const scope = new LifetimeScope();
    scope.on(target, 'change', () => hits++);
    let releases = 0;
    const release = scope.defer(() => releases++);
    target.dispatchEvent(new Event('change'));
    release(); release(); scope.dispose(); scope.dispose();
    target.dispatchEvent(new Event('change'));
    assert.equal(hits, 1);
    assert.equal(releases, 1);
    const next = new LifetimeScope();
    next.on(target, 'change', () => hits++);
    target.dispatchEvent(new Event('change'));
    next.dispose();
    assert.equal(hits, 2);
});

test('completed timeouts release bookkeeping and disposed work never executes', async () => {
    const scope = new LifetimeScope();
    await new Promise(resolve => scope.timeout(resolve));
    assert.equal(scope._releases.size, 0);
    let called = false;
    scope.timeout(() => { called = true; }, 5);
    scope.dispose();
    await new Promise(resolve => setTimeout(resolve, 15));
    assert.equal(called, false);
});

test('one throwing release cannot strand other owned resources', () => {
    const scope = new LifetimeScope();
    let released = false;
    scope.defer(() => { released = true; });
    scope.defer(() => { throw new Error('fixture'); });
    assert.throws(() => scope.dispose(), AggregateError);
    assert.equal(released, true);
    assert.equal(scope._releases.size, 0);
});

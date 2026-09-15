import test from 'node:test';
import assert from 'node:assert/strict';
import { describeNativeTransport, readNativeTransportThreshold } from '../js/scales/scale0/ui/overlays/native-transport-legend.js';

const base = { status: 'ok', reason: '', closure: 1e-15, exchangeTerms: 0, drawnLinks: 1200, max: 0.01,
    knotCount: 0, exchangeSites: 0, exchangeHidden: false };

test('names the engine and says the balance closes on a pure wave step', () => {
    const text = describeNativeTransport(base);
    assert.match(text, /\[REFERENCE ENGINE\]/);
    assert.match(text, /v1 wave step/);
    assert.match(text, /1200 links drawn/);
    assert.match(text, /closes at every site/);
    assert.doesNotMatch(text, /does not close/);
});

test('says energy is exchanged off the links and names the active terms when the balance fails', () => {
    const text = describeNativeTransport({ ...base, closure: 0.26, exchangeTerms: 1 | 2 | 8, exchangeHidden: true });
    assert.match(text, /does not close/);
    assert.match(text, /Enabled terms that can exchange it: thermostat, Gauss projection, genesis\./);
    assert.match(text, /hidden while the thermostat runs/);
    for (const sentence of text.split(/(?<=\.)\s+/)) {
        assert.ok(!(/current/i.test(sentence) && /exchang/i.test(sentence)), `a sentence calls off-link exchange a current: ${sentence}`);
    }
});

test('says no known term accounts for the exchange when none is enabled', () => {
    const text = describeNativeTransport({ ...base, closure: 0.1, exchangeTerms: 0 });
    assert.match(text, /does not close/);
    assert.match(text, /No enabled term known to this readout accounts for it\./);
});

test('names the de Broglie clock term', () => {
    const text = describeNativeTransport({ ...base, closure: 0.1, exchangeTerms: 1 << 8 });
    assert.match(text, /Enabled terms that can exchange it: de Broglie clock term\./);
});

test('an observer that is off says it is waiting for the first observed ticks', () => {
    assert.match(describeNativeTransport({ ...base, status: 'off' }), /Waiting for the engine to report its first observed ticks/);
});

test('reports matter clusters and exchange sites when present', () => {
    const text = describeNativeTransport({ ...base, closure: 0.012, exchangeTerms: 2, exchangeSites: 40, knotCount: 5 });
    assert.match(text, /40 sites exchanging energy off the links/);
    assert.match(text, /5 manifested matter clusters/);
});

test('unavailable and warming states are stated, never silent', () => {
    assert.match(describeNativeTransport({ ...base, status: 'unavailable', reason: 'symplectic_leapfrog changes the wave step' }),
        /Unavailable for this configuration: symplectic_leapfrog changes the wave step/);
    assert.match(describeNativeTransport({ ...base, status: 'warming' }), /Warming/);
    assert.match(describeNativeTransport(null), /waiting for the engine/);
});

test('the draw threshold defaults to 5% of the frame maximum outside a page', () => {
    assert.equal(readNativeTransportThreshold(), 0.05);
});

import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
const source = readFileSync(new URL('../js/ui/charts/history-index.js', import.meta.url), 'utf8');
const { getHistoryStats, projectHistoryIndices } = vm.runInNewContext(
    source.replace(/export /g, '') + '\n({getHistoryStats, projectHistoryIndices})');

function buffer(values) {
    return { values, count: values.length, total: values.length, generation: 0, reads: 0,
        get(i) { this.reads++; return this.values[i]; }, getTick(i) { return i * 3; },
        push(v) { this.values.push(v); this.count++; this.total++; },
    };
}

test('indexed windows match independent statistics, including missing samples and block boundaries', () => {
    const values = Array.from({length: 523}, (_, i) => i % 29 ? (i % 73) - 35 : NaN);
    const b = buffer(values);
    for (const [start, count] of [[0,523],[1,80],[31,66],[32,128],[513,10],[100,0]]) {
        const finite = values.slice(start,start+count).filter(Number.isFinite);
        const actual = getHistoryStats(b,start,count);
        assert.equal(actual.count, finite.length);
        assert.equal(actual.sum, finite.reduce((a,b) => a+b,0));
        assert.equal(actual.min, Math.min(...finite));
        assert.equal(actual.max, Math.max(...finite));
        assert.equal(actual.avg, finite.length ? finite.reduce((a,b) => a+b,0)/finite.length : null);
    }
});

test('append, same-tick refinement, growth, and equal-length reset invalidate the shared index', () => {
    const b = buffer(Array.from({length:32},(_,i)=>i));
    assert.equal(getHistoryStats(b).max,31);
    b.values[31]=900; b.push(-100);
    assert.equal(getHistoryStats(b).max,900);
    assert.equal(getHistoryStats(b).min,-100);
    b.values[32]=10;
    assert.equal(getHistoryStats(b).min,0);
    b.values.fill(10); b.generation++;
    assert.equal(getHistoryStats(b).min,10);
    assert.equal(getHistoryStats(b).avg,10);
    b.values.length=0; b.count=0; b.total=0; b.generation++;
    assert.equal(getHistoryStats(b).count,0);
    for (let i=0;i<1100;i++) b.push(i);
    assert.equal(getHistoryStats(b).sum,1099*1100/2);
});

test('long-run window queries and new samples read bounded source ranges', () => {
    const b=buffer(Array.from({length:200000},(_,i)=>i));
    assert.equal(getHistoryStats(b,199880,120).max,199999);
    assert.equal(b.reads,120,'short late window does not index the entire unseen run');
    assert.equal(getHistoryStats(b).max,199999);
    b.reads=0;
    const stats=getHistoryStats(b, 123,190000);
    assert.equal(stats.count,190000);
    assert.ok(b.reads<100,`window read ${b.reads} samples`);
    b.push(200000); b.reads=0;
    assert.equal(getHistoryStats(b).max,200000);
    assert.ok(b.reads<100,`append read ${b.reads} samples`);
});

test('pixel envelope keeps real endpoints, extrema, missing breaks, and aligned-channel peaks', () => {
    const a=buffer(new Array(10000).fill(1)), b=buffer(new Array(10000).fill(2));
    a.values[240]=999; a.values[1700]=-700; a.values[6500]=NaN;
    b.values[455]=777; b.values[5200]=-800;
    const indices=projectHistoryIndices([a,b],10000,300);
    assert.ok(indices.length<=600);
    for (const i of [0,9999,240,1700,6500,455,5200]) assert.ok(indices.includes(i),`missing ${i}`);
    assert.deepEqual(Array.from(indices).sort((a,b)=>a-b),Array.from(indices));
    assert.equal(a.count,10000); assert.equal(b.count,10000);
    assert.equal(projectHistoryIndices([a],100,300),null);
});

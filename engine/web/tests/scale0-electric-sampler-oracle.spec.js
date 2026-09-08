import { test, expect } from '@playwright/test';

// Real rebuilt modules, with the unchanged full-EM point diagnostic as oracle.
// No dashboard simulation is started by this isolated document.
const variants = [
    { id: 'wasm32', loader: 'ftd_core.js', factory: 'createFTDModule' },
    { id: 'wasm64', loader: 'ftd_core64.js', factory: 'createFTDModule64' },
    { id: 'wasm32-threads', loader: 'ftd_core_mt.js', factory: 'createFTDModuleMT' },
];

for (const variant of variants) {
    test(`${variant.id}: electric sampler equals full-EM oracle bitwise`, async ({ page }, testInfo) => {
        test.setTimeout(180000);
        await page.goto('/wasm/build_info.json');
        await page.addScriptTag({ url: `/wasm/${variant.loader}` });
        const result = await page.evaluate(async variant => {
            if (variant.id === 'wasm32-threads' && !crossOriginIsolated) throw new Error('Threads require actual cross-origin isolation');
            const mod = await globalThis[variant.factory]({
                locateFile: file => `/wasm/${file}`,
                mainScriptUrlOrBlob: `${location.origin}/wasm/${variant.loader}`,
                print: () => {},
            });
            const bitCopy = array => new Uint32Array(new Float32Array(array).buffer);
            function sameBits(a,b,label) {
                if (a.length !== b.length) throw new Error(`${label}: lengths ${a.length} != ${b.length}`);
                for(let i=0;i<a.length;i++) if(a[i]!==b[i]) throw new Error(`${label}[${i}]: 0x${a[i].toString(16)} != 0x${b[i].toString(16)}`);
            }
            function identity(rb) {
                const d=mod.captureDynamicalStateDigest(rb);
                return JSON.stringify([rb.currentTick(),d.tick,d.state_version,d.hash_lo,d.hash_hi,d.nonfinite_value_count,d.nondefault_value_count]);
            }
            const rows=[];
            function verify(rb,L,requestedStride,phase) {
                const before=identity(rb);
                const raw=mod.getEFieldSampled(rb,requestedStride);
                // Own bytes immediately: sampler views are borrowed WASM memory.
                const positions=bitCopy(raw.positions),vectors=bitCopy(raw.vectors);
                const {count,effectiveStride,origin}=raw;
                const stride=Math.max(1,Math.min(L,requestedStride));
                const expectedOrigin=Math.floor((L-1)/2)%stride;
                if(effectiveStride!==stride||origin!==expectedOrigin) throw new Error('Grid metadata changed');
                const expectedPositions=[],expectedVectors=[];
                let below=0,emittedSignedZero=false;
                for(let z=origin;z<L;z+=stride) for(let y=origin;y<L;y+=stride) for(let x=origin;x<L;x+=stride) {
                    const em=mod.inspectVoxel(rb,x,y,z);
                    if(em.Emag<1e-15){below++;continue;}
                    expectedPositions.push(x+0.5,y+0.5,z+0.5);
                    expectedVectors.push(em.Ex,em.Ey,em.Ez);
                    emittedSignedZero ||= [em.Ex,em.Ey,em.Ez].some(value=>Object.is(value,-0));
                }
                if(count!==expectedVectors.length/3) throw new Error('Threshold compaction count changed');
                sameBits(positions,bitCopy(expectedPositions),'positions');
                sameBits(vectors,bitCopy(expectedVectors),'vectors');
                if(identity(rb)!==before) throw new Error('Sampler/oracle advanced tick or changed dynamical digest');
                rows.push({L,phase,requestedStride,effectiveStride,origin,count,below,emittedSignedZero,identity:before});
            }
            for(const L of [1,8,9,33]) {
                const rb=new mod.RenderBridge(L);
                try {
                    verify(rb,L,1,'empty');
                    // Finite, deterministic preparations. Distinct sites at L>=8
                    // cover both threshold sides and emitted signed zeros. The
                    // existing injection ABI rounds its inputs to Float32.
                    const center=Math.floor((L-1)/2);
                    const seeds=[
                        [0,0,0,0.5e-15,0,0], [1%L,0,0,1e-15,0,-0],
                        [2%L,0,0,-2e-15,0,0], [3%L,0,0,0,1,0],
                        [center,center,center,0.25,-0.5,0.75],
                        [L-1,L-1,L-1,-2,3,-4], [0,L-1,0,1e-300,0,0],
                    ];
                    for(const seed of seeds) mod.injectWaveVel(rb,...seed);
                    // Nonzero magnetic curl remains present in the full oracle,
                    // but must have no influence on this electric-only output.
                    mod.injectFlux(rb,0,0,0,0.125,-0.25,0.5);
                    mod.injectFlux(rb,center,center,center,-0.5,0.125,0.25);
                    for(const stride of [1,2,L+11,2147483647]) verify(rb,L,stride,'prepared');
                    if(L>=8) {
                        const below=mod.inspectVoxel(rb,0,0,0);
                        const boundary=mod.inspectVoxel(rb,1,0,0);
                        if(!(below.Emag<1e-15)||boundary.Emag<1e-15) throw new Error('Threshold fixture lost its intended side');
                    }
                    rb.tick();
                    verify(rb,L,2,'after-one-tick');
                } finally { rb.delete(); }
            }
            return {variant:variant.id,rows,ieee:mod.numericSemanticsAreIeee(),crossOriginIsolated};
        }, variant);
        expect(result.ieee).toBe(true);
        expect(result.rows).toHaveLength(24);
        expect(result.rows.some(row=>row.emittedSignedZero)).toBe(true);
        expect(result.rows.filter(row=>row.phase==='empty').every(row=>row.count===0)).toBe(true);
        await testInfo.attach(`electric-oracle-${variant.id}.json`, {body:JSON.stringify(result,null,2),contentType:'application/json'});
    });
}

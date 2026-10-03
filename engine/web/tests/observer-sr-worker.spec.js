import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';

test('real session worker FIFO replays fixed-tick forces independent of presentation batching',async({page},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr);
    const evidence=await page.evaluate(async()=>{
        const {ObserverWorkerClient}=await import('/js/observer/worker-client.js');
        const replay=async batches=>{
            const client=new ObserverWorkerClient({playing:true});await client.ready;
            const start=client.latest.observer.position;
            await client.request({type:'create',entity:{name:'Replay marker',shape:'beacon',position:[0,0,0],coordinateForcePerMass:[.5,0,0]}});
            for(const frames of batches)await client.send({type:'advance',dt:frames/120,input:{move:[1,0,0],speed:.99,acceleration:.5},epoch:client.latest.epoch,sessionId:client.latest.sessionId});
            // Each request has at most 30 ticks; all batched frames stay below it.
            const result=structuredClone(client.latest),oldEpoch=result.epoch;
            await client.request({type:'pause'});
            const saved=structuredClone(client.latest);
            await client.request({type:'reset'});
            const stale=await client.send({type:'advance',dt:1/120,epoch:oldEpoch,input:{move:[1,0,0],speed:.99,acceleration:.5}});
            const reset=structuredClone(client.latest);
            await client.request({type:'load',snapshot:saved});
            const loaded=structuredClone(client.latest);
            client.dispose();return {start,result,stale:stale.ok,reset,loaded};
        };
        const a=await replay(Array(120).fill(1)),b=await replay(Array(8).fill(15));
        return {a,b};
    });
    expect(evidence.a.result.tick).toBe(120);expect(evidence.b.result.tick).toBe(120);
    expect(evidence.a.result.observer).toEqual(evidence.b.result.observer);
    expect(evidence.a.result.segments).toEqual(evidence.b.result.segments);
    const marker=evidence.a.result.entities.find(entity=>entity.name==='Replay marker');
    expect(Math.abs(marker.clockOffset-evidence.a.result.observer.properTime)).toBeLessThan(1e-12);
    expect(evidence.a.result.segments.filter(segment=>segment.entityId===marker.id).length).toBe(121);
    expect(evidence.a.stale).toBe(false);expect(evidence.a.reset.tick).toBe(0);
    expect(evidence.a.loaded.observer).toEqual(evidence.a.result.observer);
    expect(evidence.a.loaded.epoch).toBeGreaterThan(evidence.a.result.epoch);
    const path=info.outputPath('sr-worker-evidence.json');await writeFile(path,JSON.stringify({ticks:evidence.a.result.tick,clock:evidence.a.result.observer.properTime,markerClock:marker.clockOffset,markerSegments:121,staleRejected:!evidence.a.stale,resetWorldline:evidence.a.reset.observer.worldline,loadedEpoch:evidence.a.loaded.epoch},null,2));
    await info.attach('worker-FIFO-evidence',{path,contentType:'application/json'});
});

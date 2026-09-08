// Opt-in local laboratory. All evolution occurs in the compiled worker.
const byId = (id) => document.getElementById(id);
const elements = Object.fromEntries(['preparation','size','width','reset','step','run','status','tick','phase','tokens','generation','micro','blocks','provenance'].map(id=>[id,byId(id)]));
let worker=null, ownerId=null, generation='0', requestSequence=0, epoch=0;
let running=false, busy=false, lastPublish=0, microView=null, blockView=null;
const pending=new Map();
const stageNames=['Admission','Collision / relations','Streaming','Manifestation'];

function request(op, fields={}) {
  const requestId=String(++requestSequence);
  const message={requestId,op,...fields};
  if(op!=='init') Object.assign(message,{ownerId,generation});
  return new Promise((resolve,reject)=>{
    const nextGeneration=op==='init'?'0':String(BigInt(generation)+((op==='restore'||(op==='advance'&&BigInt(fields.microticks)>0n))?1n:0n));
    pending.set(requestId,{resolve,reject,expectedGeneration:nextGeneration});
    worker.postMessage(message);
  });
}
function terminate() {
  if(worker) worker.terminate();
  worker=null;
  for(const item of pending.values()) item.reject(new Error('Owner replaced'));
  pending.clear();
}
function paint(canvas, view) {
  if(!view) return;
  const side=Number(view.lattice_size)/Number(view.width);
  const columns=Math.ceil(Math.sqrt(side)), rows=Math.ceil(side/columns);
  const pixels=Math.max(200, Math.floor(canvas.clientWidth*devicePixelRatio));
  if(canvas.width!==pixels || canvas.height!==pixels) {canvas.width=pixels; canvas.height=pixels;}
  const ctx=canvas.getContext('2d');
  ctx.fillStyle='#0c131b';ctx.fillRect(0,0,pixels,pixels);
  const tile=Math.min(pixels/columns,pixels/rows), cell=tile/(side+1);
  const values=view.field_tokens.map(Number), max=Math.max(1,...values);
  for(let z=0;z<side;z++) for(let x=0;x<side;x++) for(let y=0;y<side;y++) {
    const value=values[(x*side+y)*side+z];
    const bright=13+62*Math.sqrt(value/max);
    ctx.fillStyle=`hsl(164 50% ${bright}%)`;
    const left=(z%columns)*tile+(x+.5)*cell;
    const top=Math.floor(z/columns)*tile+(y+.5)*cell;
    ctx.fillRect(left,top,Math.max(1,cell-2),Math.max(1,cell-2));
  }
}
async function observe(localEpoch) {
  const requestedWidth=elements.width.value;
  const fine=await request('observe',{width:'1',observable:'counts'});
  const blocks=await request('observe',{width:requestedWidth,observable:'counts'});
  if(localEpoch!==epoch) return;
  if(fine.ownerId!==blocks.ownerId || fine.generation!==blocks.generation || fine.payload.microtick!==blocks.payload.microtick)
    throw new Error('Observation lineage mismatch');
  microView=fine.payload;blockView=blocks.payload;
  paint(elements.micro,microView);paint(elements.blocks,blockView);
  elements.tick.textContent=fine.payload.microtick;
  elements.phase.textContent=stageNames[Number(fine.payload.phase)];
  elements.generation.textContent=blocks.generation;
  const tokens=[...fine.payload.field_tokens,...fine.payload.relation_tokens].reduce((sum,n)=>sum+BigInt(n),0n);
  elements.tokens.textContent=tokens.toString();
  elements.provenance.textContent=JSON.stringify({ownerId,generation,microtick:fine.payload.microtick,
    law:fine.payload.law_id,backend:'compiled_wasm_worker',status:fine.payload.status,
    tickStart:fine.payload.tick_start,tickEnd:fine.payload.tick_end,
    lengthUnit:fine.payload.length_unit,timeUnit:fine.payload.time_unit,
    boundary:'periodic',calibration:'unidentified',continuumRecovery:false,materialRecovery:false,gravityRecovery:false},null,2);
  window.__strictLabSnapshot={ownerId,generation,tick:fine.payload.microtick,phase:fine.payload.phase,
    width:blocks.payload.width,tokens:tokens.toString(),micro:microView,blocks:blockView};
}
function controls() {
  elements.step.disabled=busy||running||!ownerId;
  elements.run.disabled=!ownerId||(busy&&!running);
  elements.width.disabled=busy||!ownerId;
  elements.run.textContent=running?'Pause':'Run';
}
function clearPublication() {
  microView=null;blockView=null;window.__strictLabSnapshot=null;
  for(const canvas of [elements.micro,elements.blocks])canvas.getContext('2d').clearRect(0,0,canvas.width,canvas.height);
  for(const key of ['tick','phase','tokens','generation'])elements[key].textContent='—';
  elements.provenance.textContent='Waiting for one authoritative owner.';
}
function fail(error, localEpoch=epoch) {
  if(localEpoch!==epoch) return;
  epoch++;running=false;busy=false;ownerId=null;generation='0';
  terminate();clearPublication();elements.status.textContent=String(error.message||error);controls();
}
async function initialize() {
  const localEpoch=++epoch;
  running=false;busy=true;terminate();ownerId=null;generation='0';clearPublication();controls();
  elements.status.textContent='Loading a fresh finite preparation…';
  try {
    const size=Number(elements.size.value), oldWidth=Number(elements.width.value);
    elements.width.replaceChildren(...Array.from({length:size},(_,i)=>i+1).filter(v=>size%v===0).map(v=>new Option(String(v),String(v))));
    elements.width.value=String(size%oldWidth===0?oldWidth:1);
    const seedUrl=new URL(`../../build_strict/lab/${elements.preparation.value}_${size}.bin`,import.meta.url);
    const response=await fetch(seedUrl);
    if(!response.ok) throw new Error('Local preparations are missing. Run engine/strict/prepare_lab.py.');
    const checkpoint=new Uint8Array(await response.arrayBuffer());
    if(localEpoch!==epoch) return;
    worker=new Worker(new URL('../../web/js/strict/strict-worker.js',import.meta.url),{type:'module'});
    worker.onmessage=({data})=>{
      if(localEpoch!==epoch) return;
      const item=pending.get(data.requestId);if(!item)return;
      pending.delete(data.requestId);
      if(!data.ok) {item.reject(new Error(data.error));return;}
      if(ownerId!==null&&data.ownerId!==ownerId) {item.reject(new Error('Foreign owner response'));return;}
      if(typeof data.ownerId!=='string'||!data.ownerId||data.generation!==item.expectedGeneration) {
        item.reject(new Error('Invalid response lineage or generation'));return;
      }
      ownerId=data.ownerId;generation=data.generation;item.resolve(data);
    };
    worker.onerror=(event)=>{
      if(localEpoch!==epoch)return;
      fail(new Error(event.message||'Worker failed'),localEpoch);
    };
    await request('init',{wasmModuleUrl:new URL('../../build_strict_wasm/ftd_strict_wasm.mjs',import.meta.url).href,checkpoint});
    await observe(localEpoch);
    if(localEpoch===epoch) elements.status.textContent='Paused. Both resolutions share one state and clock.';
  } catch(error) {fail(error,localEpoch);}
  finally {if(localEpoch===epoch){busy=false;controls();}}
}
async function advance(ticks) {
  if(busy||!ownerId)return;
  const localEpoch=epoch;busy=true;controls();
  try {
    await request('advance',{microticks:String(ticks)});
    await observe(localEpoch);
    if(localEpoch===epoch)elements.status.textContent=running?'Running the finite candidate.':'Paused. One physical microtick completed.';
  } catch(error){fail(error,localEpoch);}
  finally {if(localEpoch===epoch){busy=false;controls();}}
}
elements.reset.onclick=initialize;
elements.size.onchange=initialize;elements.preparation.onchange=initialize;
elements.step.onclick=()=>advance(1);
elements.run.onclick=()=>{running=!running;controls();};
elements.width.onchange=async()=>{
  // Serialize observer changes with any in-flight tick; never advance for zoom.
  if(busy)return;
  const localEpoch=epoch;busy=true;controls();
  try {await observe(localEpoch);}catch(error){fail(error,localEpoch);}
  finally {if(localEpoch===epoch){busy=false;controls();}}
};
function frame(time) {
  if(running&&!busy&&time-lastPublish>=40){lastPublish=time;void advance(4);}
  requestAnimationFrame(frame);
}
new ResizeObserver(()=>{paint(elements.micro,microView);paint(elements.blocks,blockView);}).observe(document.body);
window.addEventListener('pagehide',()=>{
  epoch++;running=false;busy=false;ownerId=null;generation='0';
  terminate();clearPublication();controls();
  elements.status.textContent='Runtime unloaded. Reset to start a fresh preparation.';
});
requestAnimationFrame(frame);
void initialize();

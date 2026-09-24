// @ts-check
// Dynamic URL preserves static serving and avoids importing the vendored bundle into checkJs.
const runtimeUrl=new URL('../vendor/web-llm/0.2.85/index.js',import.meta.url).href;
/** @type {MessageEvent[]} */ const pending=[];
// The proxy sends reload immediately. Queue messages while the dynamic bundle loads.
globalThis.onmessage=(/** @type {MessageEvent} */ event)=>{pending.push(event);};
const {WebWorkerMLCEngineHandler}=await import(runtimeUrl);
const handler=new WebWorkerMLCEngineHandler();
globalThis.onmessage=(/** @type {MessageEvent} */ event)=>handler.onmessage(event);
for(const event of pending)handler.onmessage(event);
pending.length=0;

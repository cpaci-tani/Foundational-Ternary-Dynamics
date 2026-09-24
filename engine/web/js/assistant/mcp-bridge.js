// @ts-check
import {McpControl} from './mcp-control.js';

/** Opt-in, instance-owned connection to the loopback MCP relay. */
export class McpBridge {
    /** @param {{service:import('./service.js').AssistantService,onStatus?:(status:any)=>void,fetchImpl?:typeof fetch}} deps */
    constructor({service,onStatus=()=>{},fetchImpl=fetch}){
        this.control=new McpControl(service);this.onStatus=onStatus;this.fetch=fetchImpl.bind(globalThis);
        /** @type {any} */this.session=null;
        /** @type {AbortController|null} */this.connection=null;
        /** @type {Map<string,AbortController>} */this.pending=new Map();
        this.disposed=false;this.generation=0;
    }
    /** @param {string} path @param {any} body @param {AbortSignal} [signal] */
    async post(path,body,signal){
        const response=await this.fetch(`/api/mcp/${path}`,{method:'POST',headers:{'Content-Type':'application/json','X-FTD-MCP':'1'},body:JSON.stringify(body),signal,cache:'no-store'});
        const data=await response.json();
        if(!response.ok)throw new Error(data.error?.message||'Local MCP service is unavailable. Start the dashboard with serve.py.');
        return data;
    }
    async connect(){
        if(this.disposed)throw new Error('MCP bridge disposed.');
        const generation=++this.generation;
        await this.closeSession();
        if(this.disposed || generation!==this.generation)return;
        if(!['localhost','127.0.0.1','[::1]'].includes(location.hostname))throw new Error('MCP pairing requires the local dashboard server.');
        const connection=new AbortController();this.connection=connection;
        try{
            const session=await this.post('register',{label:'FTD Lattice Sim / Mind’s Eye'},AbortSignal.any([connection.signal,AbortSignal.timeout(10000)]));
            if(connection.signal.aborted || this.disposed || generation!==this.generation || this.connection!==connection){
                try{await this.post('disconnect',{sessionId:session.sessionId,browserToken:session.browserToken},AbortSignal.timeout(3000));}catch{}
                return;
            }
            this.session=session;this.control.rearm();
            const config={mcpServers:{ftd:{command:'node',args:[session.serverPath],env:{FTD_MCP_URL:location.origin,FTD_MCP_SESSION:session.sessionId,FTD_MCP_TOKEN:session.clientToken}}}};
            this.onStatus({connected:true,text:'MCP enabled for this tab · external models can read observations and request JEV-evaluated actions.',config:JSON.stringify(config,null,2)});
            void this.poll(connection,session);
        }catch(error){
            if(this.connection===connection){this.connection=null;this.onStatus({connected:false,text:error instanceof Error?error.message:'MCP pairing failed.',config:''});}
            throw error;
        }
    }
    /** @param {AbortController} connection @param {any} session */
    async poll(connection,session){
        try{
            while(!connection.signal.aborted){
                const data=await this.post('poll',{sessionId:session.sessionId,browserToken:session.browserToken},AbortSignal.any([connection.signal,AbortSignal.timeout(30000)]));
                connection.signal.throwIfAborted();
                for(const id of data.cancelled||[])this.pending.get(id)?.abort(new Error('MCP client cancelled the request.'));
                for(const request of data.requests||[])void this.dispatch(request,connection,session);
            }
        }catch(error){
            if(this.connection===connection){
                await this.disconnect();
                this.onStatus({connected:false,text:`MCP disconnected: ${error instanceof Error?error.message:'relay unavailable'}. Reconnect to continue.`,config:''});
            }
        }
    }
    /** @param {any} request @param {AbortController} connection @param {any} session */
    async dispatch(request,connection,session){
        if(this.connection!==connection || this.pending.has(request.id)||connection.signal.aborted)return;
        const controller=new AbortController();this.pending.set(request.id,controller);
        const signal=AbortSignal.any([controller.signal,connection.signal]);
        const timer=setTimeout(()=>controller.abort(new Error('MCP request deadline reached.')),Math.max(0,Math.min(60000,request.deadline-Date.now())));
        const body={sessionId:session.sessionId,browserToken:session.browserToken,id:request.id};
        try{
            if(!Number.isFinite(request.deadline)||request.deadline<=Date.now())throw new Error('Expired MCP request.');
            const result=await this.control.handle(request.method,request.args,signal);
            await this.post('reply',{...body,result},connection.signal);
        }catch(error){
            if(!connection.signal.aborted)try{await this.post('reply',{...body,error:{code:'BROWSER_COMMAND_FAILED',message:error instanceof Error?error.message:'MCP command failed.'}},connection.signal);}catch{}
        }finally{clearTimeout(timer);if(this.pending.get(request.id)===controller)this.pending.delete(request.id);}
    }
    async disconnect(){
        ++this.generation;
        await this.closeSession();
    }
    async closeSession(){
        const session=this.session;this.session=null;
        this.connection?.abort();this.connection=null;
        for(const request of this.pending.values())request.abort();this.pending.clear();
        this.control.stop();this.onStatus({connected:false,text:'MCP disconnected · no external control.',config:''});
        if(session)try{await this.post('disconnect',{sessionId:session.sessionId,browserToken:session.browserToken},AbortSignal.timeout(3000));}catch{}
    }
    dispose(){this.disposed=true;void this.disconnect();}
}

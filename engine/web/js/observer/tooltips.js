// @ts-check
/** One accessible hover/focus explanation layer for dynamically rendered UI. */
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { observerTooltipContent } from './tooltip-content.js';

export const OBSERVER_TOOLTIP_TARGETS = [
    'button','input','select','summary','.observer-field-label','.observer-section-title',
    '.observer-panel-head h2','.observer-experiment h4','.observer-save strong',
    '.observer-description','.observer-badge','.observer-hud-metric',
    '.observer-telemetry-reading dt','[data-observer-telemetry]','.observer-telemetry-note',
    '.observer-telemetry-count','.observer-telemetry-summary','.observer-readout',
    '.observer-canvas','.observer-reticle','.observer-spatial-label','.observer-title','.observer-eyebrow',
    '.observer-gizmo span','[data-observer-spacetime]','[data-observer-aberration]',
    '.observer-target strong','[data-observer-target-info]','[data-observer-selection]',
    '[data-observer-status]','.observer-worldline-readout','.observer-history-readout',
    '.observer-phenomena-instrument p','.observer-phenomena-instrument h2',
].join(',');

/** @param {HTMLElement} root @param {()=>import('./tooltip-content.js').TooltipContext} getContext */
export function createObserverTooltips(root, getContext) {
    const scope = new LifetimeScope();
    const bubble = document.createElement('div');
    bubble.className = 'observer-tooltip';
    bubble.id = 'observer-explanation';
    bubble.setAttribute('role','tooltip');
    bubble.hidden = true;
    const heading = document.createElement('strong');
    heading.className = 'observer-tooltip-title';
    const text = document.createElement('p');
    text.className = 'observer-tooltip-text';
    const equation = document.createElement('div');
    equation.className = 'observer-tooltip-equation';
    bubble.append(heading,text,equation);root.append(bubble);
    /** @type {HTMLElement|null} */ let active = null;
    /** @type {HTMLElement|null} */ let focused = null;
    /** @type {(()=>void)|null} */ let cancelShow = null;
    /** @type {(()=>void)|null} */ let cancelHide = null;
    /** @type {HTMLElement|null} */ let dismissed = null;
    let inBubble = false;
    /** @type {HTMLElement[]} */ let transparentTargets = [];

    function cancelPending() {cancelShow?.();cancelShow=null;cancelHide?.();cancelHide=null;}
    function hide() {
        cancelPending();
        if(active) {
            const ids=(active.getAttribute('aria-describedby') || '').split(/\s+/).filter(id=>id && id!==bubble.id);
            if(ids.length)active.setAttribute('aria-describedby',ids.join(' '));else active.removeAttribute('aria-describedby');
        }
        active=null;bubble.hidden=true;inBubble=false;
    }
    function position() {
        if(!active || bubble.hidden)return;
        const anchor=active.getBoundingClientRect(),bounds=bubble.getBoundingClientRect(),gap=10,pad=12;
        let left=Math.min(Math.max(pad,anchor.left),Math.max(pad,window.innerWidth-bounds.width-pad));
        const below=anchor.bottom+gap;
        let top=below+bounds.height <= window.innerHeight-pad ? below : Math.max(pad,anchor.top-bounds.height-gap);
        const drawer=active.closest('.observer-panel')?.getBoundingClientRect();
        if(active.matches('.observer-canvas')){
            left=Math.max(pad,window.innerWidth-bounds.width-pad);
            top=Math.max(pad,Math.min(Math.max(230,window.innerHeight*.35),window.innerHeight-bounds.height-pad));
        }
        if(active.closest('.observer-identity,.observer-hud')){
            // Treat adjacent header notes and clock metrics as one hover cluster.
            // A popup below one note must not cover the next note's pointer target.
            const cluster=Array.from(root.querySelectorAll('.observer-identity,.observer-hud')).filter(node=>node.getClientRects().length && getComputedStyle(node).visibility!=='hidden').map(node=>node.getBoundingClientRect());
            const clusterLeft=Math.min(...cluster.map(rect=>rect.left)),clusterRight=Math.max(...cluster.map(rect=>rect.right));
            const clusterTop=Math.min(...cluster.map(rect=>rect.top)),clusterBottom=Math.max(...cluster.map(rect=>rect.bottom));
            if(window.innerWidth-clusterRight>=bounds.width+gap+pad){
                left=clusterRight+gap;top=Math.max(pad,Math.min(anchor.top,window.innerHeight-bounds.height-pad));
            }else if(clusterLeft>=bounds.width+gap+pad){
                left=clusterLeft-bounds.width-gap;top=Math.max(pad,Math.min(anchor.top,window.innerHeight-bounds.height-pad));
            }else{
                top=clusterBottom+gap+bounds.height<=window.innerHeight-pad?clusterBottom+gap:Math.max(pad,clusterTop-bounds.height-gap);
            }
        }
        if(drawer && (drawer.left>=bounds.width+gap+pad || window.innerWidth-drawer.right>=bounds.width+gap+pad)){
            left=drawer.left>=bounds.width+gap+pad?drawer.left-bounds.width-gap:drawer.right+gap;
            top=Math.max(pad,Math.min(anchor.top,window.innerHeight-bounds.height-pad));
        }
        bubble.style.left=left+'px';bubble.style.top=top+'px';
    }
    /** @param {HTMLElement} target */
    function show(target) {
        if(document.pointerLockElement || root.classList.contains('observer-interface-hidden') || !target.isConnected || !target.getClientRects().length || dismissed===target)return;
        cancelPending();
        const source=target.matches('.observer-field-label') && target.closest('label')?.querySelector('input,select');
        const content=observerTooltipContent(source instanceof HTMLElement ? source : target,getContext());
        if(!content.text)return;
        hide();active=target;heading.textContent=content.title;text.textContent=content.text;
        equation.replaceChildren();equation.hidden=!content.math;
        if(content.math) {
            const katex=/** @type {any} */(window).katex;
            let rendered=false;
            if(katex?.render) {
                try {katex.render(content.math.latex,equation,{throwOnError:true,displayMode:true,output:'htmlAndMathml',trust:false});rendered=true;}
                catch { /* Native MathML remains available offline or after a rendering failure. */ }
            }
            if(!rendered) {
                const math=document.createElementNS('http://www.w3.org/1998/Math/MathML','math');
                math.setAttribute('display','block');math.setAttribute('aria-label',content.math.label);
                // Only module-authored MathML enters this branch; user names and notes use textContent.
                math.innerHTML=content.math.mathml;equation.append(math);
            }
        }
        const ids=new Set((target.getAttribute('aria-describedby') || '').split(/\s+/).filter(Boolean));ids.add(bubble.id);
        target.setAttribute('aria-describedby',[...ids].join(' '));bubble.hidden=false;position();
    }
    /** @param {EventTarget|null} target @returns {HTMLElement|null} */
    function targetAt(target) {
        if(!(target instanceof Element) || bubble.contains(target))return null;
        const candidate=target.closest('[data-observer-tooltip]');
        return candidate instanceof HTMLElement && root.contains(candidate) ? candidate : null;
    }
    /** The popup never intercepts clicks; geometric hover keeps its text readable.
     * @param {number} x @param {number} y
     */
    function overBubble(x,y){
        if(bubble.hidden)return false;
        const bounds=bubble.getBoundingClientRect();
        return x>=bounds.left && x<=bounds.right && y>=bounds.top && y<=bounds.bottom;
    }
    /** @param {HTMLElement} target @param {boolean} immediate */
    function request(target,immediate=false) {
        cancelPending();
        if(target!==dismissed)dismissed=null;
        if(immediate)show(target);else cancelShow=scope.timeout(()=>{cancelShow=null;show(target);},target.matches('.observer-canvas')?500:160);
    }
    function leave() {
        cancelShow?.();cancelShow=null;
        cancelHide?.();cancelHide=scope.timeout(()=>{
            cancelHide=null;if(inBubble)return;
            if(focused?.isConnected && focused!==dismissed)show(focused);else hide();
        },100);
    }
    function refresh() {
        transparentTargets=[];
        root.querySelectorAll(OBSERVER_TOOLTIP_TARGETS).forEach(target=>{
            if(!(target instanceof HTMLElement) || bubble.contains(target))return;
            target.dataset.observerTooltip='';
            if(!target.matches('.observer-canvas') && getComputedStyle(target).pointerEvents==='none')transparentTargets.push(target);
            if(target.title){target.dataset.observerTooltipNativeTitle=target.title;target.removeAttribute('title');}
            if(!target.closest('[aria-hidden="true"]') && !target.matches('button,input,select,summary') && !target.matches('.observer-field-label,.observer-telemetry-reading dt,.observer-telemetry-note,.observer-telemetry-count')) {
                if(!target.hasAttribute('tabindex'))target.tabIndex=0;
            }
        });
        if(active && (!active.isConnected || !active.getClientRects().length))hide();
    }
    const observer=new MutationObserver(records=>{
        if(records.some(record=>!bubble.contains(record.target) && [...record.addedNodes,...record.removedNodes].some(node=>node.nodeType===Node.ELEMENT_NODE)))refresh();
    });
    observer.observe(root,{childList:true,subtree:true});
    scope.defer(()=>observer.disconnect());
    scope.on(root,'pointerover',event=>{
        if(overBubble(event.clientX,event.clientY)){inBubble=true;cancelHide?.();cancelHide=null;return;}
        if(bubble.contains(event.target)){inBubble=true;cancelHide?.();cancelHide=null;return;}
        const target=targetAt(event.target);if(target && target!==active)request(target);
    });
    scope.on(root,'pointerout',event=>{
        if(overBubble(event.clientX,event.clientY)){inBubble=true;cancelHide?.();cancelHide=null;return;}
        const next=event.relatedTarget;
        if(next instanceof Node && bubble.contains(next)){inBubble=true;cancelHide?.();cancelHide=null;return;}
        if(bubble.contains(event.target)){inBubble=false;leave();return;}
        const target=targetAt(event.target);
        if(target && !(next instanceof Node && target.contains(next)))leave();
    });
    scope.on(root,'pointermove',event=>{
        if(overBubble(event.clientX,event.clientY)){inBubble=true;cancelHide?.();cancelHide=null;return;}
        if(inBubble){inBubble=false;leave();}
        if(document.pointerLockElement || !(event.target instanceof Element) || !event.target.matches('.observer-canvas'))return;
        const annotations=transparentTargets.map(target=>({target,bounds:target.getBoundingClientRect()})).filter(({target,bounds})=>{
            if(!target.getClientRects().length)return false;
            if(!(event.clientX>=bounds.left && event.clientX<=bounds.right && event.clientY>=bounds.top && event.clientY<=bounds.bottom))return false;
            return typeof target.checkVisibility==='function'?target.checkVisibility({checkVisibilityCSS:true}):getComputedStyle(target).visibility!=='hidden';
        }).sort((a,b)=>a.bounds.width*a.bounds.height-b.bounds.width*b.bounds.height);
        const annotation=annotations[0]?.target;
        if(annotation instanceof HTMLElement && annotation!==active)request(annotation);
        else if(active && transparentTargets.includes(active) && !annotation)leave();
    });
    scope.on(root,'focusin',event=>{focused=targetAt(event.target);if(focused)request(focused,true);});
    scope.on(root,'focusout',event=>{if(focused===targetAt(event.target))focused=null;leave();});
    scope.on(document,'keydown',event=>{
        if(event.key==='Escape' && !bubble.hidden){dismissed=active;hide();event.preventDefault();event.stopImmediatePropagation();}
    },true);
    scope.on(root,'scroll',()=>{
        const target=focused;
        if(target && document.activeElement===target && target!==dismissed){cancelPending();cancelShow=scope.frame(()=>{cancelShow=null;show(target);});}
        else hide();
    },true);
    scope.on(root,'click',()=>hide());
    scope.on(window,'resize',()=>hide());
    scope.on(document,'pointerlockchange',()=>{if(document.pointerLockElement)hide();});
    refresh();
    return {
        refresh,hide,
        inventory:()=>Array.from(root.querySelectorAll(OBSERVER_TOOLTIP_TARGETS)).filter(target=>!bubble.contains(target)).map(target=>({tag:target.tagName,label:target.getAttribute('aria-label') || target.textContent?.trim().slice(0,100) || '',covered:target.hasAttribute('data-observer-tooltip')})),
        dispose(){hide();scope.dispose();bubble.remove();},
    };
}

import { expect } from '@playwright/test';

/** Runs in the browser; excludes closed disclosures and non-rendered ancestors. */
export function collectRenderedTypography() {
    const rows = [];
    for (const element of document.querySelectorAll('body *')) {
        if (!element.checkVisibility({ checkOpacity: true, checkVisibilityCSS: true })) continue;
        if (element.closest('[aria-hidden="true"], [inert]') || element.matches('.katex .vlist-s')) continue;
        const rect = element.getBoundingClientRect();
        if (rect.width < 1 || rect.height < 1 || rect.right <= 0 || rect.bottom <= 0
            || rect.left >= innerWidth || rect.top >= innerHeight) continue;
        let clipped = false;
        for (let parent = element.parentElement; parent; parent = parent.parentElement) {
            const style = getComputedStyle(parent), box = parent.getBoundingClientRect();
            if (/(auto|scroll|hidden|clip)/.test(style.overflowY) && (rect.top >= box.bottom || rect.bottom <= box.top)) clipped = true;
            if (/(auto|scroll|hidden|clip)/.test(style.overflowX) && (rect.left >= box.right || rect.right <= box.left)) clipped = true;
        }
        if (clipped) continue;
        const hasText = [...element.childNodes].some(n => n.nodeType === Node.TEXT_NODE && n.textContent.trim());
        const control = element.matches('button,input,select,textarea');
        const selector = `${element.tagName.toLowerCase()}${element.id ? '#'+element.id : '.'+[...element.classList].slice(0,3).join('.')}`;
        const record = pseudo => rows.push({ selector: selector+(pseudo||''), fontSize: parseFloat(getComputedStyle(element,pseudo).fontSize), rect:rect.toJSON() });
        if (hasText || control || element.matches('svg text')) record(null);
        for (const pseudo of ['::before','::after']) {
            const content=getComputedStyle(element,pseudo).content;
            if (content && !['none','normal','""'].includes(content)) record(pseudo);
        }
    }
    return rows;
}

/** Proves targets fit and receive the pointer; overflow:hidden is no exemption. */
export async function assertControlsReachable(page, selectors, context = '') {
    const report = await page.evaluate(selectors => {
        const failures = [];
        let checked = 0;
        for (const e of document.querySelectorAll(selectors)) {
            if (!e.checkVisibility({checkOpacity:true,checkVisibilityCSS:true}) || e.matches(':disabled,[aria-disabled="true"]')) continue;
            const r = e.getBoundingClientRect();
            if (r.width < 1 || r.height < 1) continue;
            checked++;
            const id=e.id||e.getAttribute('aria-label')||e.textContent.trim().slice(0,40);
            if (r.left < -1 || r.right > innerWidth+1 || r.top < -1 || r.bottom > innerHeight+1) {
                failures.push({id,reason:'outside viewport',rect:r.toJSON()});continue;
            }
            for(const [u,v] of [[.5,.5],[.15,.15],[.85,.85]]) {
                const hit=document.elementFromPoint(r.x+r.width*u,r.y+r.height*v);
                if (hit!==e&&!e.contains(hit)) failures.push({id,reason:'pointer intercepted',at:[u,v],hit:hit?.id||hit?.className});
            }
        }
        return {checked,failures};
    },selectors);
    expect(report.checked, `controls exist ${context}`).toBeGreaterThan(0);
    expect(report.failures, `control reachability ${context}`).toEqual([]);
}

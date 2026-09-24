import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, selectScale0Scenario } from './_helpers.js';

test('diagnostics, charts and grid consume the same energy source and real sample tick', async ({ page }) => {
    await bootDashboard(page);
    await selectScale0Scenario(page, 'flux-pulse');
    await openDockPanel(page, 'diagnostics');
    if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') await page.locator('#btn-play').click();
    await page.waitForFunction(async () => {
        const { telemetryHub: h } = await import('/js/telemetry-hub.js');
        return h.energy.count > 5 && Number.isFinite(h.s0.diag?.dynamicEnergy);
    });
    await page.locator('#btn-play').click();
    const report = await page.evaluate(async () => {
        const { telemetryHub: h } = await import('/js/telemetry-hub.js');
        const panels = await import('/js/ui/panels/index.js');
        const ctx = window.__ftdCtx;
        const table = ctx.diagnosticsPanel.tablesByScale['0'].find(t => t.section.id === 'energy-budget');
        const row = table.section.rows.find(r => r.id === 'energy');
        const chart = ctx.chartsPanel.cards.get('flux-energy').chart;
        const grid = ctx.telemetryGridPanel;
        const entry = grid.charts.get('energy');
        return {
            sameComponents: ctx.diagnosticsPanel instanceof panels.DiagnosticsPanelComponent
                && ctx.chartsPanel instanceof panels.ChartsPanelComponent
                && ctx.telemetryGridPanel instanceof panels.TelemetryGridPanelComponent,
            rowSource: row.source,
            sameBuffer: table.trendBuffers.get('energy') === h.energy
                && chart.hub[chart.series.find(s => s.key === 'energy').buffer] === h.energy
                && grid._resolveBuffer(entry) === h.energy,
            gridGroup: entry.chan.telemetryGroup,
            value: h.energy.last(), diagnostic: h.s0.diag.dynamicEnergy,
            tick: h.energy.getTick(h.energy.count-1), stateTick: h.getScale0TelemetryMeta('diagnostics').tick,
        };
    });
    expect(report.rowSource).toBe('s0.diag.dynamicEnergy');
    expect(report.sameComponents).toBe(true);
    expect(report.sameBuffer).toBe(true);
    expect(report.gridGroup).toBe('diagnostics');
    expect(report.value).toBeCloseTo(report.diagnostic, 4);
    expect(report.tick).toBe(report.stateTick);
});

test('All history plots remain bounded and preserve extremes at their original ticks', async ({ page }) => {
    await bootDashboard(page);
    const result = await page.evaluate(async () => {
        const [{ MultiRingBuffer }, { UPlotChart }, { Sparkline }] = await Promise.all([
            import('/js/telemetry-hub.js'), import('/js/ui/charts/uplot-chart.js'), import('/js/ui/charts/sparkline.js'),
        ]);
        const ring = new MultiRingBuffer(32, ['a','b']);
        for (let i=0;i<20000;i++) ring.push({a:i===777?10000:Math.sin(i),b:i===15000?-10000:Math.cos(i)},i*3);
        const host = document.createElement('div');
        host.style.cssText='position:fixed;left:0;top:0;width:320px;height:180px';
        document.body.appendChild(host);
        const historyControl={ isAll:true, visibleCount:b=>b.count, subscribe:()=>()=>{} };
        const chart=new UPlotChart(host,{id:'long-history-test',hub:ring.views,
            series:[{key:'a',buffer:'a',label:'A',color:'#fff'},{key:'b',buffer:'b',label:'B',color:'#0ff'}],historyControl});
        chart.update();
        const n=chart._lastData.n;
        const peak=Array.from(chart._lastData.yColumns[0]).indexOf(10000);
        const trough=Array.from(chart._lastData.yColumns[1]).indexOf(-10000);
        const peakTick=chart._lastData.xs[peak],troughTick=chart._lastData.xs[trough];
        let commits=0; const original=chart.uplot.setData.bind(chart.uplot);
        chart.uplot.setData=(...args)=>{commits++;return original(...args);};
        for(let i=0;i<100;i++) chart.update();
        chart.destroy(); host.replaceChildren();
        const spark=new Sparkline(host,{buffer:ring.views.a,historyControl});
        spark.update(); const sparkN=spark.uplot.data[0].length;
        let sparkCommits=0; const setData=spark.uplot.setData.bind(spark.uplot);
        spark.uplot.setData=(...args)=>{sparkCommits++;return setData(...args);};
        for(let i=0;i<100;i++) spark.update();
        spark.destroy(); host.remove();
        return {n,peakTick,troughTick,commits,sparkN,sparkCommits,retained:ring.count};
    });
    expect(result.n).toBeLessThanOrEqual(640);
    expect(result.sparkN).toBeLessThanOrEqual(640);
    expect(result.peakTick).toBe(777*3);
    expect(result.troughTick).toBe(15000*3);
    expect(result.commits).toBe(0);
    expect(result.sparkCommits).toBe(0);
    expect(result.retained).toBe(20000);
});

test('Charts skips offscreen cards and resumes them when scrolled into view', async ({ page }) => {
    await page.setViewportSize({width:1280,height:720});
    await bootDashboard(page);
    await openDockPanel(page,'charts');
    await page.locator('#panel-charts .chart-card').first().scrollIntoViewIfNeeded();
    await expect.poll(()=>page.evaluate(()=>[...window.__ftdCtx.chartsPanel.cards.values()].some(c=>c.onScreen))).toBe(true);
    const result=await page.evaluate(()=>{
        const cards=[...window.__ftdCtx.chartsPanel.cards.values()];
        let hiddenCalls=0,visibleCalls=0;
        for(const card of cards){
            const original=card.chart.update;
            card.chart.update=()=>{if(card.onScreen)visibleCalls++;else hiddenCalls++;};
            card.update(); card.chart.update=original;
        }
        const hidden=cards.filter(c=>!c.onScreen);
        if(hidden[0])hidden[0].el.scrollIntoView();
        return {hidden: hidden.length, hiddenCalls,visibleCalls, reveal: hidden[0]?.descriptor.id};
    });
    expect(result.hidden).toBeGreaterThan(0);
    expect(result.visibleCalls).toBeGreaterThan(0);
    expect(result.hiddenCalls).toBe(0);
    await expect.poll(()=>page.evaluate(id=>window.__ftdCtx.chartsPanel.cards.get(id).onScreen,result.reveal)).toBe(true);
});

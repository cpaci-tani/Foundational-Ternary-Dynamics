/** Progressive enhancement: the native select remains the scenario command bus. */
export function enhanceScenarioPicker(select) {
    if (!select || select.dataset.groupPicker) return;
    select.dataset.groupPicker = '1';
    const picker = document.createElement('details');
    picker.id = 'scenario-picker'; picker.className = 'scenario-picker';
    picker.innerHTML = '<summary aria-label="Choose scenario"></summary><div class="scenario-picker-menu"><label>Find scenario<input type="search" class="ctrl-input" placeholder="Name or family"></label><div class="scenario-picker-groups"></div><p class="scenario-picker-empty" hidden>No matching scenarios.</p></div>';
    select.after(picker);
    // Retain programmatic selectOption/value/change integrations and form state.
    select.style.cssText = 'position:absolute;width:1px;height:1px;opacity:0;pointer-events:none';
    select.tabIndex = -1; select.setAttribute('aria-hidden', 'true');
    const summary = picker.querySelector('summary'), search = picker.querySelector('input');
    const groups = picker.querySelector('.scenario-picker-groups');
    const expanded = new Set();
    const sync = () => { summary.textContent = select.selectedOptions[0]?.textContent || 'Choose scenario'; };
    const render = () => {
        const filter = search.value.trim().toLowerCase();
        let matches = 0;
        groups.replaceChildren();
        for (const group of select.children) {
            const options = [...group.children].filter(option => `${group.label} ${option.textContent}`.toLowerCase().includes(filter));
            if (!options.length) continue;
            matches += options.length;
            const section = document.createElement('details');
            section.className = 'scenario-picker-group'; section.open = !!filter || expanded.has(group.label);
            const heading = document.createElement('summary'); heading.textContent = `${group.label} (${options.length})`;
            section.append(heading);
            section.addEventListener('toggle', () => {
                if (!filter && section.isConnected) section.open ? expanded.add(group.label) : expanded.delete(group.label);
            });
            for (const option of options) {
                const button = document.createElement('button'); button.type = 'button';
                button.textContent = option.textContent; button.dataset.scenario = option.value;
                button.disabled = option.disabled; button.title = option.dataset.uiTooltip || '';
                button.setAttribute('aria-pressed', String(select.value === option.value));
                button.onclick = () => {
                    select.value = option.value; select.dispatchEvent(new Event('change', {bubbles: true}));
                    sync(); picker.open = false; summary.focus();
                };
                section.append(button);
            }
            groups.append(section);
        }
        picker.querySelector('.scenario-picker-empty').hidden = matches > 0;
        sync();
    };
    select.addEventListener('change', sync);
    select.addEventListener('scenario-options-changed', render);
    search.addEventListener('input', render);
    picker.addEventListener('toggle', event => { if (event.target === picker && picker.open) { render(); search.focus(); } });
    picker.addEventListener('keydown', event => {
        // Summary/button keyboard activation must not also fire simulation hotkeys.
        event.stopPropagation();
        if (event.key === 'Escape') { event.preventDefault(); picker.open = false; summary.focus(); }
    });
    picker.addEventListener('focusout', event => {
        if (event.relatedTarget && !picker.contains(event.relatedTarget)) picker.open = false;
    });
    if (!document.getElementById('scenario-picker-style')) {
        const style = document.createElement('style'); style.id = 'scenario-picker-style';
        style.textContent = `
            .scenario-picker{position:relative;max-width:360px;min-width:0;flex:0 1 360px}
            .scenario-picker>summary{cursor:pointer;padding:6px 10px;border:1px solid var(--border-light);border-radius:6px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis}
            .scenario-picker-menu{position:absolute;top:calc(100% + 6px);left:0;width:min(480px,calc(100vw - 32px));max-height:min(480px,calc(100dvh - 160px));box-sizing:border-box;display:flex;flex-direction:column;overflow:hidden;z-index:1000;background:var(--bg-deep);color:var(--text-primary);border:1px solid var(--border-light);border-radius:10px;box-shadow:0 8px 30px #0006;white-space:normal;font-size:14px;line-height:1.45}
            .scenario-picker-menu>label{display:block;flex:none;padding:12px;border-bottom:1px solid var(--border-light);color:var(--text-secondary);font-size:12px;letter-spacing:normal;text-transform:none}
            .scenario-picker-menu input{box-sizing:border-box;width:100%;margin:6px 0 0;background:var(--bg-input);color:var(--text-primary)}
            .scenario-picker-menu input::placeholder{color:var(--text-muted)}
            .scenario-picker-groups{overflow-y:auto;overscroll-behavior:contain;min-height:0;padding:4px 6px 8px;scrollbar-gutter:stable}
            .scenario-picker-group>summary{cursor:pointer;padding:9px 8px;font-weight:600;color:var(--text-primary);border-radius:5px;white-space:normal;overflow-wrap:anywhere}
            .scenario-picker-group>button{display:block;box-sizing:border-box;width:100%;text-align:left;padding:8px 12px 8px 24px;background:transparent;color:var(--text-secondary);border:0;border-radius:5px;cursor:pointer;font:inherit;white-space:normal;overflow-wrap:anywhere}
            .scenario-picker-group>summary:hover,.scenario-picker-group>button:hover,.scenario-picker-group>button[aria-pressed=true]{background:var(--bg-input);color:var(--text-primary)}
            .scenario-picker-group>summary:focus-visible,.scenario-picker-group>button:focus-visible{outline:2px solid var(--text-primary);outline-offset:-2px}
            .scenario-picker-empty{margin:0;padding:12px;color:var(--text-muted)}
            @media(max-width:640px){.scenario-picker-menu{position:fixed;left:16px;right:16px;top:auto;width:auto;max-height:55dvh;margin-top:6px}}
        `;
        document.head.append(style);
    }
    render();
}

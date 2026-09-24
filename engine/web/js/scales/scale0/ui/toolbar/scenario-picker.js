/** Progressive enhancement: the native select remains the scenario command bus. */
export function enhanceScenarioPicker(select, {id = 'scenario-picker', label = 'Choose scenario', afterLabel = false} = {}) {
    if (!select || select.dataset.groupPicker) return;
    select.dataset.groupPicker = '1';
    const picker = document.createElement('details');
    picker.id = id; picker.className = 'scenario-picker';
    picker.innerHTML = '<summary aria-label="Choose scenario"></summary><div class="scenario-picker-menu"><label>Find scenario<input type="search" class="ctrl-input" placeholder="Name or family"></label><div class="scenario-picker-groups"></div><p class="scenario-picker-empty" hidden>No matching scenarios.</p></div>';
    (afterLabel ? select.parentElement : select).after(picker);
    // Retain programmatic selectOption/value/change integrations and form state.
    select.classList.add('scenario-picker-native');
    select.tabIndex = -1; select.setAttribute('aria-hidden', 'true');
    const summary = picker.querySelector('summary'), search = picker.querySelector('input');
    summary.setAttribute('aria-label', label);
    summary.addEventListener('click', event => { if (select.disabled) event.preventDefault(); });
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
                    if (select.disabled) return;
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
    picker.addEventListener('toggle', event => { if (event.target === picker && picker.open) {
        if (select.disabled) {picker.open = false; return;}
        render(); search.focus();
    } });
    picker.addEventListener('keydown', event => {
        // Summary/button keyboard activation must not also fire simulation hotkeys.
        event.stopPropagation();
        if (event.key === 'Escape') { event.preventDefault(); picker.open = false; summary.focus(); }
    });
    picker.addEventListener('focusout', event => {
        if (event.relatedTarget && !picker.contains(event.relatedTarget)) picker.open = false;
    });
    render();
}

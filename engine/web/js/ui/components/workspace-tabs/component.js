import { getWorkspaceTabsTemplate } from './template.js';

export class WorkspaceTabsComponent {
    constructor(root, panelDefs = []) {
        this.root = root;
        this.panelDefs = panelDefs;
    }

    init() {
        if (!this.root) return this;
        this.root.innerHTML = getWorkspaceTabsTemplate(this.panelDefs);
        this.root.dataset.workspaceTabs = 'true';
        return this;
    }
}

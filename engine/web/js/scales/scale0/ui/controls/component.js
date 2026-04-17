/**
 * Scale 0 Controls Component
 * Mounts all Scale 0 control cards into the controls panel
 */

import { createPhysicsTogglesCard } from './physics-toggles.js';
import { createSubstrateControlsCard } from './substrate-controls.js?v=2';
import { createFluxVolumeCard } from './flux-volume.js?v=2';

export class Scale0ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Find or create the panel grid that will hold Scale 0 cards.
    // Use the explicit #panel-controls-grid id — a plain `.panel-grid.panel-grid-3`
    // selector also matches the Scale 5 cosmic block inserted by
    // ensurePanelResources(), which is hidden in Scale 0 mode.
    let gridContainer = this.panel.querySelector('#panel-controls-grid');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.id = 'panel-controls-grid';
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    // Mount the three control cards
    gridContainer.appendChild(createPhysicsTogglesCard());
    gridContainer.appendChild(createSubstrateControlsCard());
    gridContainer.appendChild(createFluxVolumeCard());

    return this;
  }
}

/**
 * Scale 2 Controls Component
 * Mounts Atom Engine control cards into the controls panel.
 */

import {
  createAeForcesCard,
  createAeAdvancedCard,
  createAeIntegratorCard,
  createAeNuclearLaboratoryCard,
} from './ae-controls.js';

export class Scale2ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Target the dedicated host. The controls panel also contains hidden
    // Scale-4/5 grids; a generic class selector silently mounted every Scale-2
    // card inside the hidden cosmic block after panel-resource composition.
    let gridContainer = this.panel.querySelector('#panel-controls-grid');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.className = 'panel-grid panel-grid-3';
      gridContainer.id = 'panel-controls-grid';
      this.panel.appendChild(gridContainer);
    }

    gridContainer.appendChild(createAeForcesCard());
    gridContainer.appendChild(createAeAdvancedCard());
    gridContainer.appendChild(createAeIntegratorCard());
    gridContainer.appendChild(createAeNuclearLaboratoryCard());

    return this;
  }
}

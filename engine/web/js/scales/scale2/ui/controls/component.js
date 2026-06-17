/**
 * Scale 2 Controls Component
 * Mounts Atom Engine control cards into the controls panel.
 */

import {
  createAeForcesCard,
  createAeAdvancedCard,
  createAeIntegratorCard,
} from './ae-controls.js';

export class Scale2ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    let gridContainer = this.panel.querySelector('.panel-grid.panel-grid-3');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    gridContainer.appendChild(createAeForcesCard());
    gridContainer.appendChild(createAeAdvancedCard());
    gridContainer.appendChild(createAeIntegratorCard());

    return this;
  }
}

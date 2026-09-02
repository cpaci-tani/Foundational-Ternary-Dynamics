/**
 * Scale 1 Controls Component
 * Mounts the Scale 1 (Particle Engine) control card into the controls panel.
 */

import { createPeControlsCard, createPePhysicsCard } from './pe-controls.js?v=16';

export class Scale1ControlsComponent {
  constructor(panelControlsDiv) {
    this.panel = panelControlsDiv;
  }

  init() {
    if (!this.panel) return this;

    // Use the explicit root. A generic panel-grid selector also matches the
    // hidden Scale-5 control block inserted by ensurePanelResources().
    let gridContainer = this.panel.querySelector('#panel-controls-grid');
    if (!gridContainer) {
      gridContainer = document.createElement('div');
      gridContainer.id = 'panel-controls-grid';
      gridContainer.className = 'panel-grid panel-grid-3';
      this.panel.appendChild(gridContainer);
    }

    if (!gridContainer.querySelector('[data-scale1-control-card="context"]')) {
      const contextCard = createPeControlsCard();
      contextCard.dataset.scale1ControlCard = 'context';
      gridContainer.appendChild(contextCard);
    }

    if (!gridContainer.querySelector('[data-scale1-physics-card]')) {
      const physicsCard = createPePhysicsCard();
      physicsCard.dataset.scale1PhysicsCard = '1';
      physicsCard.dataset.scale1ControlCard = 'physics';
      gridContainer.appendChild(physicsCard);
    }

    return this;
  }
}

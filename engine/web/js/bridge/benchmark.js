import { CosmicMockBridge } from './mock-scale5.js';

const bridge = new CosmicMockBridge();
bridge.setupScenario('cosmic-galaxy'); 
console.log('Bodies:', bridge._bodies.length);

const start = performance.now();
for(let i=0; i<30; i++) { 
    bridge.run(5);
}
const diff = performance.now() - start;
console.log('Took', diff.toFixed(2), 'ms for 150 ticks');
console.log('Average per tick:', (diff/150).toFixed(2), 'ms');

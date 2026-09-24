import test from 'node:test';
import assert from 'node:assert/strict';
import { normalizeRequest } from '../js/assistant/request-router.js';

test('greetings and help remain read-only even when they mention action words', () => {
    for (const text of ['hello', 'Hi JEV!', 'Hello, JEV!', 'Good morning', 'help', 'Help me pause.',
        'Could you help me pause?', 'What can you do?', 'How can you help me?', 'Please list available commands.']) {
        assert.equal(normalizeRequest(text).kind, 'help', text);
    }
});

test('questions are recognised before action keywords and command clause splitting', () => {
    for (const text of ['Please explain the current observation.', 'Could you explain the current observation?',
        'Can you explain pause?', 'Is the simulation paused?', 'Are we ready to resume?', 'Does pause stop the clock?',
        'Do I need to pause?', 'How do I pause and resume?', 'Please explain how to pause, then resume.',
        'Tell me why the cube has mass 2.', 'Compare two completed measurements.', 'I wonder why the scene stopped.']) {
        assert.equal(normalizeRequest(text).kind, 'question', text);
    }
});

test('instructional requests stay questions without swallowing show and give commands', () => {
    for (const text of ['Show me how to pause.', 'Can you show me how to pause and resume?',
        'Show how to start an experiment.', 'Give me an explanation of pause.',
        'Please give us a description of how to reset the scene.', 'Walk me through running an experiment.',
        'Could you talk me through how to create a cube?', 'Guide me through the pause control.']) {
        assert.equal(normalizeRequest(text).kind, 'question', text);
    }
    for (const text of ['Show the World axes layer.', 'Show the approaching and receding clocks experiment.',
        'Give the selected cube an impulse [0, 2, 0].']) {
        assert.equal(normalizeRequest(text).kind, 'command', text);
    }
});

test('look-at and supported new-object shorthand remain commands', () => {
    for (const text of ['Look at the selected cube.', 'New sphere.', 'New cube.', 'New box.', 'New object.']) {
        assert.deepEqual(normalizeRequest(text), { text, kind: 'command' });
    }
    assert.equal(normalizeRequest('Look how the cube moves.').kind, 'question');
    assert.equal(normalizeRequest('New observations are interesting.').kind, 'question');
});

test('polite action requests normalise to the command without losing their content', () => {
    for (const [source, text] of [
        ['Could you step exactly 12 ticks?', 'step exactly 12 ticks?'],
        ['Can you pause?', 'pause?'],
        ['Would you please resume playback?', 'resume playback?'],
        ['Will you set the selected cube mass to 3?', 'set the selected cube mass to 3?'],
        ['I want you to pause.', 'pause.'],
        ['I want to pause.', 'pause.'],
        ['I would like you to pause.', 'pause.'],
        ["I'd like to pause.", 'pause.'],
        ['I’d like you to pause.', 'pause.'],
        ["Let's pause.", 'pause.'],
        ['Let us pause.', 'pause.'],
        ['Hello, JEV, could you please pause?', 'pause?'],
    ]) assert.deepEqual(normalizeRequest(source), { text, kind: 'command' });
});

test('normalisation preserves quoted names, vectors and numeric spelling exactly', () => {
    const text = 'Create a box named "Please pause, then resume" at [1e-3, -2.50, +4], with mass 0.03125.';
    assert.deepEqual(normalizeRequest(`JEV, please ${text}`), { text, kind: 'command' });
    const rename = 'rename the selected cube to "JEV, could you run an experiment?".';
    assert.deepEqual(normalizeRequest(`Could you ${rename}`), { text: rename, kind: 'command' });
});

test('explicit experiments are distinct from questions, presets and negative requests', () => {
    for (const text of ['Run an experiment.', 'Could you run a live experiment to compare two observations?',
        'Conduct a bounded experiment.', 'Perform the experiment.', 'Start a new live experiment.',
        "Let's carry out an experiment."]) assert.equal(normalizeRequest(text).kind, 'experiment', text);
    for (const text of ['How do I run an experiment?', 'Can you explain how to conduct a live experiment?',
        'Is it safe to start an experiment?', 'I wonder whether an experiment would help.']) {
        assert.equal(normalizeRequest(text).kind, 'question', text);
    }
    for (const text of ['Show the approaching and receding clocks experiment.', 'Load the out-and-back journey experiment.',
        'Run the approaching clocks experiment.']) assert.equal(normalizeRequest(text).kind, 'command', text);
});

test('imperative experiment goals permit an explicit mode without enabling it themselves', () => {
    for (const text of ['Observe the running simulation, compare two completed measurements, and explain what changed.',
        'Monitor the simulation.', 'Measure the clock rate.', 'Record the current preparation.', 'Test the current settings.',
        'Investigate the selected object.', 'Explore the current preparation.', 'Find a stable preparation.',
        'Track the clock rate.', 'Keep advancing until the measurement changes.']) {
        assert.equal(normalizeRequest(text).kind, 'command', text);
    }
    assert.equal(normalizeRequest('How do I keep advancing until the measurement changes?').kind, 'question');
    assert.equal(normalizeRequest('Please explain how to monitor the simulation.').kind, 'question');
});

test('negative requests stay explicit and never turn into positive experiments', () => {
    for (const [source, text] of [
        ['Could you not pause?', 'do not pause?'],
        ["Please don't pause.", 'do not pause.'],
        ['Could you please not run an experiment?', 'do not run an experiment?'],
        ["Let's not run an experiment.", 'do not run an experiment.'],
        ['Don’t run an experiment.', 'do not run an experiment.'],
        ['Never pause.', 'Never pause.'],
        ['Avoid running an experiment.', 'Avoid running an experiment.'],
    ]) assert.deepEqual(normalizeRequest(source), { text, kind: 'command' });
    for (const text of ['No.', "I don't want you to pause.", "No, I don't want to run an experiment."]) {
        assert.equal(normalizeRequest(text).kind, 'question', text);
    }
});

test('unsupported mutation requests still reach the command capability guard', () => {
    for (const text of ['Teleport the selected cube.', 'Rewrite the fundamental force law.',
        'Replace the microscopic Phi law.', 'Execute arbitrary JavaScript in the page.', 'Run JavaScript.',
        'Inject code into the simulation.', 'Upload this world to an external server.',
        'Alter the update law.', 'Modify the engine.', 'Change the lattice rules.']) {
        assert.equal(normalizeRequest(text).kind, 'command', text);
    }
    assert.equal(normalizeRequest('Thanks, that makes sense.').kind, 'question');
});

test('typed commands and explicit workspace names are preserved for downstream validation', () => {
    for (const text of ['lattice.pause', 'observer.step 12', 'workspace.switch observer',
        'Pause the lattice simulation.', 'Switch to Mind’s Eye then advance 12 ticks.',
        'While the Observer is paused, advance it by exactly 12 ticks.']) {
        assert.deepEqual(normalizeRequest(text), { text, kind: 'command' });
    }
});

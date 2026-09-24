// @ts-check
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */

/** Examples only for the active owner's advertised capabilities.
 * @param {ObservationEnvelope} observation
 */
export function commandExamples(observation){
    const examples=[];const has=(/** @type {string} */ type)=>observation.capabilities.includes(type);
    if(has(`${observation.workspace}.pause`))examples.push('Pause the simulation.');
    if(has(`${observation.workspace}.step`))examples.push('Advance 10 ticks.');
    if(has('observer.create'))examples.push('Create a box.');
    if(has('observer.update') && observation.selected?.current?.alive)examples.push('Make the selected object twice as heavy.');
    if(has('lattice.catalog'))examples.push('Search the scenario catalog.');
    if(has('workspace.switch'))examples.push(observation.workspace==='lattice'?'Switch to Mind’s Eye.':'Switch to Lattice Sim.');
    return examples.slice(0,5);
}

/** @param {ObservationEnvelope|null|undefined} observation @param {{ready?:boolean,connected?:boolean}} [state] */
export function assistantHelp(observation,state={}){
    const workspace=observation?.workspace==='observer'?'Mind’s Eye':observation?.workspace==='lattice'?'Lattice Sim':'no active simulation';
    const examples=observation?commandExamples(observation):[];
    return[`You are in ${workspace}. You can ask a question, give a command, or request a live experiment.`,
        'For a question, try “Please explain the current observation.”',
        ...(examples.length?[`Commands available here:\n${examples.map(text=>`• ${text}`).join('\n')}`]:['Open Lattice Sim or Mind’s Eye to control its simulation.']),
        'To start a bounded experiment, say “Run an experiment to advance 10 ticks and compare the measurements,” or enable Live experiment.',
        ...(state.ready===false?['Choose “Download / load model” under Connection & local model to enable language inference.']:[]),
        ...(state.connected===false?['Connect a JEV key to execute commands or experiments. Questions and documentation search do not require a key.']:[])
    ].join('\n\n');
}

/** @param {string} text @param {ObservationEnvelope} observation */
export function unavailableCommand(text,observation){
    const workspace=observation.workspace==='observer'?'Mind’s Eye':'Lattice Sim';
    const objectRequest=observation.workspace==='lattice' && /\b(cube|box|sphere|object|gravity|tether|force gun|camera|fov)\b/i.test(text);
    const examples=commandExamples(observation);
    return objectRequest
        ?`You are in ${workspace}. Object, gravity and force-gun controls belong to Mind’s Eye. Say “Switch to Mind’s Eye” first, then select the object you want to edit.`
        :`I could not map that request to a supported ${workspace} action. ${examples.length?`Available examples: ${examples.map(s=>`“${s}”`).join(' ')}`:'This owner currently advertises no editable controls.'} Ask “What can you do?” for help, or start an experiment with “Run an experiment to …”.`;
}

/** Explicitly naming another workspace cannot silently mutate the active one.
 * @param {string} text @param {ObservationEnvelope} observation
 */
export function workspaceMismatch(text,observation){
    if(/^(?:switch|open|return|go)\b/i.test(text))return null;
    // Quoted object names are data, not workspace qualifiers.
    const scope=text.replace(/["“][^"”]*["”]/g,'');
    const lattice=/\blattice(?: sim(?:ulation)?)?\b/i.test(scope);
    const observer=/\b(?:mind['’]?s eye|observer)\b/i.test(scope);
    if(lattice && !observer && observation.workspace!=='lattice')return 'This command names Lattice Sim, but Mind’s Eye is active. Say “Switch to Lattice Sim” first. No action was taken.';
    if(observer && !lattice && observation.workspace!=='observer')return 'This command names Mind’s Eye, but Lattice Sim is active. Say “Switch to Mind’s Eye” first. No action was taken.';
    return null;
}

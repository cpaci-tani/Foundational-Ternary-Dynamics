/**
 * FAQ sidebar data — 16 canonical hard problems framed through the FTD lens.
 * See docs/superpowers/specs/2026-04-18-faq-panel-design.md.
 *
 * Tag vocabulary for ftdAngle bullets:
 *   THEOREM     rigorously proven from FTD axioms
 *   SELECTION   argued from consistency, not uniquely proven
 *   PARAMETRIC  SM formula with FTD numbers inserted
 *   CONJECTURE  proposed interpretation requiring validation
 *   OPEN        framed by FTD but unresolved
 */

export const FAQ_TAGS = Object.freeze(['THEOREM', 'SELECTION', 'PARAMETRIC', 'CONJECTURE', 'OPEN']);

export const FAQ_SECTIONS = Object.freeze([
    {
        id: 'physics',
        title: 'Physics',
        description: 'Twelve hard problems of modern physics, framed through the FTD lens.',
        entries: [
            {
                id: 'hard-problem-consciousness',
                question: 'Why is there subjective experience at all?',
                shortQuestion: 'The hard problem of consciousness',
                problem: [
                    'Chalmers\' "hard problem": even if every functional brain process were fully modeled, the question "why is there something it is like to be that system?" remains untouched. No combination of information-processing steps seems, on its face, to entail experience.',
                ],
                mainstreamStruggle: [
                    'Integrated Information Theory, Global Workspace, Higher-Order-Thought, and related frameworks characterize neural correlates of consciousness. They are models of when experience occurs, not explanations of why there is experience at all. The explanatory gap remains open.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Consciousness enters as the phase angle theta_C of the master-quadratic consciousness extension. The observable fraction cos^2(theta_C) = G*/8 follows from the ternary-state algebra.' },
                    { tag: 'CONJECTURE', text: 'The sLoop self-reference ring is proposed as the structural locus where reference closes on itself — a candidate substrate for subjective experience, not an explanation of qualia.' },
                    { tag: 'OPEN', text: 'FTD offers a geometry for where consciousness could live in the formalism; it does not derive qualia from that geometry.' },
                ],
                stillOpen: [
                    'No operational test distinguishes the "FTD consciousness phase" from a purely functional account.',
                    'The relationship between the sLoop structure and the subjective quality of experience is asserted, not derived.',
                ],
                theoryRefs: [
                    'docs/theory/06_consciousness/FOUND_CONSCIOUSNESS_ONTOLOGY.md',
                    'docs/theory/06_consciousness/DERIV_CONSCIOUSNESS_FRACTION.md',
                ],
            },
        ],
    },
    {
        id: 'foundations',
        title: 'Foundations',
        description: 'Four foundational questions about existence, time, and the shape of reality.',
        entries: [
            {
                id: 'why-anything-exists',
                question: 'Why does anything exist at all?',
                shortQuestion: 'Why something rather than nothing',
                problem: [
                    'Leibniz\'s question: why is there something rather than nothing? Any physical theory that starts from existing objects (fields, particles, spacetime) presupposes the very thing it was supposed to explain.',
                ],
                mainstreamStruggle: [
                    'Modern physics typically brackets this question as outside the domain of empirical science. "Quantum fluctuations produced the universe" still begs for the quantum vacuum itself to exist. No mainstream framework derives existence from a principle more primitive than existence.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'FTD starts from three primitive states {-1, 0, +1} with no prior structure — a minimal asymmetry. The ternary choice itself is argued (not proven) to be the smallest self-consistent ontology supporting nontrivial dynamics.' },
                    { tag: 'CONJECTURE', text: 'The "void" state 0 is interpreted not as absence but as balanced potential — the dispositional ground from which manifestation emerges. Existence is then the recurrence of imbalance.' },
                    { tag: 'OPEN', text: 'FTD moves the question one step: why the ternary structure rather than a unary or binary one? The selection argument constrains but does not eliminate the choice.' },
                ],
                stillOpen: [
                    'The ternary postulate is primitive. FTD explains what follows from it, not why it holds.',
                    'No meta-theory distinguishes "FTD is the ontology" from "FTD is one ontology among many possible minimal ones".',
                ],
                theoryRefs: [
                    'docs/theory/02_foundations/FOUND_ONTOLOGICAL_EMERGENCE.md',
                ],
            },
        ],
    },
]);

function _validate(sections) {
    const required = ['problem', 'mainstreamStruggle', 'ftdAngle', 'stillOpen'];
    for (const section of sections) {
        if (!Array.isArray(section.entries) || section.entries.length === 0) {
            throw new Error(`FAQ section '${section.id}' has no entries`);
        }
        for (const entry of section.entries) {
            for (const field of required) {
                const value = entry[field];
                if (!Array.isArray(value) || value.length === 0) {
                    throw new Error(`FAQ entry '${entry.id}' missing non-empty '${field}' array`);
                }
            }
            for (let i = 0; i < entry.ftdAngle.length; i++) {
                const bullet = entry.ftdAngle[i];
                if (!bullet || typeof bullet !== 'object') {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] is not an object`);
                }
                if (!FAQ_TAGS.includes(bullet.tag)) {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] tag '${bullet.tag}' not in ${FAQ_TAGS.join(',')}`);
                }
                if (typeof bullet.text !== 'string' || !bullet.text.trim()) {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] missing text`);
                }
            }
        }
    }
}
_validate(FAQ_SECTIONS);

export function getFaqSections() {
    return FAQ_SECTIONS;
}

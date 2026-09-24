// @ts-check

/** @typedef {'help'|'question'|'command'|'experiment'} RequestKind */

const commandHead = /^(?:pause|stop|freeze|halt|resume|start|play|continue|advance|step|tick|set|apply|show|hide|load|create|spawn|add|delete|remove|restore|undelete|rename|disable|enable|turn|switch|open|return|reset|undo|select|deselect|clear|make|give|double|halve|triple|move|translate|rotate|resize|scale|increase|decrease|reduce|change|alter|modify|replace|rewrite|teleport|inject|eval|evaluate|execute|run|conduct|perform|carry\s+out|upload|download|export|import|save|search|seed|preview|push|kick)\b/i;
// These goals may use the user's explicit Live experiment mode, but do not
// enable that mode merely by matching an observational or research verb.
const goalHead = /^(?:observe|watch|monitor|measure|record|test|investigate|explore|find|track|keep)\b/i;
const questionHead = /^(?:why|what|how|when|where|which|who|is|are|was|were|does|do|did|has|have|had|should|could|would|can|will|may|might|explain|describe|compare|tell\s+me|let\s+me\s+know)\b/i;
const instructionalQuestionHead = /^(?:(?:show|teach)(?:\s+(?:me|us))?\s+(?:how|why|what|when|where|which|whether)|give\s+(?:me|us)\s+(?:(?:a|an|the)\s+)?(?:explanation|description|summary|overview|walkthrough|tutorial)|(?:walk|talk|guide)\s+(?:me|us)\s+through)\b/i;
const experimentHead = /^(?:run|conduct|perform|start|carry\s+out)\s+(?:(?:a|an|the)\s+)?(?:(?:live|bounded|new)\s+)*experiments?\b/i;

/** Imperative research goals require explicit experiment mode, not incidental action keywords.
 * @param {string} text Already-normalized request text.
 */
export function isExperimentGoal(text) { return goalHead.test(text); }

/**
 * Route intent before selecting action keywords or splitting command clauses.
 * This recognises request forms only: it does not grant capabilities or infer
 * an action from conversational text. Only leading conversational wrappers are
 * removed, so quoted names, quantities and the rest of a command stay intact.
 *
 * @param {string} source
 * @returns {{text:string,kind:RequestKind}}
 */
export function normalizeRequest(source) {
    let text = source.trim();
    // Peel a finite number of prefix wrappers; each successful pass shortens
    // the input. In particular, never replace words inside an object's name.
    for (let pass = 0; pass < 12; pass++) {
        const before = text;
        text = text.replace(/^(?:hello|hi|hey|greetings|good\s+(?:morning|afternoon|evening))(?:\s+jev)?(?:\s*[,!:]\s*|\s+(?=please\b|(?:can|could|would|will)\s+you\b|jev\b))/i, '');
        text = text.replace(/^jev(?:\s*[,!:]\s*|\s+)/i, '');
        text = text.replace(/^please\b[\s,:]*/i, '');
        text = text.replace(/^(?:can|could|would|will)\s+you\b[\s,:]*/i, '');
        text = text.replace(/^i\s+want(?:\s+you)?\s+to\b\s*/i, '');
        text = text.replace(/^i(?:\s+would|['’]d)\s+like(?:\s+you)?\s+to\b\s*/i, '');
        text = text.replace(/^let(?:['’]s|\s+us)\b[\s,:]*/i, '');
        text = text.trim();
        if (text === before) break;
    }

    if (!text || /^(?:(?:hello|hi|hey|greetings|good\s+(?:morning|afternoon|evening))(?:\s+jev)?|jev)[.!?]*$/i.test(text)
        || /^help\b/i.test(text)
        || /^(?:what\s+can\s+you\s+do|how\s+can\s+you\s+help(?:\s+me)?|(?:show|list)(?:\s+me)?\s+(?:(?:the|your|available|supported)\s+)*(?:commands|capabilities|controls|help))[.!?]*$/i.test(text)) {
        return { text, kind: 'help' };
    }

    // A polite negative request must retain a form the command guard rejects.
    // It must never become a positive experiment or command by prefix removal.
    text = text.replace(/^not\b\s*/i, 'do not ');
    text = text.replace(/^don[’']t\b/i, 'do not');
    if (/^(?:do\s+not|never|avoid)\b/i.test(text)) return { text, kind: 'command' };
    if (questionHead.test(text) || instructionalQuestionHead.test(text)) return { text, kind: 'question' };
    if (experimentHead.test(text)) return { text, kind: 'experiment' };
    if (commandHead.test(text) || goalHead.test(text) || /^(?:look\s+at|new\s+(?:sphere|cube|box|object))\b/i.test(text)
        || /^(?:lattice|observer|workspace)\.[a-z][\w.-]*\b/i.test(text)
        || /^while\b[\s\S]*[,;]\s*(?:pause|advance|step|set|apply|resume)\b/i.test(text)) {
        return { text, kind: 'command' };
    }
    return { text, kind: 'question' };
}

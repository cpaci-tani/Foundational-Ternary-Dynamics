# Restatement Template

Use this template for outputs from the Restatement or Re-derivation agents. Place outputs in a `restatements/` directory keyed by finding ID.

```markdown
# Restatement for Finding <finding_id>

**Agent**: Restatement | Re-derivation
**Run date**: <YYYY-MM-DD>
**Source artifact**: <path>
**Original audit file**: <path>
**Triage action**: RESTATE | RE-DERIVE | RETRACT

## Original passage

> <verbatim quote>

## Proposed replacement

<new text; multiple paragraphs permitted>

## Diff explanation

- **Change**: <what specifically changed>
- **Rule addressed**: <which proscribed-moves rule this resolves>
- **Equivalence**: EXACT | WEAKER | STRONGER
- **Content preserved**: <what survives>
- **Content altered**: <any difference>
- **Proof implications**: <does this change what the passage's proof requires?>

## Context note for user review

<one or two sentences; what should the user particularly verify>

## For re-derivations only

### Axioms invoked
- <axiom>
- <axiom>

### Prior theorems invoked
- <theorem, with current tag and location>

### Proof sketch
1. <step with justification>
2. ...

### Finitary verification
<confirm no proscribed moves in the new proof>

### New epistemic tag
THEOREM | SELECTION PRINCIPLE | HYPOTHESIS | CONJECTURE

## Retraction only

### What is removed
<verbatim passages being retracted>

### Surrounding adjustments needed
- <location>: <adjustment>

### Downstream citations that need updating
- <citation source>: <proposed replacement or deletion>

## Self-check
- Replacement uses only permitted language per canonical doc: YES | NO
- Equivalence claim is honest: YES | NO
- Proof implications are flagged: YES | NO
- Context note is present: YES | NO
```

## Summary

<!-- Briefly describe your changes -->

## Type of Change

- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Physics derivation or correction
- [ ] Manuscript content update

## Checklist

### General
- [ ] My code follows the project's style guidelines
- [ ] I have performed a self-review of my changes
- [ ] I have commented my code where necessary
- [ ] My changes generate no new warnings

### For Physics/Theory Changes
- [ ] Epistemic tags are correct ([AXIOM], [THEOREM], [SELECTION], [CONJECTURE], [NUMEROLOGY])
- [ ] Derivations are documented in CLAUDE.md or appropriate docs
- [ ] Unit tests verify numerical accuracy
- [ ] Cross-references to related sections are updated

### For Manuscript Changes
- [ ] Figures have alt text for accessibility
- [ ] Mathematical notation is consistent with [docs/reference/REF_SYMBOL_GLOSSARY.md](docs/reference/REF_SYMBOL_GLOSSARY.md)
- [ ] Cross-references and citations are valid
- [ ] Quarto build succeeds locally

### For Code Changes
- [ ] Tests pass locally (`pytest`)
- [ ] Linting passes (`ruff check .`)
- [ ] Dependencies are properly pinned in requirements.txt

## Testing

<!-- Describe how you tested your changes -->

## Related Issues

<!-- Link to related issues: Fixes #123, Related to #456 -->

## Additional Notes

<!-- Any additional context or screenshots -->

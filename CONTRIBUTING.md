# Contributing to Foundational Ternary Dynamics

Thank you for your interest in contributing to FTD! This document provides guidelines for contributions.

## Types of Contributions

### Bug Reports
- Use GitHub Issues to report bugs
- Include Python version, OS, and full error traceback
- Provide minimal reproducible example if possible

### Feature Requests
- Open an issue describing the proposed feature
- Explain how it fits within the FTD framework
- Consider epistemic implications

### Code Contributions
- Fork the repository
- Create a feature branch
- Follow the code style guidelines below
- Submit a pull request

### Documentation
- Corrections to derivations
- Improved explanations
- Additional examples

## Code Style

### Python
- Follow PEP 8
- Use type hints where practical
- Run `ruff check .` and `black .` before committing

### Epistemic Labels
All claims must be properly labeled:

```python
# [THEOREM] - Proven from axioms
alpha_inv = 137.0361714582  # From master quadratic

# [SELECTION] - Argued from consistency
sin2_theta_w = N_c / n_eff  # = 3/13 = 0.2308

# [NUMEROLOGY] - Pattern without rigorous derivation
lambda_cabibbo = sqrt(2 * sin2_theta_w * alpha_s)  # 3.7% error

# [OPEN] - Unresolved question
# TODO: Derive e-folding number from first principles
```

### Documentation
- Use Markdown for documentation
- Include mathematical notation in LaTeX format
- Reference related sections with cross-links

## Development Setup

```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/Foundational-Ternary-Dynamics.git
cd Foundational-Ternary-Dynamics

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
pip install -r requirements-dev.txt
```

## Testing

```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=. --cov-report=term-missing

# Run specific test file
pytest tests/test_master_quadratic.py -v
```

## Pull Request Process

1. Update documentation for any changed functionality
2. Ensure all tests pass
3. Update CHANGELOG.md if appropriate
4. Ensure epistemic labels are correct
5. Request review from maintainers

## Epistemic Standards

FTD maintains high epistemic standards. When contributing:

1. **Don't overclaim**: If something is pattern-matching, label it [NUMEROLOGY]
2. **Acknowledge limitations**: Document what is NOT proven
3. **Provide derivations**: Show your work, don't just assert results
4. **Consider falsification**: What would disprove your contribution?

## Code of Conduct

- Be respectful and constructive
- Focus on the science, not personalities
- Extraordinary claims require extraordinary evidence
- It's okay to be wrong - that's how science progresses

## Questions?

Open an issue with the "question" label or contact the maintainers.

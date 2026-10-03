"""The hosted boundary defers only explicit functions and leaves opt-in honest."""

from pathlib import Path
from types import SimpleNamespace
import unittest

import pytest_budget as B


class BudgetTests(unittest.TestCase):
    def test_all_manifest_functions_exist_and_have_reasons(self):
        root = Path(__file__).resolve().parents[2]
        self.assertGreater(len(B.read_manifest(root)), 0)

    def test_only_exact_function_and_all_its_parameters_are_deferred(self):
        root = Path(__file__).resolve().parents[2]
        path = root / "scripts/tests/test_example.py"
        items = [SimpleNamespace(path=path, originalname=name, name=f"{name}[{i}]")
                 for i, name in enumerate(("test_full", "test_full", "test_unit", "test_fuller"))]
        kept, excluded, counts = B.select(items, root, {"scripts/tests/test_example.py::test_full": "matrix"})
        self.assertEqual(kept, items[2:])
        self.assertEqual(excluded, items[:2])
        self.assertEqual(counts, {"scripts/tests/test_example.py::test_full": 2})

    def test_default_collection_is_unfiltered(self):
        items = [object()]
        config = SimpleNamespace(getoption=lambda name: False)
        B.pytest_collection_modifyitems(config, items)
        self.assertEqual(len(items), 1)
        self.assertFalse(hasattr(config, "_ftd_ci_budget"))


if __name__ == "__main__":
    unittest.main()

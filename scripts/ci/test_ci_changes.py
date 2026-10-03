"""Test routing boundaries and conservative reuse of successful PR checks."""

import unittest
import re
from pathlib import Path
from unittest.mock import patch

import ci_changes as C


class ChangesTests(unittest.TestCase):
    def test_web_changes_do_not_start_python_or_native_jobs(self):
        flags = C.classify(["engine/web/js/observer/compact-star.js", "engine/web/docs/REF_STAR.md"])
        self.assertTrue(flags["web"])
        self.assertTrue(flags["integrity"])
        self.assertFalse(flags["python"])
        self.assertFalse(flags["lint"])
        self.assertFalse(flags["engine"])

    def test_python_dependency_and_reference_changes(self):
        for path in ("scripts/constants.py", "requirements.txt", "pyproject.toml",
                     "docs/theory/01_reference/dimensional_map.json"):
            with self.subTest(path=path):
                self.assertTrue(C.classify([path])["python"])

    def test_open_and_resolved_tracker_reference_changes_run_python_checks(self):
        for name in ("TRACKER_OPEN_ITEMS.md", "TRACKER_RESOLVED_ITEMS.md", "TRACKER_OPEN_ITEMS_INDEX.md"):
            with self.subTest(name=name):
                flags = C.classify([f"docs/theory/07_assessment/core_ledgers/{name}"])
                self.assertTrue(flags["python"])
                self.assertTrue(flags["integrity"])

    def test_web_python_and_shared_constants_run_python_consumers(self):
        for path in ("engine/web/server_controls.py", "engine/web/serve.py", "engine/web/nested/helper.py"):
            with self.subTest(path=path):
                flags = C.classify([path])
                self.assertTrue(flags["python"])
                self.assertTrue(flags["lint"])
                self.assertTrue(flags["web"])
                self.assertFalse(flags["engine"])
        flags = C.classify(["engine/web/js/constants.js"])
        self.assertTrue(flags["python"])
        self.assertTrue(flags["web"])
        self.assertFalse(flags["engine"])

    def test_strict_native_changes_run_their_python_regressions(self):
        flags = C.classify(["engine/strict/src/strict_cli.cpp", "engine/strict/CMakeLists.txt"])
        self.assertTrue(flags["python"])
        # The conventional CPU gate does not build the isolated strict engine.
        self.assertFalse(flags["engine"])

    def test_source_gate_and_classifier_ownership(self):
        self.assertTrue(C.classify(["engine/tests/support/bridge_fixtures.cpp"])["engine"])
        self.assertTrue(C.classify(["scripts/ci/ci_changes.py"])["web"])
        self.assertTrue(all(C.classify(["scripts/ci/ci_changes.py"]).values()))
        self.assertTrue(C.classify([".github/workflows/web-source-gate.yml"])["web"])

    def test_every_source_workflow_trigger_has_a_routed_representative(self):
        root = Path(__file__).resolve().parents[2]
        for workflow, scope in (("web-source-gate.yml", "web"), ("engine-source-gate.yml", "engine")):
            source = (root / ".github/workflows" / workflow).read_text(encoding="utf-8")
            patterns = set(re.findall(r"^\s+- '([^']+)'\s*$", source, re.MULTILINE))
            self.assertGreater(len(patterns), 0)
            for pattern in patterns:
                representative = ("scripts/ci/ci_changes.py" if pattern == "scripts/ci/**"
                                  else pattern.replace("**", "nested/fixture").replace("*", "fixture"))
                with self.subTest(workflow=workflow, pattern=pattern):
                    self.assertTrue(C.classify([representative])[scope])

    def test_budget_configuration_runs_only_the_python_scope(self):
        flags = C.classify(["scripts/ci/manual_tests.json", "scripts/ci/pytest_budget.py"])
        self.assertTrue(flags["python"])
        self.assertFalse(flags["web"])
        self.assertFalse(flags["engine"])

    def test_unrelated_asset_does_not_run_source_suites(self):
        self.assertFalse(any(C.classify(["assets/logo.svg"]).values()))

    def test_branch_creation_is_conservative(self):
        self.assertIsNone(C.changed_paths("push", {"before": "0" * 40, "after": "a" * 40}))
        self.assertIsNone(C.changed_paths("workflow_dispatch", {}))

    @patch.dict("os.environ", {"GITHUB_REPOSITORY": "owner/repository"})
    def test_merge_reuse_requires_success_and_identical_tree(self):
        pr = {"merged_at": "now", "merge_commit_sha": "merge", "head": {"sha": "head"}}
        for status, conclusion, tree, expected in (
            ("completed", "success", "same", True),
            ("completed", "failure", "same", False),
            ("in_progress", None, "same", False),
            ("completed", "success", "different", False),
        ):
            with self.subTest(status=status, conclusion=conclusion, tree=tree):
                calls = [[pr], {"workflow_runs": [{"status": status, "conclusion": conclusion,
                                                    "run_started_at": "2026-10-03T05:00:00Z"}]}]
                with patch.object(C, "github_json", side_effect=calls), patch.object(
                    C, "git", side_effect=["merge first head", tree, "same"]
                ):
                    self.assertEqual(C.checked_pr_merge("push", {"after": "merge"}, "ci.yml"), expected)

    def test_direct_push_and_pr_never_reuse_checks(self):
        with patch.object(C, "git", return_value="commit parent"), patch.object(C, "github_json") as api:
            self.assertFalse(C.checked_pr_merge("push", {"after": "commit"}, "ci.yml"))
            self.assertFalse(C.checked_pr_merge("pull_request", {}, "ci.yml"))
            api.assert_not_called()

    @patch.dict("os.environ", {"GITHUB_REPOSITORY": "owner/repository"})
    def test_failed_rerun_cannot_reuse_an_older_pass(self):
        pr = {"merged_at": "now", "merge_commit_sha": "merge", "head": {"sha": "head"}}
        runs = {"workflow_runs": [{"status": "completed", "conclusion": "failure",
                                    "run_started_at": "2026-10-03T05:00:00Z"},
                                  {"status": "completed", "conclusion": "success",
                                    "run_started_at": "2026-10-03T04:00:00Z"}]}
        with patch.object(C, "github_json", side_effect=[[pr], runs]), patch.object(
            C, "git", side_effect=["merge first head", "same", "same"]
        ):
            self.assertFalse(C.checked_pr_merge("push", {"after": "merge"}, "ci.yml"))

    @patch.dict("os.environ", {"GITHUB_REPOSITORY": "owner/repository"})
    def test_older_created_run_with_new_attempt_controls_reuse(self):
        pr = {"merged_at": "now", "merge_commit_sha": "merge", "head": {"sha": "head"}}
        for status, conclusion, expected in (("in_progress", None, False),
                                             ("completed", "failure", False),
                                             ("completed", "success", True)):
            with self.subTest(status=status, conclusion=conclusion):
                # API returns the newly created pass first, followed by an
                # older-created run whose latest attempt started afterward.
                runs = {"workflow_runs": [
                    {"status": "completed", "conclusion": "success", "run_attempt": 1,
                     "created_at": "2026-10-03T04:00:00Z", "run_started_at": "2026-10-03T04:00:00Z"},
                    {"status": status, "conclusion": conclusion, "run_attempt": 2,
                     "created_at": "2026-10-03T03:00:00Z", "run_started_at": "2026-10-03T05:00:00Z"},
                ]}
                with patch.object(C, "github_json", side_effect=[[pr], runs]), patch.object(
                    C, "git", side_effect=["merge first head", "same", "same"]
                ):
                    self.assertEqual(C.checked_pr_merge("push", {"after": "merge"}, "ci.yml"), expected)

    @patch.dict("os.environ", {"GITHUB_REPOSITORY": "owner/repository"})
    def test_missing_or_ambiguous_attempt_time_declines_reuse(self):
        pr = {"merged_at": "now", "merge_commit_sha": "merge", "head": {"sha": "head"}}
        missing = {"status": "completed", "conclusion": "success"}
        dated = {**missing, "created_at": "2026-10-03T04:00:00Z"}
        for attempts in ([missing], [dated, dict(dated)]):
            with self.subTest(attempts=attempts), patch.object(
                C, "github_json", side_effect=[[pr], {"workflow_runs": attempts}]
            ), patch.object(C, "git", side_effect=["merge first head", "same", "same"]):
                self.assertFalse(C.checked_pr_merge("push", {"after": "merge"}, "ci.yml"))


if __name__ == "__main__":
    unittest.main()

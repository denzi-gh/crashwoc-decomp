"""Verifies the save-slot resynchronization fix: authoritative progress must
reapply after the local Game state regresses (a lower-progress save load),
even when the incoming progress revision has already been seen before.

`merge_progress` in tools/coop_bridge/session.py implements the exact same
OR/max/idempotent merge algorithm required of the GameCube-side
CoopApplyInboundProgress (src/mod/coop.c) by PR3 Phase 7: level flags OR, hub
flags OR, hub crystals max, powerbits/gembits OR. Because the C game state
cannot be executed in this Python test environment, this test exercises that
shared merge decision directly (a "verifiable helper", per PR3 Phase 12)
rather than only checking the C source text. A companion structural test
below confirms the GC-side function was actually rewritten to match: no
early return gates the merge, and the applied revision only ever advances.
"""

from __future__ import annotations

import re
import unittest
from pathlib import Path

from tools.coop_bridge.session import empty_progress, merge_progress


ROOT = Path(__file__).resolve().parents[3]
COOP_C = ROOT / "src" / "mod" / "coop.c"


def _function_body(source: str, name: str) -> str:
    match = re.search(rf"\b{name}\s*\([^)]*\)\s*\{{", source)
    assert match is not None, f"function {name} not found"
    depth = 0
    start = match.end() - 1
    for i in range(start, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start : i + 1]
    raise AssertionError(f"unterminated body for function {name}")


class SaveSlotReapplyBehaviorTests(unittest.TestCase):
    """Behavioral test of the shared OR/max merge decision."""

    def test_reload_of_lower_progress_save_is_restored_on_reapply(self) -> None:
        current = empty_progress()
        incoming = empty_progress()
        incoming["hub_crystals"][0] = 3
        incoming["gembits"] = 0x04

        # 1-3: authoritative progress contains a crystal/gem flag the local
        # game state initially lacks; the first application adds it.
        self.assertTrue(merge_progress(current, incoming))
        self.assertEqual(current["hub_crystals"][0], 3)
        self.assertEqual(current["gembits"], 0x04)

        # 4: simulate loading a lower-progress save slot by regressing the
        # local fields directly (as a save load would, outside of any
        # merge), without touching `incoming` (the server's authoritative
        # copy is unaffected by a local save load).
        current["hub_crystals"][0] = 0
        current["gembits"] = 0

        # 5-6: reapplying the *same* authoritative progress (unchanged
        # content, and in the GC case, a revision already seen before) must
        # restore the missing fields, and report that state changed.
        self.assertTrue(merge_progress(current, incoming))
        self.assertEqual(current["hub_crystals"][0], 3)
        self.assertEqual(current["gembits"], 0x04)

        # 7-8: a third application with no local regression is idempotent
        # (this is the "changed" signal that gates CalculateGamePercentage
        # on the GC side).
        self.assertFalse(merge_progress(current, incoming))

    def test_higher_progress_save_expands_authoritative_state(self) -> None:
        current = empty_progress()
        current["powerbits"] = 0x01
        incoming = empty_progress()
        incoming["powerbits"] = 0x02

        self.assertTrue(merge_progress(current, incoming))
        self.assertEqual(current["powerbits"], 0x03)

    def test_level_and_hub_flags_use_or_not_overwrite(self) -> None:
        current = empty_progress()
        current["level_flags"][5] = 0x01
        current["hub_flags"][2] = 0x01
        incoming = empty_progress()
        incoming["level_flags"][5] = 0x02
        incoming["hub_flags"][2] = 0x02

        self.assertTrue(merge_progress(current, incoming))
        self.assertEqual(current["level_flags"][5], 0x03)
        self.assertEqual(current["hub_flags"][2], 0x03)


class ApplyInboundProgressStructureTests(unittest.TestCase):
    """Confirms the GC-side merge function matches the required shape."""

    def setUp(self) -> None:
        self.source = COOP_C.read_text(encoding="utf-8")
        self.apply_fn = _function_body(self.source, "CoopApplyInboundProgress")

    def test_no_early_return_gates_the_merge(self) -> None:
        # The merge loops (level_flags OR) must not be preceded by any
        # `return;` in the function body -- the old
        # `if (revision <= last_applied) return;` guard is gone.
        for_loop_pos = self.apply_fn.find("for (i = 0; i < 35; i++)")
        self.assertGreater(for_loop_pos, -1)
        preamble = self.apply_fn[:for_loop_pos]
        self.assertNotIn("return;", preamble)

    def test_last_applied_revision_only_advances(self) -> None:
        self.assertRegex(
            self.apply_fn,
            r"if\s*\(progress->revision\s*>\s*gCoopMailbox\.last_applied_progress_revision\)\s*\{\s*"
            r"gCoopMailbox\.last_applied_progress_revision\s*=\s*progress->revision;\s*\}",
        )
        # It must never be assigned outside that guard (which would allow a
        # lower incoming revision to move it backward).
        unconditional = re.sub(
            r"if\s*\(progress->revision\s*>\s*gCoopMailbox\.last_applied_progress_revision\)\s*\{[^}]*\}",
            "",
            self.apply_fn,
        )
        self.assertNotIn("gCoopMailbox.last_applied_progress_revision =", unconditional)

    def test_percentage_recalculated_only_when_changed(self) -> None:
        self.assertRegex(
            self.apply_fn, r"if\s*\(changed\s*!=\s*0\)\s*\{\s*CalculateGamePercentage\(&Game\);\s*\}"
        )


if __name__ == "__main__":
    unittest.main()

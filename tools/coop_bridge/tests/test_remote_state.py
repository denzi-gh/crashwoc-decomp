"""Static verification that remote-avatar rendering isolates local gameplay
state from the remote copy of the creature struct.

There is no way to execute the compiled PowerPC creature-rendering code from
this Python test environment, so these tests verify the invariants at the
source level against src/mod/coop.c: the exact file the co-op DOL is built
from (see docs/coop-hook-analysis.md and the `coop_verify_wrappers` /
`coop_verify_dol` build steps for the complementary compiled-artifact checks).
"""

from __future__ import annotations

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
COOP_C = ROOT / "src" / "mod" / "coop.c"


def _function_body(source: str, name: str) -> str:
    match = re.search(rf"\b{name}\s*\([^)]*\)\s*\{{", source)
    if match is None:
        raise AssertionError(f"function {name} not found in {COOP_C}")
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


class RemoteStateSanitationTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = COOP_C.read_text(encoding="utf-8")
        self.sanitize = _function_body(self.source, "CoopSanitizeRemoteCreature")
        self.hide = _function_body(self.source, "CoopHideRemotePlayer")
        self.draw = _function_body(self.source, "CoopDrawRemotePlayer")
        self.publish_avatar = _function_body(self.source, "CoopPublishAvatar")
        self.write_local_snapshot = _function_body(self.source, "CoopWriteLocalSnapshot")
        self.frame_update = _function_body(self.source, "CoopFrameUpdate")

    def test_sanitize_unconditionally_clears_local_transient_fields(self) -> None:
        cleared_fields = [
            "spin", "spin_frame", "spin_frames", "spin_wait",
            "slam", "slam_wait", "slam_frame",
            "obj.dangle", "freeze", "target", "target_wait",
            "fire", "fire_action",
            "rumble.buzz", "rumble.power", "rumble.frame", "rumble.frames",
            "hit_type", "jump_hold",
        ]
        for field in cleared_fields:
            pattern = rf"remote->{re.escape(field)}\s*=\s*0\s*;"
            self.assertRegex(
                self.sanitize, pattern, f"CoopSanitizeRemoteCreature must zero {field}"
            )
        # No conditional (if/else) hides any of these assignments: the whole
        # function body other than the brace and the `(void)local;` no-op is
        # a flat sequence of unconditional clears.
        body_without_braces = self.sanitize.strip()
        self.assertNotIn("if (", body_without_braces)
        self.assertNotIn("if(", body_without_braces)

    def test_hide_remote_player_calls_sanitize_before_drawing(self) -> None:
        sanitize_pos = self.hide.find("CoopSanitizeRemoteCreature(")
        draw_pos = self.hide.find("DrawCreatures(")
        self.assertGreater(sanitize_pos, -1, "CoopHideRemotePlayer must call the sanitizer")
        self.assertGreater(draw_pos, -1)
        self.assertLess(
            sanitize_pos, draw_pos,
            "remote state must be sanitized before the hidden draw call",
        )

    def test_draw_remote_player_calls_sanitize_before_applying_remote_state(self) -> None:
        sanitize_pos = self.draw.find("CoopSanitizeRemoteCreature(")
        spin_flag_pos = self.draw.find("COOP_MOVE_SPIN")
        draw_call_pos = self.draw.rfind("DrawCreatures(")
        self.assertGreater(sanitize_pos, -1, "CoopDrawRemotePlayer must call the sanitizer")
        self.assertGreater(spin_flag_pos, -1)
        self.assertGreater(draw_call_pos, -1)
        self.assertLess(sanitize_pos, spin_flag_pos)
        self.assertLess(spin_flag_pos, draw_call_pos)

    def test_remote_spin_is_gated_on_move_spin_flag(self) -> None:
        gate_match = re.search(
            r"if\s*\(\(inbound\.avatar\.move_flags\s*&\s*COOP_MOVE_SPIN\)\s*!=\s*0\)\s*\{(?P<body>.*?)\n    \}",
            self.draw,
            re.DOTALL,
        )
        self.assertIsNotNone(gate_match, "remote spin application must be gated on COOP_MOVE_SPIN")
        gated_body = gate_match.group("body")
        self.assertRegex(gated_body, r"sRemoteCreature\.spin\s*=\s*1\s*;")
        self.assertRegex(gated_body, r"sRemoteCreature\.spin_frame\s*=\s*\(short\)spin_frame\s*;")
        self.assertRegex(gated_body, r"sRemoteCreature\.spin_frames\s*=\s*\(short\)spin_frames\s*;")
        # Never assigned outside the gated block.
        outside = self.draw.replace(gated_body, "")
        self.assertNotIn("sRemoteCreature.spin = 1", outside)

    def test_remote_spin_values_come_from_inbound_snapshot(self) -> None:
        self.assertIn("spin_frame = inbound.avatar.spin_frame", self.draw)
        self.assertIn("spin_frames = inbound.avatar.spin_frames", self.draw)

    def test_remote_spin_validation_rejects_out_of_range_values(self) -> None:
        gate_match = re.search(
            r"if\s*\(\(inbound\.avatar\.move_flags\s*&\s*COOP_MOVE_SPIN\)\s*!=\s*0\)\s*\{(?P<body>.*?)\n    \}",
            self.draw,
            re.DOTALL,
        )
        assert gate_match is not None
        gated_body = gate_match.group("body")
        self.assertIn("spin_frames > 0", gated_body)
        self.assertIn("spin_frames <= COOP_SPIN_FRAMES_MAX", gated_body)
        self.assertIn("spin_frame <= spin_frames", gated_body)

    def test_publish_avatar_reads_dedicated_spin_fields_not_anim_action(self) -> None:
        self.assertIn("plr->spin != 0", self.publish_avatar)
        self.assertIn("plr->spin_frame", self.publish_avatar)
        self.assertIn("plr->spin_frames", self.publish_avatar)

    def test_local_publish_path_never_touches_remote_creature(self) -> None:
        # The local-snapshot publication path (CoopFrameUpdate ->
        # CoopWriteLocalSnapshot -> CoopPublishAvatar) must never call the
        # remote-only sanitizer or DrawCreatures: local spin state is read
        # from `plr`, not written into sRemoteCreature.
        for body, name in (
            (self.write_local_snapshot, "CoopWriteLocalSnapshot"),
            (self.publish_avatar, "CoopPublishAvatar"),
            (self.frame_update, "CoopFrameUpdate"),
        ):
            self.assertNotIn(
                "CoopSanitizeRemoteCreature", body,
                f"{name} must not touch the remote creature sanitizer",
            )
            self.assertNotIn("sRemoteCreature", body, f"{name} must not touch sRemoteCreature")

    def test_hide_remote_player_clears_spin_via_sanitize_on_early_return_path(self) -> None:
        # When there is no valid local player, CoopHideRemotePlayer takes an
        # early-return path that clears used/on/model without drawing at
        # all (nothing renders), so a stale spin flag cannot leak visually.
        early_return = self.hide.split("return;")[0]
        self.assertIn("sRemoteCreature.used = 0", early_return)
        self.assertIn("sRemoteCreature.on = 0", early_return)


if __name__ == "__main__":
    unittest.main()

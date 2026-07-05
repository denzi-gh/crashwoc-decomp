from __future__ import annotations

import struct
import unittest

from tools.coop_bridge import protocol_generated as proto
from tools.coop_bridge.memory import (
    DIFFERENT_LOCATION_SENTINEL,
    FakeMemoryAdapter,
    MAILBOX_ADDRESS,
    ProgressRevisionMapper,
    apply_progress_to_snapshot,
    ensure_hidden_inbound_snapshot,
    force_different_location,
    make_active_remote_snapshot,
    make_connected_no_remote_snapshot,
    make_disconnected_snapshot,
    mirror_local_snapshot,
    progress_from_snapshot,
    read_inbound_snapshot_consistent,
    read_local_snapshot_consistent,
    read_debug_state,
    require_ready_mailbox,
    read_snapshot_consistent,
    refresh_bridge_heartbeat,
    write_inbound_snapshot,
)
from tools.coop_bridge.messages import (
    ProtocolError,
    WIRE_PROTOCOL_VERSION,
    validate_client_message,
    validate_hello,
)


def empty_progress(revision: int = 0) -> dict[str, object]:
    return {
        "revision": revision,
        "level_flags": [0] * 35,
        "hub_flags": [0] * 6,
        "hub_crystals": [0] * 6,
        "powerbits": 0,
        "gembits": 0,
        "cutbits": 0,
    }


# COOP_MOVE_SPIN from src/mod/coop.h; not part of the generated protocol.
MOVE_SPIN = 0x00000001


def sample_snapshot(
    pos_x: float = 0.0,
    progress: dict[str, object] | None = None,
    move_flags: int = 0,
    spin_frame: int = 0,
    spin_frames: int = 0,
) -> bytes:
    snapshot = bytearray(proto.SNAPSHOT_SIZE)
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    struct.pack_into(">I", snapshot, snap["status_flags"], proto.STATUS_CONNECTED | proto.STATUS_ACTIVE)
    struct.pack_into(">I", snapshot, snap["avatar"] + avatar["flags"], 1)
    struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], pos_x)
    struct.pack_into(">I", snapshot, snap["avatar"] + avatar["move_flags"], move_flags)
    struct.pack_into(">H", snapshot, snap["avatar"] + avatar["spin_frame"], spin_frame)
    struct.pack_into(">H", snapshot, snap["avatar"] + avatar["spin_frames"], spin_frames)
    if progress is not None:
        snapshot = bytearray(apply_progress_to_snapshot(snapshot, progress))
    return bytes(snapshot)


def snapshot_spin_state(snapshot: bytes) -> tuple[int, int, int]:
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    base = snap["avatar"]
    move_flags = struct.unpack_from(">I", snapshot, base + avatar["move_flags"])[0]
    spin_frame = struct.unpack_from(">H", snapshot, base + avatar["spin_frame"])[0]
    spin_frames = struct.unpack_from(">H", snapshot, base + avatar["spin_frames"])[0]
    return move_flags, spin_frame, spin_frames


def prime_coop_mailbox(mem: FakeMemoryAdapter) -> None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    mem.write(MAILBOX_ADDRESS + mailbox["magic"], struct.pack(">I", proto.MAGIC))
    mem.write(MAILBOX_ADDRESS + mailbox["abi_version"], struct.pack(">H", proto.ABI_VERSION))
    mem.write(MAILBOX_ADDRESS + mailbox["struct_size"], struct.pack(">H", proto.MAILBOX_SIZE))
    mem.write(MAILBOX_ADDRESS + mailbox["build_id"], struct.pack(">I", proto.BUILD_ID))


class ProtocolTests(unittest.TestCase):
    def test_offsets_and_sizes(self) -> None:
        self.assertEqual(proto.ABI_VERSION, 2)
        self.assertEqual(proto.MAILBOX_SIZE, 0x1B8)
        self.assertEqual(proto.SNAPSHOT_SIZE, 0xB8)
        self.assertEqual(proto.SIZES["CoopAvatar"], 0x34)
        self.assertEqual(proto.OFFSETS["CoopMailbox"]["local_snapshot"], 0x1C)
        self.assertEqual(proto.OFFSETS["CoopMailbox"]["inbound_snapshot"], 0xD8)
        self.assertEqual(proto.OFFSETS["CoopProgress"]["cutbits"], 0x5C)
        self.assertEqual(proto.OFFSETS["CoopAvatar"]["move_flags"], 0x2C)
        self.assertEqual(proto.OFFSETS["CoopAvatar"]["spin_frame"], 0x30)
        self.assertEqual(proto.OFFSETS["CoopAvatar"]["spin_frames"], 0x32)

    def test_big_endian_pack(self) -> None:
        packed = proto.pack_values("CoopLocation", 1, 2, 3, 4, 5, 6, 7, 8)
        self.assertEqual(packed[:8], b"\x00\x00\x00\x01\x00\x00\x00\x02")
        self.assertEqual(proto.unpack_values("CoopLocation", packed), (1, 2, 3, 4, 5, 6, 7, 8))

    def test_avatar_spin_fields_round_trip(self) -> None:
        values = (1, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, -1, 1.0, -1, MOVE_SPIN, 42, 128)
        packed = proto.pack_values("CoopAvatar", *values)
        self.assertEqual(proto.unpack_values("CoopAvatar", packed), values)

    def test_snapshot_spin_flag_present(self) -> None:
        snapshot = sample_snapshot(move_flags=MOVE_SPIN, spin_frame=10, spin_frames=30)
        self.assertEqual(snapshot_spin_state(snapshot), (MOVE_SPIN, 10, 30))

    def test_snapshot_spin_flag_absent(self) -> None:
        snapshot = sample_snapshot()
        self.assertEqual(snapshot_spin_state(snapshot), (0, 0, 0))

    def test_snapshot_spin_zero_values_with_flag_set(self) -> None:
        snapshot = sample_snapshot(move_flags=MOVE_SPIN, spin_frame=0, spin_frames=0)
        self.assertEqual(snapshot_spin_state(snapshot), (MOVE_SPIN, 0, 0))

    def test_snapshot_spin_maximum_accepted_values(self) -> None:
        snapshot = sample_snapshot(move_flags=MOVE_SPIN, spin_frame=600, spin_frames=600)
        self.assertEqual(snapshot_spin_state(snapshot), (MOVE_SPIN, 600, 600))

    def test_snapshot_spin_malformed_values_round_trip_without_error(self) -> None:
        # The wire/mailbox layer only carries bytes; range validation is the
        # GameCube side's job (CoopDrawRemotePlayer). Confirm an out-of-range
        # or inverted spin_frame/spin_frames pair still encodes/decodes
        # safely instead of raising, so a malformed remote value can reach
        # the GC side to be rejected there rather than crashing the bridge.
        snapshot = sample_snapshot(move_flags=MOVE_SPIN, spin_frame=0xFFFF, spin_frames=0)
        self.assertEqual(snapshot_spin_state(snapshot), (MOVE_SPIN, 0xFFFF, 0))

    def test_sequence_lock_rejects_torn_snapshot(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 1))
        self.assertIsNone(
            read_snapshot_consistent(
                mem, mailbox["local_seq"], mailbox["local_snapshot"]
            )
        )

    def test_read_local_snapshot_consistent_accepts_even_sequence(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snapshot = sample_snapshot(7.0)
        mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 2))
        self.assertEqual(read_local_snapshot_consistent(mem), snapshot)

    def test_write_inbound_snapshot_uses_even_sequence(self) -> None:
        mem = FakeMemoryAdapter()
        snapshot = bytes([0x5A]) * proto.SNAPSHOT_SIZE
        write_inbound_snapshot(mem, snapshot)
        mailbox = proto.OFFSETS["CoopMailbox"]
        seq = struct.unpack(">I", mem.read(MAILBOX_ADDRESS + mailbox["inbound_seq"], 4))[0]
        self.assertEqual(seq, 2)
        self.assertEqual(
            mem.read(MAILBOX_ADDRESS + mailbox["inbound_snapshot"], proto.SNAPSHOT_SIZE),
            snapshot,
        )

    def test_progress_round_trip_excludes_unlisted_fields(self) -> None:
        progress = {
            "revision": 9,
            "level_flags": [0] * 35,
            "hub_flags": [0] * 6,
            "hub_crystals": [0] * 6,
            "powerbits": 3,
            "gembits": 4,
            "cutbits": 0,
        }
        progress["level_flags"][2] = 0x800
        progress["hub_flags"][1] = 7
        progress["hub_crystals"][1] = 5
        snapshot = apply_progress_to_snapshot(bytes(proto.SNAPSHOT_SIZE), progress)
        self.assertEqual(progress_from_snapshot(snapshot), progress)

    def test_mirror_snapshot_offsets_avatar(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snap = proto.OFFSETS["CoopSnapshot"]
        avatar = proto.OFFSETS["CoopAvatar"]
        snapshot = bytearray(proto.SNAPSHOT_SIZE)
        struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], 3.0)
        mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 2))
        self.assertTrue(mirror_local_snapshot(mem, offset_x=2.0))
        inbound = mem.read(MAILBOX_ADDRESS + mailbox["inbound_snapshot"], proto.SNAPSHOT_SIZE)
        self.assertEqual(struct.unpack_from(">f", inbound, snap["avatar"] + avatar["pos_x"])[0], 5.0)

    def test_force_different_location_sets_all_location_fields(self) -> None:
        snap = proto.OFFSETS["CoopSnapshot"]
        loc = proto.OFFSETS["CoopLocation"]
        snapshot = bytearray(proto.SNAPSHOT_SIZE)
        for i, offset in enumerate(loc.values()):
            struct.pack_into(">i", snapshot, snap["location"] + offset, i + 1)

        hidden = force_different_location(snapshot)

        for offset in loc.values():
            self.assertEqual(
                struct.unpack_from(">i", hidden, snap["location"] + offset)[0],
                DIFFERENT_LOCATION_SENTINEL,
            )

    def test_different_level_mirror_offsets_avatar_and_hides_location(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snap = proto.OFFSETS["CoopSnapshot"]
        loc = proto.OFFSETS["CoopLocation"]
        avatar = proto.OFFSETS["CoopAvatar"]
        snapshot = bytearray(proto.SNAPSHOT_SIZE)
        struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], 3.0)
        mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 2))

        self.assertTrue(mirror_local_snapshot(mem, offset_x=2.0, different_level=True))

        inbound = mem.read(MAILBOX_ADDRESS + mailbox["inbound_snapshot"], proto.SNAPSHOT_SIZE)
        self.assertEqual(struct.unpack_from(">f", inbound, snap["avatar"] + avatar["pos_x"])[0], 5.0)
        for offset in loc.values():
            self.assertEqual(
                struct.unpack_from(">i", inbound, snap["location"] + offset)[0],
                DIFFERENT_LOCATION_SENTINEL,
            )

    def test_ensure_hidden_inbound_rewrites_same_location_snapshot(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snap = proto.OFFSETS["CoopSnapshot"]
        loc = proto.OFFSETS["CoopLocation"]
        avatar = proto.OFFSETS["CoopAvatar"]
        snapshot = bytearray(proto.SNAPSHOT_SIZE)
        struct.pack_into(">i", snapshot, snap["location"] + loc["level"], 37)
        struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], 3.0)
        mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 2))
        write_inbound_snapshot(mem, snapshot)

        self.assertTrue(ensure_hidden_inbound_snapshot(mem, offset_x=2.0))

        inbound = read_inbound_snapshot_consistent(mem)
        self.assertIsNotNone(inbound)
        assert inbound is not None
        self.assertEqual(struct.unpack_from(">f", inbound, snap["avatar"] + avatar["pos_x"])[0], 5.0)
        for offset in loc.values():
            self.assertEqual(
                struct.unpack_from(">i", inbound, snap["location"] + offset)[0],
                DIFFERENT_LOCATION_SENTINEL,
            )

    def test_ensure_hidden_inbound_refreshes_existing_hidden_snapshot(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snap = proto.OFFSETS["CoopSnapshot"]
        loc = proto.OFFSETS["CoopLocation"]
        avatar = proto.OFFSETS["CoopAvatar"]
        snapshot = bytearray(proto.SNAPSHOT_SIZE)
        struct.pack_into(">i", snapshot, snap["location"] + loc["level"], 37)
        struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], 3.0)
        mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 2))
        write_inbound_snapshot(mem, force_different_location(snapshot))
        before_seq = struct.unpack(">I", mem.read(MAILBOX_ADDRESS + mailbox["inbound_seq"], 4))[0]

        self.assertTrue(ensure_hidden_inbound_snapshot(mem, offset_x=2.0))

        inbound = read_inbound_snapshot_consistent(mem)
        after_seq = struct.unpack(">I", mem.read(MAILBOX_ADDRESS + mailbox["inbound_seq"], 4))[0]
        self.assertIsNotNone(inbound)
        assert inbound is not None
        self.assertEqual(after_seq, before_seq + 2)
        self.assertEqual(struct.unpack_from(">f", inbound, snap["avatar"] + avatar["pos_x"])[0], 5.0)
        for offset in loc.values():
            self.assertEqual(
                struct.unpack_from(">i", inbound, snap["location"] + offset)[0],
                DIFFERENT_LOCATION_SENTINEL,
            )

    def test_refresh_bridge_heartbeat_does_not_rewrite_inbound_snapshot(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        snapshot = bytes([0xA5]) * proto.SNAPSHOT_SIZE
        write_inbound_snapshot(mem, snapshot)
        mem.write(MAILBOX_ADDRESS + mailbox["game_heartbeat"], struct.pack(">I", 42))
        before = mem.read(MAILBOX_ADDRESS + mailbox["inbound_snapshot"], proto.SNAPSHOT_SIZE)

        self.assertEqual(refresh_bridge_heartbeat(mem), 42)

        bridge_heartbeat = struct.unpack(
            ">I", mem.read(MAILBOX_ADDRESS + mailbox["bridge_heartbeat"], 4)
        )[0]
        after = mem.read(MAILBOX_ADDRESS + mailbox["inbound_snapshot"], proto.SNAPSHOT_SIZE)
        self.assertEqual(bridge_heartbeat, 42)
        self.assertEqual(after, before)

    def test_active_remote_snapshot_sets_connected_active(self) -> None:
        progress = empty_progress(12)
        inbound = make_active_remote_snapshot(sample_snapshot(9.0), progress)
        snap = proto.OFFSETS["CoopSnapshot"]
        self.assertEqual(
            struct.unpack_from(">I", inbound, snap["status_flags"])[0],
            proto.STATUS_CONNECTED | proto.STATUS_ACTIVE,
        )
        self.assertEqual(progress_from_snapshot(inbound), progress)

    def test_connected_no_remote_clears_avatar_flags(self) -> None:
        progress = empty_progress(13)
        inbound = make_connected_no_remote_snapshot(progress)
        snap = proto.OFFSETS["CoopSnapshot"]
        avatar = proto.OFFSETS["CoopAvatar"]
        self.assertEqual(
            struct.unpack_from(">I", inbound, snap["status_flags"])[0],
            proto.STATUS_CONNECTED,
        )
        self.assertEqual(struct.unpack_from(">I", inbound, snap["avatar"] + avatar["flags"])[0], 0)
        self.assertEqual(progress_from_snapshot(inbound), progress)

    def test_disconnected_snapshot_clears_avatar_and_preserves_progress(self) -> None:
        progress = empty_progress(14)
        progress["powerbits"] = 3
        inbound = make_disconnected_snapshot(progress)
        snap = proto.OFFSETS["CoopSnapshot"]
        avatar = proto.OFFSETS["CoopAvatar"]
        self.assertEqual(struct.unpack_from(">I", inbound, snap["status_flags"])[0], 0)
        self.assertEqual(struct.unpack_from(">I", inbound, snap["avatar"] + avatar["flags"])[0], 0)
        self.assertEqual(progress_from_snapshot(inbound), progress)

    def test_debug_state_reads_reserved_words(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        values = [0x43444247, 2, 8, 1, 3, 5, 0xC0000000, 0]
        mem.write(
            MAILBOX_ADDRESS + mailbox["reserved"],
            struct.pack(">8I", *values),
        )

        debug = read_debug_state(mem)

        self.assertEqual(debug.magic, 0x43444247)
        self.assertEqual(debug.calls, 2)
        self.assertEqual(debug.reason, 8)
        self.assertEqual(debug.draws, 1)
        self.assertEqual(debug.hides, 3)
        self.assertEqual(debug.local_level, 5)
        self.assertEqual(debug.inbound_level, -0x40000000)
        self.assertEqual(debug.same_location, 0)

    def test_require_ready_mailbox_accepts_matching_header(self) -> None:
        mem = FakeMemoryAdapter()
        prime_coop_mailbox(mem)

        header = require_ready_mailbox(mem)

        self.assertEqual(header.magic, proto.MAGIC)
        self.assertEqual(header.abi_version, proto.ABI_VERSION)
        self.assertEqual(header.struct_size, proto.MAILBOX_SIZE)
        self.assertEqual(header.build_id, proto.BUILD_ID)

    def test_require_ready_mailbox_rejects_bad_magic(self) -> None:
        mem = FakeMemoryAdapter()

        with self.assertRaisesRegex(RuntimeError, "magic mismatch"):
            require_ready_mailbox(mem)

    def test_require_ready_mailbox_rejects_bad_size(self) -> None:
        mem = FakeMemoryAdapter()
        prime_coop_mailbox(mem)
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(MAILBOX_ADDRESS + mailbox["struct_size"], struct.pack(">H", 1))

        with self.assertRaisesRegex(RuntimeError, "size mismatch"):
            require_ready_mailbox(mem)

    def test_validate_hello_accepts_current_versions(self) -> None:
        hello = validate_hello(
            {
                "type": "HELLO",
                "wire_version": WIRE_PROTOCOL_VERSION,
                "abi_version": proto.ABI_VERSION,
                "build_id": proto.BUILD_ID,
                "token": None,
            }
        )
        self.assertEqual(hello["abi_version"], proto.ABI_VERSION)

    def test_validate_hello_rejects_wrong_wire_version(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "wire"):
            validate_hello(
                {
                    "type": "HELLO",
                    "wire_version": WIRE_PROTOCOL_VERSION + 1,
                    "abi_version": proto.ABI_VERSION,
                    "build_id": proto.BUILD_ID,
                    "token": None,
                }
            )

    def test_validate_hello_rejects_wrong_abi(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "ABI"):
            validate_hello(
                {
                    "type": "HELLO",
                    "wire_version": WIRE_PROTOCOL_VERSION,
                    "abi_version": proto.ABI_VERSION + 1,
                    "build_id": proto.BUILD_ID,
                    "token": None,
                }
            )

    def test_validate_hello_rejects_wrong_build(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "build"):
            validate_hello(
                {
                    "type": "HELLO",
                    "wire_version": WIRE_PROTOCOL_VERSION,
                    "abi_version": proto.ABI_VERSION,
                    "build_id": proto.BUILD_ID + 1,
                    "token": None,
                }
            )

    def test_validate_hello_rejects_bad_token(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "token"):
            validate_hello(
                {
                    "type": "HELLO",
                    "wire_version": WIRE_PROTOCOL_VERSION,
                    "abi_version": proto.ABI_VERSION,
                    "build_id": proto.BUILD_ID,
                    "token": "wrong",
                },
                token="right",
            )

    def test_validate_client_rejects_unsupported_type(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "unsupported"):
            validate_client_message({"type": "SESSION_STATE"})

    def test_validate_client_rejects_non_object_json(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "object"):
            validate_client_message([])

    def test_validate_client_rejects_invalid_raw_hex(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "hexadecimal"):
            validate_client_message({"type": "LOCAL_SNAPSHOT", "client_seq": 1, "raw": "zz"})

    def test_validate_client_rejects_odd_raw_hex(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "odd"):
            validate_client_message({"type": "LOCAL_SNAPSHOT", "client_seq": 1, "raw": "0"})

    def test_validate_client_rejects_short_raw_snapshot(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "bytes"):
            validate_client_message({"type": "LOCAL_SNAPSHOT", "client_seq": 1, "raw": "00"})

    def test_validate_client_rejects_long_raw_snapshot(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "bytes"):
            validate_client_message(
                {
                    "type": "LOCAL_SNAPSHOT",
                    "client_seq": 1,
                    "raw": bytes(proto.SNAPSHOT_SIZE + 1).hex(),
                }
            )

    def test_validate_client_rejects_boolean_sequence(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "integer"):
            validate_client_message(
                {"type": "LOCAL_SNAPSHOT", "client_seq": True, "raw": sample_snapshot().hex()}
            )

    def test_validate_client_rejects_out_of_range_sequence(self) -> None:
        with self.assertRaisesRegex(ProtocolError, "range"):
            validate_client_message(
                {
                    "type": "LOCAL_SNAPSHOT",
                    "client_seq": 0x100000000,
                    "raw": sample_snapshot().hex(),
                }
            )

    def test_progress_revision_mapper_maps_above_mailbox_revision(self) -> None:
        mapper = ProgressRevisionMapper(50)
        mapped = mapper.map_progress(empty_progress(0))
        self.assertEqual(mapped["revision"], 51)

    def test_progress_revision_mapper_reuses_repeated_state(self) -> None:
        mapper = ProgressRevisionMapper(50)
        progress = empty_progress(0)
        self.assertEqual(mapper.map_progress(progress)["revision"], 51)
        self.assertEqual(mapper.map_progress(progress)["revision"], 51)

    def test_progress_revision_mapper_increments_when_content_changes(self) -> None:
        mapper = ProgressRevisionMapper(50)
        first = empty_progress(0)
        second = empty_progress(1)
        second["gembits"] = 1
        self.assertEqual(mapper.map_progress(first)["revision"], 51)
        self.assertEqual(mapper.map_progress(second)["revision"], 52)

    def test_progress_revision_mapper_handles_reconnect_lower_server_revision(self) -> None:
        mapper = ProgressRevisionMapper(75)
        mapped = mapper.map_progress(empty_progress(0))
        self.assertEqual(mapped["revision"], 76)

    def test_progress_revision_mapper_rejects_same_revision_different_content(self) -> None:
        mapper = ProgressRevisionMapper(1)
        first = empty_progress(0)
        second = empty_progress(0)
        second["powerbits"] = 1
        mapper.map_progress(first)
        with self.assertRaisesRegex(ValueError, "different content"):
            mapper.map_progress(second)

    def test_progress_revision_mapper_overflow_is_clear(self) -> None:
        mapper = ProgressRevisionMapper(0xFFFFFFFF)
        with self.assertRaisesRegex(OverflowError, "overflow"):
            mapper.map_progress(empty_progress(0))


if __name__ == "__main__":
    unittest.main()

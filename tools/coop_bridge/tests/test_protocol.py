from __future__ import annotations

import struct
import unittest

from tools.coop_bridge import protocol_generated as proto
from tools.coop_bridge.memory import (
    FakeMemoryAdapter,
    MAILBOX_ADDRESS,
    apply_progress_to_snapshot,
    mirror_local_snapshot,
    progress_from_snapshot,
    read_snapshot_consistent,
    write_inbound_snapshot,
)


class ProtocolTests(unittest.TestCase):
    def test_offsets_and_sizes(self) -> None:
        self.assertEqual(proto.MAILBOX_SIZE, 0x1A8)
        self.assertEqual(proto.SNAPSHOT_SIZE, 0xB0)
        self.assertEqual(proto.OFFSETS["CoopMailbox"]["local_snapshot"], 0x1C)
        self.assertEqual(proto.OFFSETS["CoopMailbox"]["inbound_snapshot"], 0xD0)
        self.assertEqual(proto.OFFSETS["CoopProgress"]["cutbits"], 0x5C)

    def test_big_endian_pack(self) -> None:
        packed = proto.pack_values("CoopLocation", 1, 2, 3, 4, 5, 6, 7, 8)
        self.assertEqual(packed[:8], b"\x00\x00\x00\x01\x00\x00\x00\x02")
        self.assertEqual(proto.unpack_values("CoopLocation", packed), (1, 2, 3, 4, 5, 6, 7, 8))

    def test_sequence_lock_rejects_torn_snapshot(self) -> None:
        mem = FakeMemoryAdapter()
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", 1))
        self.assertIsNone(
            read_snapshot_consistent(
                mem, mailbox["local_seq"], mailbox["local_snapshot"]
            )
        )

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


if __name__ == "__main__":
    unittest.main()

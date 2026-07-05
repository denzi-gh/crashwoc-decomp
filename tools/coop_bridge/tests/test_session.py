from __future__ import annotations

import asyncio
import struct
import unittest

from tools.coop_bridge import protocol_generated as proto
from tools.coop_bridge.client import BridgeClient
from tools.coop_bridge.memory import (
    FakeMemoryAdapter,
    MAILBOX_ADDRESS,
    apply_progress_to_snapshot,
    progress_from_snapshot,
    read_inbound_snapshot_consistent,
)
from tools.coop_bridge.messages import WIRE_PROTOCOL_VERSION, make_hello
from tools.coop_bridge.net import read_message, write_message
from tools.coop_bridge.session import (
    LatestStateQueue,
    SessionServer,
    empty_progress,
    merge_progress,
)


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
    loc = proto.OFFSETS["CoopLocation"]
    avatar = proto.OFFSETS["CoopAvatar"]
    struct.pack_into(">I", snapshot, snap["status_flags"], proto.STATUS_CONNECTED | proto.STATUS_ACTIVE)
    struct.pack_into(">i", snapshot, snap["location"] + loc["level"], 1)
    struct.pack_into(">I", snapshot, snap["avatar"] + avatar["flags"], 1)
    struct.pack_into(">f", snapshot, snap["avatar"] + avatar["pos_x"], pos_x)
    struct.pack_into(">i", snapshot, snap["avatar"] + avatar["action"], 1)
    struct.pack_into(">I", snapshot, snap["avatar"] + avatar["move_flags"], move_flags)
    struct.pack_into(">H", snapshot, snap["avatar"] + avatar["spin_frame"], spin_frame)
    struct.pack_into(">H", snapshot, snap["avatar"] + avatar["spin_frames"], spin_frames)
    if progress is not None:
        snapshot = bytearray(apply_progress_to_snapshot(snapshot, progress))
    return bytes(snapshot)


def inbound_spin_state(mem: FakeMemoryAdapter) -> tuple[int, int, int] | None:
    inbound = read_inbound_snapshot_consistent(mem)
    if inbound is None:
        return None
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    base = snap["avatar"]
    move_flags = struct.unpack_from(">I", inbound, base + avatar["move_flags"])[0]
    spin_frame = struct.unpack_from(">H", inbound, base + avatar["spin_frame"])[0]
    spin_frames = struct.unpack_from(">H", inbound, base + avatar["spin_frames"])[0]
    return move_flags, spin_frame, spin_frames


def write_local_snapshot(mem: FakeMemoryAdapter, snapshot: bytes) -> None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    current = struct.unpack(">I", mem.read(MAILBOX_ADDRESS + mailbox["local_seq"], 4))[0]
    if current & 1:
        current += 1
    mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", current + 1))
    mem.write(MAILBOX_ADDRESS + mailbox["local_snapshot"], snapshot)
    mem.write(MAILBOX_ADDRESS + mailbox["local_seq"], struct.pack(">I", current + 2))


def prime_coop_mailbox(mem: FakeMemoryAdapter) -> None:
    mailbox = proto.OFFSETS["CoopMailbox"]
    mem.write(MAILBOX_ADDRESS + mailbox["magic"], struct.pack(">I", proto.MAGIC))
    mem.write(MAILBOX_ADDRESS + mailbox["abi_version"], struct.pack(">H", proto.ABI_VERSION))
    mem.write(MAILBOX_ADDRESS + mailbox["struct_size"], struct.pack(">H", proto.MAILBOX_SIZE))
    mem.write(MAILBOX_ADDRESS + mailbox["build_id"], struct.pack(">I", proto.BUILD_ID))


def inbound_pos_x(mem: FakeMemoryAdapter) -> float | None:
    inbound = read_inbound_snapshot_consistent(mem)
    if inbound is None:
        return None
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    return struct.unpack_from(">f", inbound, snap["avatar"] + avatar["pos_x"])[0]


def inbound_avatar_flags(mem: FakeMemoryAdapter) -> int | None:
    inbound = read_inbound_snapshot_consistent(mem)
    if inbound is None:
        return None
    snap = proto.OFFSETS["CoopSnapshot"]
    avatar = proto.OFFSETS["CoopAvatar"]
    return struct.unpack_from(">I", inbound, snap["avatar"] + avatar["flags"])[0]


def inbound_progress(mem: FakeMemoryAdapter) -> dict[str, object] | None:
    inbound = read_inbound_snapshot_consistent(mem)
    if inbound is None:
        return None
    return progress_from_snapshot(inbound)


def inbound_pos_is(mem: FakeMemoryAdapter, expected: float) -> bool:
    value = inbound_pos_x(mem)
    return value is not None and abs(value - expected) < 0.01


async def wait_until(predicate, timeout: float = 2.0) -> None:
    loop = asyncio.get_running_loop()
    deadline = loop.time() + timeout
    while loop.time() < deadline:
        if predicate():
            return
        await asyncio.sleep(0.01)
    raise AssertionError("condition timed out")


class MergeTests(unittest.TestCase):
    def test_progress_merge_is_monotonic_and_idempotent(self) -> None:
        current = empty_progress()
        incoming = empty_progress()
        incoming["level_flags"][0] = 0x800
        incoming["hub_flags"][0] = 2
        incoming["hub_crystals"][0] = 3
        incoming["powerbits"] = 1
        incoming["gembits"] = 4
        self.assertTrue(merge_progress(current, incoming))
        self.assertEqual(current["revision"], 1)
        self.assertFalse(merge_progress(current, incoming))
        self.assertEqual(current["revision"], 1)

    def test_cutbits_disabled_by_default(self) -> None:
        current = empty_progress()
        incoming = empty_progress()
        incoming["cutbits"] = 0xFFFFFFFF
        self.assertFalse(merge_progress(current, incoming))
        self.assertEqual(current["cutbits"], 0)


class ServerTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self) -> None:
        self.server_state = SessionServer()
        self.server = await self.server_state.start("127.0.0.1", 0)
        sock = self.server.sockets[0]
        self.port = sock.getsockname()[1]

    async def asyncTearDown(self) -> None:
        await self.server_state.close()

    async def connect(self):
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        await write_message(writer, make_hello())
        return reader, writer, await read_message(reader)

    async def test_rejects_third_player(self) -> None:
        c1 = await self.connect()
        c2 = await self.connect()
        self.assertEqual(c1[2]["type"], "WELCOME")
        self.assertEqual(c2[2]["type"], "WELCOME")
        reader3, writer3 = await asyncio.open_connection("127.0.0.1", self.port)
        await write_message(writer3, make_hello())
        error = await read_message(reader3)
        self.assertEqual(error["type"], "ERROR")
        self.assertIn("full", error["error"])
        for _, writer, _ in (c1, c2):
            await write_message(writer, {"type": "DISCONNECT"})
            writer.close()
            await writer.wait_closed()
        writer3.close()
        await writer3.wait_closed()

    async def test_rejects_bad_abi(self) -> None:
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        hello = make_hello()
        hello["abi_version"] = proto.ABI_VERSION + 1
        await write_message(writer, hello)
        error = await read_message(reader)
        self.assertEqual(error["type"], "ERROR")
        self.assertIn("ABI", error["error"])
        writer.close()
        await writer.wait_closed()

    async def test_rejects_incompatible_wire_build_and_token(self) -> None:
        for key, value, needle in (
            ("wire_version", WIRE_PROTOCOL_VERSION + 1, "wire"),
            ("build_id", proto.BUILD_ID + 1, "build"),
        ):
            reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
            hello = make_hello()
            hello[key] = value
            await write_message(writer, hello)
            error = await read_message(reader)
            self.assertEqual(error["type"], "ERROR")
            self.assertIn(needle, error["error"])
            writer.close()
            await writer.wait_closed()

        token_server = SessionServer(token="secret")
        server = await token_server.start("127.0.0.1", 0)
        port = server.sockets[0].getsockname()[1]
        reader, writer = await asyncio.open_connection("127.0.0.1", port)
        await write_message(writer, make_hello("wrong"))
        error = await read_message(reader)
        self.assertEqual(error["type"], "ERROR")
        self.assertIn("token", error["error"])
        writer.close()
        await writer.wait_closed()
        await token_server.close()

    async def test_derives_progress_from_raw_snapshot(self) -> None:
        reader, writer, welcome = await self.connect()
        self.assertEqual(welcome["type"], "WELCOME")
        progress = empty_progress()
        progress["gembits"] = 2
        await write_message(
            writer,
            {"type": "LOCAL_SNAPSHOT", "client_seq": 1, "raw": sample_snapshot(1.0, progress).hex()},
        )
        await wait_until(lambda: self.server_state.progress["gembits"] == 2)
        writer.close()
        await writer.wait_closed()

    async def test_disconnect_triggers_remote_null_and_progress_persists(self) -> None:
        c1 = await self.connect()
        c2 = await self.connect()
        progress = empty_progress()
        progress["powerbits"] = 1
        await write_message(
            c2[1],
            {"type": "LOCAL_SNAPSHOT", "client_seq": 1, "raw": sample_snapshot(9.0, progress).hex()},
        )
        message = await read_message(c1[0])
        while message["type"] != "SESSION_STATE" or message["remote_raw"] is None:
            message = await read_message(c1[0])
        self.assertEqual(self.server_state.progress["powerbits"], 1)
        await write_message(c2[1], {"type": "DISCONNECT"})
        c2[1].close()
        await c2[1].wait_closed()
        message = await read_message(c1[0])
        while message["type"] != "SESSION_STATE" or message["remote_raw"] is not None:
            message = await read_message(c1[0])
        self.assertIsNone(message["remote_raw"])
        self.assertEqual(message["progress"]["powerbits"], 1)
        c1[1].close()
        await c1[1].wait_closed()


class LatestStateQueueTests(unittest.IsolatedAsyncioTestCase):
    async def test_latest_state_queue_is_bounded_and_keeps_newest(self) -> None:
        queue = LatestStateQueue()
        for seq in range(10):
            queue.put_latest({"type": "SESSION_STATE", "state_seq": seq})
            self.assertLessEqual(queue.qsize(), 1)
        self.assertEqual(queue.replaced_count, 9)
        newest = await queue.get()
        self.assertEqual(newest["state_seq"], 9)
        queue.task_done()


class BridgeIntegrationTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self) -> None:
        self.server_state = SessionServer()
        self.server = await self.server_state.start("127.0.0.1", 0)
        self.port = self.server.sockets[0].getsockname()[1]

    async def asyncTearDown(self) -> None:
        await self.server_state.close()

    async def start_client(self, mem: FakeMemoryAdapter) -> tuple[BridgeClient, asyncio.Task[None]]:
        client = BridgeClient(mem, "127.0.0.1", self.port, reconnect=False)
        task = asyncio.create_task(client.run())
        await asyncio.sleep(0)
        return client, task

    async def stop_client(self, client: BridgeClient, task: asyncio.Task[None]) -> None:
        client.stop()
        task.cancel()
        await asyncio.gather(task, return_exceptions=True)

    async def test_two_fake_clients_exchange_independent_snapshots_and_reconnect(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: inbound_pos_is(mem1, 5.0))
            await wait_until(lambda: inbound_pos_is(mem2, 1.0))
            write_local_snapshot(mem2, sample_snapshot(8.0, progress))
            await wait_until(lambda: inbound_pos_is(mem1, 8.0))
            self.assertAlmostEqual(inbound_pos_x(mem2), 1.0, places=2)

            shared = empty_progress()
            shared["gembits"] = 6
            write_local_snapshot(mem1, sample_snapshot(2.0, shared))
            await wait_until(
                lambda: (inbound_progress(mem2) or {}).get("gembits") == 6
            )

            await self.stop_client(client2, task2)
            await wait_until(lambda: inbound_avatar_flags(mem1) == 0)

            mem2b = FakeMemoryAdapter()
            prime_coop_mailbox(mem2b)
            write_local_snapshot(mem2b, sample_snapshot(11.0, progress))
            client2b, task2b = await self.start_client(mem2b)
            try:
                await wait_until(lambda: inbound_pos_is(mem1, 11.0))
            finally:
                await self.stop_client(client2b, task2b)
        finally:
            await self.stop_client(client1, task1)

    async def test_single_client_receives_welcome_progress_and_revision_maps_up(self) -> None:
        self.server_state.progress["revision"] = 0
        self.server_state.progress["gembits"] = 4
        mem = FakeMemoryAdapter()
        prime_coop_mailbox(mem)
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(
            MAILBOX_ADDRESS + mailbox["last_applied_progress_revision"],
            struct.pack(">I", 50),
        )
        write_local_snapshot(mem, sample_snapshot(3.0, empty_progress()))
        client, task = await self.start_client(mem)
        try:
            await wait_until(lambda: progress_from_snapshot(read_inbound_snapshot_consistent(mem) or bytes(proto.SNAPSHOT_SIZE))["gembits"] == 4)
            inbound = read_inbound_snapshot_consistent(mem)
            assert inbound is not None
            delivered = progress_from_snapshot(inbound)
            self.assertEqual(delivered["revision"], 51)
            self.assertEqual(delivered["gembits"], 4)
        finally:
            await self.stop_client(client, task)


class SpinTransportTests(unittest.IsolatedAsyncioTestCase):
    """PR3 Phase 14: each player's dedicated spin state (move_flags,
    spin_frame, spin_frames) must transit independently through the
    session server to the other player's inbound snapshot, never to the
    sender's own inbound snapshot."""

    async def asyncSetUp(self) -> None:
        self.server_state = SessionServer()
        self.server = await self.server_state.start("127.0.0.1", 0)
        self.port = self.server.sockets[0].getsockname()[1]

    async def asyncTearDown(self) -> None:
        await self.server_state.close()

    async def start_client(self, mem: FakeMemoryAdapter) -> tuple[BridgeClient, asyncio.Task[None]]:
        client = BridgeClient(mem, "127.0.0.1", self.port, reconnect=False)
        task = asyncio.create_task(client.run())
        await asyncio.sleep(0)
        return client, task

    async def stop_client(self, client: BridgeClient, task: asyncio.Task[None]) -> None:
        client.stop()
        task.cancel()
        await asyncio.gather(task, return_exceptions=True)

    async def test_only_player_one_spinning_is_seen_only_by_player_two(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress, move_flags=MOVE_SPIN, spin_frame=5, spin_frames=20))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: (inbound_spin_state(mem2) or (0, 0, 0))[0] == MOVE_SPIN)
            self.assertEqual(inbound_spin_state(mem2), (MOVE_SPIN, 5, 20))
            # Client 1's own inbound snapshot (player 2's state) never
            # carries player 1's own spin back to itself.
            await wait_until(lambda: inbound_pos_is(mem1, 5.0))
            self.assertEqual((inbound_spin_state(mem1) or (0, 0, 0))[0], 0)
        finally:
            await self.stop_client(client1, task1)
            await self.stop_client(client2, task2)

    async def test_only_player_two_spinning_is_seen_only_by_player_one(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress, move_flags=MOVE_SPIN, spin_frame=7, spin_frames=25))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: (inbound_spin_state(mem1) or (0, 0, 0))[0] == MOVE_SPIN)
            self.assertEqual(inbound_spin_state(mem1), (MOVE_SPIN, 7, 25))
            await wait_until(lambda: inbound_pos_is(mem2, 1.0))
            self.assertEqual((inbound_spin_state(mem2) or (0, 0, 0))[0], 0)
        finally:
            await self.stop_client(client1, task1)
            await self.stop_client(client2, task2)

    async def test_both_players_spinning_are_both_delivered(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress, move_flags=MOVE_SPIN, spin_frame=3, spin_frames=15))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress, move_flags=MOVE_SPIN, spin_frame=9, spin_frames=18))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: (inbound_spin_state(mem1) or (0, 0, 0))[0] == MOVE_SPIN)
            await wait_until(lambda: (inbound_spin_state(mem2) or (0, 0, 0))[0] == MOVE_SPIN)
            self.assertEqual(inbound_spin_state(mem1), (MOVE_SPIN, 9, 18))
            self.assertEqual(inbound_spin_state(mem2), (MOVE_SPIN, 3, 15))
        finally:
            await self.stop_client(client1, task1)
            await self.stop_client(client2, task2)

    async def test_spin_ending_clears_promptly_on_the_remote_side(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress, move_flags=MOVE_SPIN, spin_frame=5, spin_frames=20))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: (inbound_spin_state(mem2) or (0, 0, 0))[0] == MOVE_SPIN)

            write_local_snapshot(mem1, sample_snapshot(1.0, progress))
            await wait_until(lambda: (inbound_spin_state(mem2) or (MOVE_SPIN, 0, 0))[0] == 0)
            self.assertEqual(inbound_spin_state(mem2), (0, 0, 0))
        finally:
            await self.stop_client(client1, task1)
            await self.stop_client(client2, task2)

    async def test_independent_spin_frame_progression_is_not_swapped(self) -> None:
        mem1 = FakeMemoryAdapter()
        mem2 = FakeMemoryAdapter()
        prime_coop_mailbox(mem1)
        prime_coop_mailbox(mem2)
        progress = empty_progress()
        write_local_snapshot(mem1, sample_snapshot(1.0, progress, move_flags=MOVE_SPIN, spin_frame=1, spin_frames=30))
        write_local_snapshot(mem2, sample_snapshot(5.0, progress, move_flags=MOVE_SPIN, spin_frame=11, spin_frames=30))
        client1, task1 = await self.start_client(mem1)
        client2, task2 = await self.start_client(mem2)
        try:
            await wait_until(lambda: (inbound_spin_state(mem1) or (0, 0, 0))[1] == 11)
            await wait_until(lambda: (inbound_spin_state(mem2) or (0, 0, 0))[1] == 1)

            write_local_snapshot(mem1, sample_snapshot(1.0, progress, move_flags=MOVE_SPIN, spin_frame=2, spin_frames=30))
            write_local_snapshot(mem2, sample_snapshot(5.0, progress, move_flags=MOVE_SPIN, spin_frame=12, spin_frames=30))
            await wait_until(lambda: (inbound_spin_state(mem1) or (0, 0, 0))[1] == 12)
            await wait_until(lambda: (inbound_spin_state(mem2) or (0, 0, 0))[1] == 2)

            # Neither recipient ever receives its own spin_frame value back.
            self.assertNotEqual(inbound_spin_state(mem1)[1], 2)
            self.assertNotEqual(inbound_spin_state(mem2)[1], 12)
        finally:
            await self.stop_client(client1, task1)
            await self.stop_client(client2, task2)


class RevisionMapperResetTests(unittest.IsolatedAsyncioTestCase):
    """PR3 Phase 8/13: a fresh ProgressRevisionMapper must be created for
    every successful WELCOME, seeded from the mailbox's current applied
    revision, so a new (or restarted) server session's revision-zero
    progress is never rejected as a revision reuse of a previous session."""

    async def start_client(self, mem: FakeMemoryAdapter, port: int) -> tuple[BridgeClient, asyncio.Task[None]]:
        client = BridgeClient(mem, "127.0.0.1", port, reconnect=False)
        task = asyncio.create_task(client.run())
        await asyncio.sleep(0)
        return client, task

    async def stop_client(self, client: BridgeClient, task: asyncio.Task[None]) -> None:
        client.stop()
        task.cancel()
        await asyncio.gather(task, return_exceptions=True)

    async def test_reconnect_to_new_server_session_with_lower_revision_zero_is_accepted(self) -> None:
        mem = FakeMemoryAdapter()
        prime_coop_mailbox(mem)
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(MAILBOX_ADDRESS + mailbox["last_applied_progress_revision"], struct.pack(">I", 50))
        write_local_snapshot(mem, sample_snapshot(3.0, empty_progress()))

        server_a = SessionServer()
        server_a_net = await server_a.start("127.0.0.1", 0)
        port_a = server_a_net.sockets[0].getsockname()[1]
        try:
            # Server A advances its own progress revision past 0 before the
            # client connects, so server B (below) starting fresh at
            # revision 0 is a genuinely lower server-side revision.
            server_a.progress["gembits"] = 1
            server_a.progress["revision"] = 5

            client1, task1 = await self.start_client(mem, port_a)
            try:
                await wait_until(lambda: (inbound_progress(mem) or {}).get("gembits") == 1)
                first_mapper = client1._mapper
                self.assertIsNotNone(first_mapper)
                first_delivered = inbound_progress(mem)
                assert first_delivered is not None
                self.assertGreater(first_delivered["revision"], 50)
            finally:
                await self.stop_client(client1, task1)
        finally:
            await server_a.close()

        # In real Dolphin, CoopFrameUpdate() advances the mailbox's
        # last_applied_progress_revision as it applies each delivery. The
        # fake memory adapter has no running GC code, so mirror that one
        # effect here to faithfully model the mailbox state a real reconnect
        # would see.
        mailbox = proto.OFFSETS["CoopMailbox"]
        mem.write(
            MAILBOX_ADDRESS + mailbox["last_applied_progress_revision"],
            struct.pack(">I", first_delivered["revision"]),
        )

        # Server A is gone; a new server (a restarted host) reuses server
        # revision zero with content that differs from what server A last
        # delivered.
        server_b = SessionServer()
        server_b_net = await server_b.start("127.0.0.1", 0)
        port_b = server_b_net.sockets[0].getsockname()[1]
        try:
            self.assertEqual(server_b.progress["revision"], 0)
            server_b.progress["gembits"] = 9

            client2, task2 = await self.start_client(mem, port_b)
            try:
                await wait_until(lambda: (inbound_progress(mem) or {}).get("gembits") == 9)
                self.assertIsNot(client2._mapper, None)
                delivered = inbound_progress(mem)
                assert delivered is not None
                # No revision-reuse error occurred, and the mapped revision
                # never moved backward relative to what was already applied.
                self.assertGreater(delivered["revision"], first_delivered["revision"])
            finally:
                await self.stop_client(client2, task2)
        finally:
            await server_b.close()

    async def test_reconnect_to_same_server_session_gets_a_new_mapper_instance(self) -> None:
        mem = FakeMemoryAdapter()
        prime_coop_mailbox(mem)
        write_local_snapshot(mem, sample_snapshot(3.0, empty_progress()))

        server = SessionServer()
        server_net = await server.start("127.0.0.1", 0)
        port = server_net.sockets[0].getsockname()[1]
        try:
            client1, task1 = await self.start_client(mem, port)
            try:
                await wait_until(lambda: inbound_progress(mem) is not None)
                mapper1 = client1._mapper
                session1 = client1._session_id
            finally:
                await self.stop_client(client1, task1)

            # Reconnect (a new BridgeClient instance, same mailbox) to the
            # same still-running server session.
            client2, task2 = await self.start_client(mem, port)
            try:
                await wait_until(lambda: client2._session_id is not None)
                self.assertEqual(client2._session_id, session1)
                self.assertIsNot(client2._mapper, mapper1)
            finally:
                await self.stop_client(client2, task2)
        finally:
            await server.close()


if __name__ == "__main__":
    unittest.main()

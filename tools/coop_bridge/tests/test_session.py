from __future__ import annotations

import asyncio
import unittest

from tools.coop_bridge import protocol_generated as proto
from tools.coop_bridge.net import read_message, write_message
from tools.coop_bridge.session import SessionServer, empty_progress, merge_progress


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
        self.server = await asyncio.start_server(self.server_state.handle_client, "127.0.0.1", 0)
        sock = self.server.sockets[0]
        self.port = sock.getsockname()[1]

    async def asyncTearDown(self) -> None:
        self.server.close()
        await self.server.wait_closed()

    async def connect(self):
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        await write_message(
            writer,
            {"type": "HELLO", "abi_version": proto.ABI_VERSION, "build_id": proto.BUILD_ID},
        )
        return reader, writer, await read_message(reader)

    async def test_rejects_third_player(self) -> None:
        c1 = await self.connect()
        c2 = await self.connect()
        self.assertEqual(c1[2]["type"], "WELCOME")
        self.assertEqual(c2[2]["type"], "WELCOME")
        reader3, writer3 = await asyncio.open_connection("127.0.0.1", self.port)
        await write_message(
            writer3,
            {"type": "HELLO", "abi_version": proto.ABI_VERSION, "build_id": proto.BUILD_ID},
        )
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
        await write_message(
            writer,
            {"type": "HELLO", "abi_version": proto.ABI_VERSION + 1, "build_id": proto.BUILD_ID},
        )
        error = await read_message(reader)
        self.assertEqual(error["type"], "ERROR")
        self.assertIn("ABI", error["error"])
        writer.close()
        await writer.wait_closed()


if __name__ == "__main__":
    unittest.main()

from __future__ import annotations

import asyncio
import time
import uuid
from dataclasses import dataclass, field
from typing import Any

from .memory import progress_from_snapshot
from .messages import (
    ProtocolError,
    TICK_HZ,
    WIRE_PROTOCOL_VERSION,
    encode_raw_snapshot,
    make_error,
    validate_client_message,
    validate_hello,
)
from .net import read_message, write_message


def empty_progress() -> dict[str, Any]:
    return {
        "revision": 0,
        "level_flags": [0] * 35,
        "hub_flags": [0] * 6,
        "hub_crystals": [0] * 6,
        "powerbits": 0,
        "gembits": 0,
        "cutbits": 0,
    }


def merge_progress(
    current: dict[str, Any], incoming: dict[str, Any], share_cutbits: bool = False
) -> bool:
    changed = False
    for i in range(35):
        value = current["level_flags"][i] | incoming["level_flags"][i]
        changed |= value != current["level_flags"][i]
        current["level_flags"][i] = value
    for i in range(6):
        value = current["hub_flags"][i] | incoming["hub_flags"][i]
        changed |= value != current["hub_flags"][i]
        current["hub_flags"][i] = value
        crystals = max(current["hub_crystals"][i], incoming["hub_crystals"][i])
        changed |= crystals != current["hub_crystals"][i]
        current["hub_crystals"][i] = crystals
    for key in ("powerbits", "gembits"):
        value = current[key] | incoming[key]
        changed |= value != current[key]
        current[key] = value
    if share_cutbits:
        value = current["cutbits"] | incoming.get("cutbits", 0)
        changed |= value != current["cutbits"]
        current["cutbits"] = value
    if changed:
        current["revision"] = (current["revision"] + 1) & 0xFFFFFFFF
    return changed


class LatestStateQueue:
    def __init__(self) -> None:
        self._queue: asyncio.Queue[dict[str, Any]] = asyncio.Queue(maxsize=1)
        self.replaced_count = 0

    def put_latest(self, message: dict[str, Any]) -> None:
        if self._queue.full():
            try:
                self._queue.get_nowait()
                self._queue.task_done()
                self.replaced_count += 1
            except asyncio.QueueEmpty:
                pass
        self._queue.put_nowait(message)

    async def get(self) -> dict[str, Any]:
        return await self._queue.get()

    def task_done(self) -> None:
        self._queue.task_done()

    def qsize(self) -> int:
        return self._queue.qsize()


@dataclass
class Client:
    player_id: int
    writer: asyncio.StreamWriter
    write_lock: asyncio.Lock = field(default_factory=asyncio.Lock)
    state_queue: LatestStateQueue = field(default_factory=LatestStateQueue)
    snapshot: bytes | None = None
    sender_task: asyncio.Task[None] | None = None


@dataclass
class SessionServer:
    token: str | None = None
    share_cutbits: bool = False
    progress: dict[str, Any] = field(default_factory=empty_progress)
    tick_hz: int = TICK_HZ
    session_id: str = field(default_factory=lambda: uuid.uuid4().hex)
    clients: dict[int, Client] = field(default_factory=dict)
    state_seq: int = 0
    dirty: bool = False
    server: asyncio.AbstractServer | None = None
    publisher_task: asyncio.Task[None] | None = None

    def __post_init__(self) -> None:
        self._dirty_event = asyncio.Event()
        self._closed = False

    async def start(self, host: str, port: int) -> asyncio.AbstractServer:
        self._ensure_publisher()
        self.server = await asyncio.start_server(self.handle_client, host, port)
        return self.server

    async def close(self) -> None:
        self._closed = True
        if self.server is not None:
            self.server.close()
            await self.server.wait_closed()
        for player_id in list(self.clients):
            await self._remove_client(player_id, close_writer=True)
        if self.publisher_task is not None:
            self.publisher_task.cancel()
            await asyncio.gather(self.publisher_task, return_exceptions=True)
            self.publisher_task = None

    def _ensure_publisher(self) -> None:
        if self.publisher_task is None or self.publisher_task.done():
            self.publisher_task = asyncio.create_task(self._publisher_loop())

    def _allocate_player_id(self) -> int | None:
        for player_id in (1, 2):
            if player_id not in self.clients:
                return player_id
        return None

    def mark_dirty(self) -> None:
        self.dirty = True
        self._dirty_event.set()

    async def handle_client(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        self._ensure_publisher()
        player_id = 0
        try:
            hello = await read_message(reader)
            validate_hello(hello, self.token)
            player_id = self._allocate_player_id() or 0
            if player_id == 0:
                print("session full")
                await self._send_error(writer, "session full")
                return

            client = Client(player_id=player_id, writer=writer)
            self.clients[player_id] = client
            client.sender_task = asyncio.create_task(self._client_sender(client))
            print(f"player joined: {player_id}")
            await self._send_control(
                client,
                {
                    "type": "WELCOME",
                    "wire_version": WIRE_PROTOCOL_VERSION,
                    "player_id": player_id,
                    "session_id": self.session_id,
                    "tick_hz": self.tick_hz,
                    "progress": self.progress,
                },
            )
            self.mark_dirty()

            while True:
                try:
                    message = validate_client_message(await read_message(reader))
                except ProtocolError as exc:
                    print(f"protocol rejection: {exc}")
                    await self._send_control(client, make_error(str(exc)))
                    return

                mtype = message["type"]
                if mtype == "LOCAL_SNAPSHOT":
                    client.snapshot = message["raw"]
                    if merge_progress(
                        self.progress,
                        progress_from_snapshot(client.snapshot),
                        self.share_cutbits,
                    ):
                        self.mark_dirty()
                    else:
                        self.dirty = True
                elif mtype == "PING":
                    await self._send_control(
                        client,
                        {"type": "PONG", "wire_version": WIRE_PROTOCOL_VERSION},
                    )
                elif mtype == "DISCONNECT":
                    return
        except ProtocolError as exc:
            print(f"protocol rejection: {exc}")
            await self._send_error(writer, str(exc))
        except ValueError as exc:
            print(f"protocol rejection: {exc}")
            client = self.clients.get(player_id)
            if client is not None:
                await self._send_control(client, make_error(str(exc)))
            else:
                await self._send_error(writer, str(exc))
        except (asyncio.IncompleteReadError, ConnectionError, TimeoutError):
            pass
        finally:
            if player_id:
                print(f"player disconnected: {player_id}")
                await self._remove_client(player_id, close_writer=True)

    async def _send_error(self, writer: asyncio.StreamWriter, error: str) -> None:
        try:
            await write_message(writer, make_error(error))
        except (ConnectionError, OSError, ValueError):
            pass

    async def _send_control(self, client: Client, message: dict[str, Any]) -> None:
        async with client.write_lock:
            await write_message(client.writer, message)

    async def _client_sender(self, client: Client) -> None:
        try:
            while True:
                message = await client.state_queue.get()
                try:
                    async with client.write_lock:
                        await write_message(client.writer, message)
                finally:
                    client.state_queue.task_done()
        except asyncio.CancelledError:
            raise
        except (ConnectionError, OSError, ValueError):
            return

    async def _remove_client(self, player_id: int, close_writer: bool) -> None:
        client = self.clients.pop(player_id, None)
        if client is None:
            return
        if client.sender_task is not None:
            client.sender_task.cancel()
            await asyncio.gather(client.sender_task, return_exceptions=True)
        if close_writer:
            try:
                client.writer.close()
                await client.writer.wait_closed()
            except (ConnectionError, OSError):
                pass
        self.mark_dirty()

    async def _publisher_loop(self) -> None:
        interval = 1.0 / float(self.tick_hz)
        next_tick = time.monotonic()
        try:
            while not self._closed:
                now = time.monotonic()
                timeout = max(0.0, next_tick - now)
                if timeout > 0.0:
                    try:
                        await asyncio.wait_for(self._dirty_event.wait(), timeout=timeout)
                        self._dirty_event.clear()
                    except asyncio.TimeoutError:
                        pass
                    continue
                now = time.monotonic()
                while next_tick <= now:
                    next_tick += interval
                if self.dirty and self.clients:
                    self._dirty_event.clear()
                    self.dirty = False
                    self._publish_once()
        except asyncio.CancelledError:
            raise

    def _publish_once(self) -> None:
        self.state_seq = (self.state_seq + 1) & 0xFFFFFFFF
        for client in list(self.clients.values()):
            client.state_queue.put_latest(self._state_for_client(client))

    def _state_for_client(self, client: Client) -> dict[str, Any]:
        remote_raw: bytes | None = None
        for other_id, other in self.clients.items():
            if other_id != client.player_id and other.snapshot is not None:
                remote_raw = other.snapshot
                break
        return {
            "type": "SESSION_STATE",
            "wire_version": WIRE_PROTOCOL_VERSION,
            "session_id": self.session_id,
            "state_seq": self.state_seq,
            "recipient_player_id": client.player_id,
            "progress": self.progress,
            "remote_raw": encode_raw_snapshot(remote_raw),
        }

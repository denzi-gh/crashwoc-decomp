from __future__ import annotations

import asyncio
import time
from typing import Any, Callable

from . import protocol_generated as proto
from .memory import (
    MAILBOX_ADDRESS,
    MemoryAdapter,
    ProgressRevisionMapper,
    make_active_remote_snapshot,
    make_connected_no_remote_snapshot,
    make_disconnected_snapshot,
    read_header,
    read_local_snapshot_consistent,
    require_ready_mailbox,
    refresh_bridge_heartbeat,
    write_inbound_snapshot,
)
from .messages import (
    ProtocolError,
    TICK_HZ,
    U32_MAX,
    make_hello,
    validate_server_message,
    validate_welcome,
)
from .net import read_message, write_message


LogFunc = Callable[[str], None]


class BridgeClient:
    def __init__(
        self,
        adapter: MemoryAdapter,
        host: str,
        port: int,
        token: str | None = None,
        mailbox: int = MAILBOX_ADDRESS,
        reconnect: bool = True,
        log: LogFunc | None = None,
    ) -> None:
        self.adapter = adapter
        self.host = host
        self.port = port
        self.token = token
        self.mailbox = mailbox
        self.reconnect = reconnect
        self.log = log or (lambda message: None)
        self._stopping = False
        self._client_seq = 0
        self._last_state_seq = -1
        self._session_id: str | None = None
        self._mapper: ProgressRevisionMapper | None = None
        self._last_progress: dict[str, object] | None = None
        self._last_connection_succeeded = False

    def stop(self) -> None:
        self._stopping = True

    async def run(self) -> None:
        self.adapter.hook()
        self.log("Dolphin hooked")
        header = require_ready_mailbox(self.adapter, self.mailbox)
        self._mapper = ProgressRevisionMapper(header.last_applied_progress_revision)
        self.log(f"co-op mailbox ready at 0x{self.mailbox:08X}")
        backoff = 0.5
        while not self._stopping:
            self._last_connection_succeeded = False
            try:
                await self.run_connection()
                if not self.reconnect:
                    return
            except asyncio.CancelledError:
                self._stopping = True
                raise
            except Exception as exc:
                if self._stopping:
                    break
                self.log(f"connection lost: {exc}")
                self.write_disconnected_state()
                if not self.reconnect:
                    return
                if self._last_connection_succeeded:
                    backoff = 0.5
            if self._stopping:
                break
            self.log(f"reconnecting in {backoff:.1f}s")
            await asyncio.sleep(backoff)
            backoff = min(backoff * 2.0, 5.0)

    async def run_connection(self) -> None:
        self.log(f"connecting to {self.host}:{self.port}")
        reader, writer = await asyncio.open_connection(self.host, self.port)
        send_task: asyncio.Task[None] | None = None
        receive_task: asyncio.Task[None] | None = None
        try:
            await write_message(writer, make_hello(self.token))
            welcome = validate_welcome(await read_message(reader))
            self._last_connection_succeeded = True
            self._session_id = welcome["session_id"]
            self._last_state_seq = -1
            self.log(
                f"connected player ID {welcome['player_id']} session {self._session_id}"
            )
            self.apply_authoritative_progress(welcome["progress"], remote_raw=None)

            send_task = asyncio.create_task(self.send_loop(writer, int(welcome["tick_hz"])))
            receive_task = asyncio.create_task(self.receive_loop(reader))
            done, pending = await asyncio.wait(
                {send_task, receive_task}, return_when=asyncio.FIRST_EXCEPTION
            )
            for task in pending:
                task.cancel()
            await asyncio.gather(*pending, return_exceptions=True)
            for task in done:
                if task.cancelled():
                    continue
                exc = task.exception()
                if exc is not None:
                    raise exc
        finally:
            pending_tasks = [
                task
                for task in (send_task, receive_task)
                if task is not None and not task.done()
            ]
            for task in pending_tasks:
                task.cancel()
            if pending_tasks:
                await asyncio.gather(*pending_tasks, return_exceptions=True)
            try:
                if not writer.is_closing():
                    await write_message(writer, {"type": "DISCONNECT"})
            except (ConnectionError, OSError, ValueError):
                pass
            writer.close()
            try:
                await writer.wait_closed()
            except (ConnectionError, OSError):
                pass
            self.write_disconnected_state()

    async def send_loop(self, writer: asyncio.StreamWriter, tick_hz: int = TICK_HZ) -> None:
        interval = 1.0 / float(tick_hz)
        next_tick = time.monotonic()
        while not self._stopping:
            now = time.monotonic()
            if now < next_tick:
                await asyncio.sleep(next_tick - now)
                continue
            while next_tick <= now:
                next_tick += interval
            try:
                snapshot = read_local_snapshot_consistent(self.adapter, self.mailbox)
            except Exception as exc:
                raise RuntimeError(f"failed to read Dolphin local snapshot: {exc}") from exc
            if snapshot is None:
                continue
            self._client_seq = (self._client_seq + 1) & U32_MAX
            await write_message(
                writer,
                {
                    "type": "LOCAL_SNAPSHOT",
                    "client_seq": self._client_seq,
                    "raw": snapshot.hex(),
                },
            )

    async def receive_loop(self, reader: asyncio.StreamReader) -> None:
        while not self._stopping:
            message = validate_server_message(await read_message(reader))
            if message["type"] == "ERROR":
                raise RuntimeError(str(message["error"]))
            if message["type"] == "PONG":
                continue
            state = message
            if state["session_id"] != self._session_id:
                raise ProtocolError("server session changed within one connection")
            if state["state_seq"] <= self._last_state_seq:
                continue
            self._last_state_seq = int(state["state_seq"])
            self.apply_authoritative_progress(state["progress"], state["remote_raw"])

    def apply_authoritative_progress(
        self,
        progress: dict[str, object],
        remote_raw: bytes | None,
    ) -> None:
        if self._mapper is None:
            self._mapper = ProgressRevisionMapper(
                read_header(self.adapter, self.mailbox).last_applied_progress_revision
            )
        mapped_progress = self._mapper.map_progress(progress)
        if remote_raw is None:
            inbound = make_connected_no_remote_snapshot(mapped_progress)
        else:
            inbound = make_active_remote_snapshot(remote_raw, mapped_progress)
        write_inbound_snapshot(self.adapter, inbound, self.mailbox)
        refresh_bridge_heartbeat(self.adapter, self.mailbox)
        self._last_progress = mapped_progress

    def write_disconnected_state(self) -> None:
        try:
            inbound = make_disconnected_snapshot(self._last_progress)
            write_inbound_snapshot(self.adapter, inbound, self.mailbox)
            refresh_bridge_heartbeat(self.adapter, self.mailbox)
        except Exception as exc:
            self.log(f"failed to write disconnected state: {exc}")

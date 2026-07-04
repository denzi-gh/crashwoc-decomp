from __future__ import annotations

import asyncio
import json
import struct
from typing import Any


MAX_MESSAGE_SIZE = 65536


async def read_message(reader: asyncio.StreamReader) -> Any:
    header = await asyncio.wait_for(reader.readexactly(4), timeout=10.0)
    size = struct.unpack(">I", header)[0]
    if size > MAX_MESSAGE_SIZE:
        raise ValueError(f"message too large: {size}")
    payload = await asyncio.wait_for(reader.readexactly(size), timeout=10.0)
    return json.loads(payload.decode("utf-8"))


async def write_message(writer: asyncio.StreamWriter, message: dict[str, Any]) -> None:
    payload = json.dumps(message, separators=(",", ":"), sort_keys=True).encode("utf-8")
    if len(payload) > MAX_MESSAGE_SIZE:
        raise ValueError(f"message too large: {len(payload)}")
    writer.write(struct.pack(">I", len(payload)) + payload)
    await writer.drain()

from __future__ import annotations

import copy
from typing import Any

from . import protocol_generated as proto


WIRE_PROTOCOL_VERSION = 1
TICK_HZ = 20
MAX_TOKEN_LENGTH = 128
MAX_SESSION_ID_LENGTH = 64
U32_MAX = 0xFFFFFFFF


class ProtocolError(ValueError):
    pass


def _require_object(message: Any) -> dict[str, Any]:
    if not isinstance(message, dict):
        raise ProtocolError("message must be a JSON object")
    mtype = message.get("type")
    if not isinstance(mtype, str):
        raise ProtocolError("message type is required")
    return message


def _require_type(message: dict[str, Any], expected: str) -> None:
    if message.get("type") != expected:
        raise ProtocolError(f"expected {expected}")


def _require_int(message: dict[str, Any], key: str, minimum: int = 0, maximum: int = U32_MAX) -> int:
    value = message.get(key)
    if isinstance(value, bool) or not isinstance(value, int):
        raise ProtocolError(f"{key} must be an integer")
    if value < minimum or value > maximum:
        raise ProtocolError(f"{key} out of range")
    return value


def _require_optional_token(message: dict[str, Any]) -> str | None:
    token = message.get("token")
    if token is None:
        return None
    if not isinstance(token, str):
        raise ProtocolError("token must be a string or null")
    if len(token) > MAX_TOKEN_LENGTH:
        raise ProtocolError("token is too long")
    return token


def _require_string(message: dict[str, Any], key: str, max_length: int) -> str:
    value = message.get(key)
    if not isinstance(value, str):
        raise ProtocolError(f"{key} must be a string")
    if not value or len(value) > max_length:
        raise ProtocolError(f"{key} has invalid length")
    return value


def decode_raw_snapshot(raw: Any) -> bytes:
    if not isinstance(raw, str):
        raise ProtocolError("raw snapshot must be hexadecimal text")
    if len(raw) % 2 != 0:
        raise ProtocolError("raw snapshot hex has odd length")
    try:
        decoded = bytes.fromhex(raw)
    except ValueError as exc:
        raise ProtocolError("raw snapshot is not valid hexadecimal") from exc
    if len(decoded) != proto.SNAPSHOT_SIZE:
        raise ProtocolError(
            f"raw snapshot must decode to {proto.SNAPSHOT_SIZE} bytes"
        )
    return decoded


def encode_raw_snapshot(raw: bytes | None) -> str | None:
    if raw is None:
        return None
    if len(raw) != proto.SNAPSHOT_SIZE:
        raise ProtocolError("raw snapshot has invalid length")
    return raw.hex()


def _validate_wire_version(message: dict[str, Any]) -> None:
    wire_version = _require_int(message, "wire_version", 0, U32_MAX)
    if wire_version != WIRE_PROTOCOL_VERSION:
        raise ProtocolError("incompatible wire version")


def validate_hello(message: Any, token: str | None = None) -> dict[str, Any]:
    message = _require_object(message)
    _require_type(message, "HELLO")
    _validate_wire_version(message)
    abi_version = _require_int(message, "abi_version", 0, 0xFFFF)
    if abi_version != proto.ABI_VERSION:
        raise ProtocolError("incompatible ABI")
    build_id = _require_int(message, "build_id", 0, U32_MAX)
    if build_id != proto.BUILD_ID:
        raise ProtocolError("incompatible build")
    hello_token = _require_optional_token(message)
    if token is not None and hello_token != token:
        raise ProtocolError("bad token")
    return {
        "type": "HELLO",
        "wire_version": WIRE_PROTOCOL_VERSION,
        "abi_version": abi_version,
        "build_id": build_id,
        "token": hello_token,
    }


def make_hello(token: str | None = None) -> dict[str, Any]:
    return {
        "type": "HELLO",
        "wire_version": WIRE_PROTOCOL_VERSION,
        "abi_version": proto.ABI_VERSION,
        "build_id": proto.BUILD_ID,
        "token": token,
    }


def make_error(error: str) -> dict[str, Any]:
    return {
        "type": "ERROR",
        "wire_version": WIRE_PROTOCOL_VERSION,
        "error": error[:256],
    }


def validate_client_message(message: Any) -> dict[str, Any]:
    message = _require_object(message)
    mtype = message["type"]
    if mtype == "LOCAL_SNAPSHOT":
        client_seq = _require_int(message, "client_seq", 0, U32_MAX)
        raw = decode_raw_snapshot(message.get("raw"))
        return {"type": mtype, "client_seq": client_seq, "raw": raw}
    if mtype == "PING":
        return {"type": mtype}
    if mtype == "DISCONNECT":
        return {"type": mtype}
    raise ProtocolError(f"unsupported message type: {mtype}")


def validate_progress(progress: Any) -> dict[str, Any]:
    if not isinstance(progress, dict):
        raise ProtocolError("progress must be an object")
    result: dict[str, Any] = {}
    result["revision"] = _require_int(progress, "revision", 0, U32_MAX)
    level_flags = progress.get("level_flags")
    if not isinstance(level_flags, list) or len(level_flags) != 35:
        raise ProtocolError("level_flags must have 35 entries")
    result["level_flags"] = [
        _progress_list_int(level_flags, i, "level_flags", 0, 0xFFFF)
        for i in range(35)
    ]
    hub_flags = progress.get("hub_flags")
    if not isinstance(hub_flags, list) or len(hub_flags) != 6:
        raise ProtocolError("hub_flags must have 6 entries")
    result["hub_flags"] = [
        _progress_list_int(hub_flags, i, "hub_flags", 0, 0xFF)
        for i in range(6)
    ]
    hub_crystals = progress.get("hub_crystals")
    if not isinstance(hub_crystals, list) or len(hub_crystals) != 6:
        raise ProtocolError("hub_crystals must have 6 entries")
    result["hub_crystals"] = [
        _progress_list_int(hub_crystals, i, "hub_crystals", 0, 0xFF)
        for i in range(6)
    ]
    result["powerbits"] = _require_int(progress, "powerbits", 0, 0xFF)
    result["gembits"] = _require_int(progress, "gembits", 0, 0xFF)
    result["cutbits"] = _require_int(progress, "cutbits", 0, U32_MAX)
    return result


def _progress_list_int(
    values: list[Any], index: int, key: str, minimum: int, maximum: int
) -> int:
    value = values[index]
    if isinstance(value, bool) or not isinstance(value, int):
        raise ProtocolError(f"{key}[{index}] must be an integer")
    if value < minimum or value > maximum:
        raise ProtocolError(f"{key}[{index}] out of range")
    return value


def validate_welcome(message: Any) -> dict[str, Any]:
    message = _require_object(message)
    _require_type(message, "WELCOME")
    _validate_wire_version(message)
    player_id = _require_int(message, "player_id", 1, 2)
    session_id = _require_string(message, "session_id", MAX_SESSION_ID_LENGTH)
    tick_hz = _require_int(message, "tick_hz", 1, 120)
    return {
        "type": "WELCOME",
        "wire_version": WIRE_PROTOCOL_VERSION,
        "player_id": player_id,
        "session_id": session_id,
        "tick_hz": tick_hz,
        "progress": validate_progress(message.get("progress")),
    }


def validate_session_state(message: Any) -> dict[str, Any]:
    message = _require_object(message)
    _require_type(message, "SESSION_STATE")
    _validate_wire_version(message)
    session_id = _require_string(message, "session_id", MAX_SESSION_ID_LENGTH)
    state_seq = _require_int(message, "state_seq", 0, U32_MAX)
    recipient = _require_int(message, "recipient_player_id", 1, 2)
    remote_raw_value = message.get("remote_raw")
    remote_raw = None if remote_raw_value is None else decode_raw_snapshot(remote_raw_value)
    return {
        "type": "SESSION_STATE",
        "wire_version": WIRE_PROTOCOL_VERSION,
        "session_id": session_id,
        "state_seq": state_seq,
        "recipient_player_id": recipient,
        "progress": validate_progress(message.get("progress")),
        "remote_raw": remote_raw,
    }


def validate_server_message(message: Any) -> dict[str, Any]:
    message = _require_object(message)
    mtype = message["type"]
    if mtype == "WELCOME":
        return validate_welcome(message)
    if mtype == "SESSION_STATE":
        return validate_session_state(message)
    if mtype == "PONG":
        _validate_wire_version(message)
        return {"type": "PONG", "wire_version": WIRE_PROTOCOL_VERSION}
    if mtype == "ERROR":
        _validate_wire_version(message)
        error = message.get("error")
        if not isinstance(error, str):
            raise ProtocolError("error must be a string")
        return {"type": "ERROR", "wire_version": WIRE_PROTOCOL_VERSION, "error": error}
    raise ProtocolError(f"unsupported message type: {mtype}")


def progress_content_key(progress: dict[str, Any]) -> tuple[Any, ...]:
    normalized = validate_progress(progress)
    return (
        tuple(normalized["level_flags"]),
        tuple(normalized["hub_flags"]),
        tuple(normalized["hub_crystals"]),
        normalized["powerbits"],
        normalized["gembits"],
        normalized["cutbits"],
    )


def clone_progress(progress: dict[str, Any]) -> dict[str, Any]:
    return copy.deepcopy(validate_progress(progress))

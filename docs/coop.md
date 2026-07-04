# Crash WOC Dolphin Co-op MVP

This branch adds an experimental two-player online co-op MVP for `GCBE7D`, the GameCube USA Rev 0 release.

## Legal Requirements

You need your own legal extracted game dump. Do not commit or distribute ISOs, memory cards, original assets, original `main.dol`, or patched DOLs.

## Build

Normal retail builds are unchanged:

```sh
python configure.py --version GCBE7D --toolchain prodg35
ninja
```

Build the co-op DOL in a separate directory:

```sh
python configure.py --version GCBE7D --toolchain prodg35 --build-dir build-coop --coop
ninja build-coop/GCBE7D/main_coop_verify.ok
```

Output:

```text
build-coop/GCBE7D/main_coop.dol
```

The verifier checks that only approved hook and arena-preservation words changed, hook targets enter `.coop_text`, the mailbox is inside `.coop_data`, `__ArenaLo` follows the reservation, and the update/draw wrappers keep their intended responsibilities.

## Install

Replace `sys/main.dol` in a legal extracted `GCBE7D` dump with `build-coop/GCBE7D/main_coop.dol`. Keep a copy of the original `sys/main.dol` so you can restore it.

Keep the repo's `orig/GCBE7D/sys/main.dol` as the clean retail input for builds. If you want to launch from an extracted directory, use a second extracted copy for Dolphin and replace `sys/main.dol` there. Replacing the repo input DOL will make dtk stop with a retail hash mismatch on the next configure/build.

## Python Setup

Use Python 3.9 or newer:

```sh
python -m pip install dolphin-memory-engine
```

The bridge also has fake-memory tests that do not require Dolphin:

```sh
python -m unittest discover tools/coop_bridge/tests
```

## Runtime Model

The update hook calls the original `UpdatePlayerStats()` first, then runs `CoopFrameUpdate()`. That is where the game heartbeat advances, the local snapshot is published, and inbound progress is applied.

The draw hook calls the original `DrawCreatures()` first, then runs `CoopDrawRemotePlayer()`. Rendering the remote avatar does not publish local state.

## Diagnose

Boot the co-op DOL in Dolphin, then run:

```sh
python -m tools.coop_bridge diagnose
```

This prints Dolphin hook status, mailbox magic, ABI version, build ID, heartbeat, current level, local position, and compatibility status.

## One-Dolphin Avatar Test

Run:

```sh
python -m tools.coop_bridge inject-avatar --offset-x 2.0
```

This mirrors the local avatar into the inbound remote snapshot with an X offset. Use `Ctrl+C` to stop.

User-verified result:
A second non-colliding Crash was visible and mirrored the local player.

Runtime details still to record:
Dolphin version, host OS, tested level, test duration, and observed animation limitations.

To test location hiding:

```sh
python -m tools.coop_bridge inject-avatar --offset-x 2.0 --different-level --prime-visible 1.0
```

This uses one bridge writer: it mirrors a visible remote Crash briefly, then switches the same process to the different-location hidden snapshot. Do not run a normal `inject-avatar` process and a `--different-level` process at the same time; the bridge rejects that because competing writers can make the inbound avatar flicker.

For runtime diagnostics:

```sh
python -m tools.coop_bridge debug-log --interval 0.1
```

In the hidden-location test, `mailbox_inbound_level` should stay at the sentinel value `-1073741824`, and the game debug reason should report `location_mismatch`.

## PR2 LAN Host/Join

PR2 adds a robust LAN/development session server plus one local bridge client per
Dolphin instance. Two computers can connect to the same host process, publish
their own local Crash snapshots independently, and receive the other player's
latest validated snapshot. The transport runs at one fixed 20 Hz session tick;
it does not interpolate or extrapolate yet, so visible stepping or jitter is
expected on real networks.

The host process is only the session server. To play on the host computer, use
two terminals.

Host computer, terminal 1:

```sh
python -m tools.coop_bridge host --bind 0.0.0.0 --port 24827 --token test-session
```

Host computer, terminal 2:

```sh
python -m tools.coop_bridge join 127.0.0.1 --port 24827 --token test-session
```

Second computer:

```sh
python -m tools.coop_bridge join <host-lan-ip> --port 24827 --token test-session
```

Allow the TCP port through the host firewall. The shared token is sent over the
TCP connection as plain JSON; it is not encrypted. This host is for trusted LAN
and development use only and must not be exposed directly to the public
internet.

Room-code hosting, public relays, TLS, NAT traversal, accounts, and internet
matchmaking are deferred to later work.

The bridge applies authoritative shared progress from `WELCOME` immediately,
even when only one client is connected. The server retains merged progress only
for the lifetime of the current host process. Disconnects are explicit: the
remaining client receives `remote_raw: null`, writes an inactive inbound avatar,
and the remote Crash should disappear promptly. `join` reconnects
automatically after unexpected network failure; use `--no-reconnect` for
deterministic manual testing.

## Bridge Wire Protocol

The GameCube mailbox ABI remains version 1. PR2 adds a separate bridge wire
protocol version:

```text
WIRE_PROTOCOL_VERSION = 1
```

Frames are unchanged: a 4-byte unsigned big-endian payload size followed by a
UTF-8 JSON payload.

Client to server:

```json
{"type":"HELLO","wire_version":1,"abi_version":1,"build_id":1195582535,"token":null}
{"type":"LOCAL_SNAPSHOT","client_seq":123,"raw":"<176-byte snapshot as hex>"}
{"type":"PING"}
{"type":"DISCONNECT"}
```

Server to client:

```json
{"type":"WELCOME","wire_version":1,"player_id":1,"session_id":"<server epoch>","tick_hz":20,"progress":{}}
{"type":"SESSION_STATE","wire_version":1,"session_id":"<server epoch>","state_seq":456,"recipient_player_id":1,"progress":{},"remote_raw":null}
{"type":"SESSION_STATE","wire_version":1,"session_id":"<server epoch>","state_seq":457,"recipient_player_id":1,"progress":{},"remote_raw":"<other player's snapshot as hex>"}
{"type":"PONG","wire_version":1}
{"type":"ERROR","wire_version":1,"error":"<reason>"}
```

`remote_raw: null` is the explicit no-remote-player state. The server supports
two connected players and does not send a dictionary of all players.

## What Syncs

- level award flags
- hub flags
- hub crystal counts
- powerbits
- gembits
- remote avatar position/rotation/animation in supported same-location on-foot contexts

Progress merging is monotonic and idempotent.

## What Stays Local

- lives
- Wumpa
- mask state
- checkpoints
- crates
- enemies
- bosses
- vehicles
- collision
- time-trial records
- player names
- language/audio/settings

## Automated Fake-Memory Verification

The Python test suite covers deterministic fake-memory networking, including:

- two independent `BridgeClient` instances connected to one `SessionServer`
- newest-only bounded state delivery
- immediate no-remote state on disconnect
- reconnect restoring remote delivery
- authoritative progress delivery from `WELCOME`
- local progress delivery revisions mapped above the mailbox's last applied
  revision

Run:

```sh
python -m unittest discover tools/coop_bridge/tests
```

## User-Verified Dolphin Runtime Result

Preserved existing results:

- A second non-colliding Crash was visible and mirrored the local player.
- The different-location hide test stayed hidden after priming the avatar from
  the same bridge process.

PR2 LAN result reported by the project owner:

- A remote Crash was visible and could be moved through the network session.
- The remote Crash disappeared when one player entered a level. The transition
  was not instant, but that polish is deferred.
- After exiting a level with a collected crystal, the crystal appeared on both
  accounts through authoritative shared progress.

Runtime metadata still to record:
Dolphin version, host OS, client OS, tested levels, test duration, observed
latency, observed animation issues, and exact progress items tested.

## Limitations

This is not Dolphin Netplay and does not synchronize full simulation state. Each player can be in a different level. Remote Crash is visual only: no collision, no AI, no pickups, no damage authority, no particles, no sounds, and no camera influence.

Collision, movement interpolation, synchronized world objects, crates, enemies,
bosses, checkpoints, vehicles, sounds, particles, shared damage, and shared
camera are not part of PR2.

Runtime verification details still need Dolphin; see `docs/coop-runtime-checklist.md`.

## Recovery

Stop the bridge. The game should remain playable and the remote avatar should time out after about three seconds. To remove the mod, restore the original `sys/main.dol` from your legal extracted dump.

## Protocol Versioning

The mailbox ABI is generated from `tools/coop/protocol_schema.json`. Change the ABI version whenever field order, field size, struct size, or semantics change. Regenerate with:

```sh
python tools/coop/generate_protocol.py
```

Check generated files with:

```sh
python tools/coop/generate_protocol.py --check
```

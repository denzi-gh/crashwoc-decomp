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

## LAN Host/Join

Two-PC networking is not yet considered verified for PR1.

On the host computer:

```sh
python -m tools.coop_bridge host --bind 0.0.0.0 --port 24827
```

On each player computer:

```sh
python -m tools.coop_bridge join <host-ip> --port 24827
```

Optional shared token:

```sh
python -m tools.coop_bridge host --token secret
python -m tools.coop_bridge join <host-ip> --token secret
```

Allow the TCP port through the host firewall.

Room-code hosting is not part of PR1.

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

## Limitations

This is not Dolphin Netplay and does not synchronize full simulation state. Each player can be in a different level. Remote Crash is visual only: no collision, no AI, no pickups, no damage authority, no particles, no sounds, and no camera influence.

Collision and synchronized world objects are not part of PR1.

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

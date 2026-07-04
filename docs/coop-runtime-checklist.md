# Co-op Runtime Checklist

Static implementation and tooling are present. A one-Dolphin mirror-avatar result has been observed by the project owner, but the exact Dolphin version, host OS, tested level, and test duration are not recorded in repository notes.

User-verified result:
A second non-colliding Crash was visible and mirrored the local player.

Runtime details still to record:
Dolphin version, host OS, tested level, test duration, and observed animation limitations.

Two-PC networking is not yet considered verified. Room-code hosting, collision, synchronized crates, enemies, bosses, vehicles, and other world-object synchronization are not part of PR1.

## Static Preflight

Build the co-op DOL:

```sh
python configure.py --version GCBE7D --toolchain prodg35 --build-dir build-coop --coop
ninja build-coop/GCBE7D/main_coop_verify.ok
```

The build verifies the strict DOL patch set and checks that state publication runs from the update wrapper, while remote rendering remains in the draw wrapper.

## One-Dolphin Manual Checklist

1. Boot the verified co-op DOL.

2. Run:

   ```sh
   python -m tools.coop_bridge diagnose
   ```

3. Confirm the game heartbeat advances during normal gameplay.

4. Run:

   ```sh
   python -m tools.coop_bridge inject-avatar --offset-x 2.0
   ```

5. Confirm the mirrored Crash is still visible.

6. Pause, enter a menu, trigger a fade, and return to gameplay.

7. Confirm the game does not crash.

8. Confirm the remote Crash returns correctly.

9. Stop the bridge.

10. Confirm the remote Crash disappears after the stale timeout and the game continues.

## Location-Hiding Check

Run:

```sh
python -m tools.coop_bridge inject-avatar --different-level
```

Expected: remote Crash is hidden, local snapshot publication continues, and the game heartbeat continues to advance.

## Two-PC Checks To Record Later

1. Start host bridge:

   ```sh
   python -m tools.coop_bridge host --bind 0.0.0.0 --port 24827
   ```

2. Start each Dolphin instance with the co-op DOL and one local bridge:

   ```sh
   python -m tools.coop_bridge join <host-ip> --port 24827
   ```

3. Put both players in the same supported on-foot level section.

4. Confirm each side sees the other player as a non-colliding Crash.

5. Move one player to a different level and confirm both games continue independently.

6. Confirm level flags, crystals, gems, relics, hub unlock progress, powerbits, and gembits propagate monotonically.

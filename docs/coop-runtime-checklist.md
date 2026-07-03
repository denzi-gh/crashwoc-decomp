# Co-op Runtime Checklist

Static implementation and tooling are present, but Dolphin runtime verification still needs to be performed on a machine running Dolphin with the co-op DOL.

## One-Dolphin Checks

1. Build the co-op DOL:

   ```sh
   python configure.py --version GCBE7D --toolchain prodg35 --build-dir build-coop --coop
   ninja build-coop/GCBE7D/main_coop_verify.ok
   ```

2. Install `build-coop/GCBE7D/main_coop.dol` into a legal extracted `GCBE7D` dump.

3. Boot it in Dolphin.

4. Run:

   ```sh
   python -m tools.coop_bridge diagnose
   ```

   Expected: good magic, ABI `1`, build ID `0x47432447`, and advancing game heartbeat.

5. Run:

   ```sh
   python -m tools.coop_bridge inject-avatar --offset-x 2.0
   ```

   Expected: a second non-colliding Crash appears in supported on-foot contexts.

6. Run:

   ```sh
   python -m tools.coop_bridge inject-avatar --different-level
   ```

   Expected: remote Crash is hidden.

7. Stop the bridge.

   Expected: game continues without pause, crash, reset, or blocking; remote avatar disappears after about three seconds.

## Two-Dolphin Checks

1. Start host bridge:

   ```sh
   python -m tools.coop_bridge host --bind 0.0.0.0 --port 24827
   ```

2. Start each Dolphin instance with the co-op DOL and one local bridge:

   ```sh
   python -m tools.coop_bridge join <host-ip> --port 24827
   ```

3. Put both players in the same supported on-foot level section.

   Expected: each sees the other player as a non-colliding Crash.

4. Move one player to a different level.

   Expected: both games continue independently; remote avatar is hidden while locations differ.

5. Collect level awards on either side.

   Expected: level flags, crystals, gems, relics, hub unlock progress, powerbits, and gembits propagate monotonically.

6. Confirm excluded fields remain local:

   - lives
   - Wumpa
   - mask
   - checkpoint state
   - player names
   - language/audio/settings
   - time-trial records


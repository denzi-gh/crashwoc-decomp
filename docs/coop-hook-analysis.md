# Co-op Hook Analysis

Target: Crash Bandicoot: The Wrath of Cortex, GameCube USA Rev 0 (`GCBE7D`).

Evidence sources:

- `build/GCBE7D/asm/main.s`
- `tools/coop/hooks/GCBE7D.json`
- `build-coop/GCBE7D/main_coop_patch.json`
- `tools/coop/verify_wrappers.py`

## Update Hook

Hook address: `0x80052740`

Original opcode: `0x4800DB3D`

Original target: `UpdatePlayerStats` at `0x8006027C`

Wrapper: `CoopUpdatePlayerStatsWrapper`

Required wrapper behavior:

```c
void CoopUpdatePlayerStatsWrapper(struct creature_s *plr)
{
    UpdatePlayerStats(plr);
    CoopFrameUpdate(plr);
}
```

State publication, `game_heartbeat` advancement, inbound snapshot reads, and inbound progress application run from this update hook. This gives one normal gameplay `CoopFrameUpdate()` call per hooked player update.

Surrounding original instructions:

```asm
8005272C: 4B FD CF 81  bl      UpdateScreenWumpas
80052730: 80 0D 82 1C  lwz     r0, PLAYERCOUNT@sda21(r0)
80052734: 2C 00 00 00  cmpwi   r0, 0
80052738: 41 82 00 0C  beq     0x80052744
8005273C: 7F 23 CB 78  mr      r3, r25
80052740: 48 00 DB 3D  bl      UpdatePlayerStats
80052744: 48 00 E0 D1  bl      UpdatePanelDebris
```

Rationale: this call already has the player pointer in `r3` and runs from normal player-update gameplay flow. The wrapper preserves the original call first, then publishes/applies co-op mailbox state.

## Draw Hook

Hook address: `0x80052AB4`

Original opcode: `0x4BFCB3A9`

Original target: `DrawCreatures` at `0x8001DE5C`

Wrapper: `CoopDrawCreaturesWrapper`

Required wrapper behavior:

```c
void CoopDrawCreaturesWrapper(
    struct creature_s *c,
    int count,
    int render,
    int shadow)
{
    DrawCreatures(c, count, render, shadow);
    CoopDrawRemotePlayer();
}
```

The draw hook preserves the original local-player draw call and then draws the remote avatar. It must not call `CoopFrameUpdate()` and must not publish local state.

Surrounding original instructions:

```asm
80052A9C: 38 0F B6 C8  addi    r0, r15, GameCam@l
80052AA0: 38 71 CA D8  addi    r3, r17, Character@l
80052AA4: 90 0D A1 24  stw     r0, pCam@sda21(r0)
80052AA8: 38 80 00 01  li      r4, 1
80052AAC: 7F C5 F3 78  mr      r5, r30
80052AB0: 38 C0 00 01  li      r6, 1
80052AB4: 4B FC B3 A9  bl      DrawCreatures
80052AB8: 81 2D 85 D0  lwz     r9, FRAMES@sda21(r0)
```

Rationale: this is the first `DrawCreatures` call for `Character[0]`. Later calls draw `Character + 1`, `OppTubCreature`, or the panel/death pass, so they are not used for the MVP remote Crash draw.

## Current Co-op Reservation

Current linked sections:

- `.coop_text`: `0x803E6000`
- `.coop_data`: `0x803F6000`
- `gCoopMailbox`: inside `.coop_data`
- `__ArenaLo`: after the co-op reservation

Verifier target:

```sh
ninja build-coop/GCBE7D/main_coop_verify.ok
```

The co-op verifier checks the DOL patch set and the wrapper-call verifier checks that the update wrapper calls `UpdatePlayerStats` then `CoopFrameUpdate`, while the draw wrapper calls `DrawCreatures` then `CoopDrawRemotePlayer` and does not call `CoopFrameUpdate`.

## Runtime Status

User-verified result:
A second non-colliding Crash was visible and mirrored the local player.

Runtime details still to record:
Dolphin version, host OS, tested level, test duration, and observed animation limitations.

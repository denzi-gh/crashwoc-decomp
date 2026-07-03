# Co-op Hook Analysis

Target: Crash Bandicoot: The Wrath of Cortex, GameCube USA Rev 0 (`GCBE7D`).

Evidence sources:

- `build/GCBE7D/asm/main.s`
- `build-coop/GCBE7D/main_coop_patch.json`
- `tools/coop/hooks/GCBE7D.json`

## Update Hook

Hook address: `0x80052740`

Original opcode: `0x4800DB3D`

Original target: `UpdatePlayerStats` at `0x8006027C`

Wrapper: `CoopUpdatePlayerStatsWrapper`

Patched target in the current co-op build: `0x803E693C`

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

Rationale: this call runs after the per-frame simulation/player HUD update path and already has the player pointer in `r3`. The wrapper preserves the original call and then publishes/applies co-op mailbox state.

## Draw Hook

Hook address: `0x80052AB4`

Original opcode: `0x4BFCB3A9`

Original target: `DrawCreatures` at `0x8001DE5C`

Wrapper: `CoopDrawCreaturesWrapper`

Patched target in the current co-op build: `0x803E6960`

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

- `.coop_text`: `0x803E6000`, size about `0x9A0`
- `.coop_data`: `0x803F6000`, size about `0x220`
- `gCoopMailbox`: `0x803F6000`
- `__ArenaLo`: `0x803FA000`

Verifier target:

```sh
ninja build-coop/GCBE7D/main_coop_verify.ok
```


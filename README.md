Crash Bandicoot: The Wrath of Cortex (GameCube)
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] [![Discord Badge]][discord]
=============

[Build Status]: https://github.com/denzi-gh/crashwoc-decomp/actions/workflows/decomp-dev.yml/badge.svg
[actions]: https://github.com/denzi-gh/crashwoc-decomp/actions/workflows/decomp-dev.yml
[Code Progress]: https://decomp.dev/denzi-gh/crashwoc-decomp.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/denzi-gh/crashwoc-decomp.svg?mode=shield&measure=data&label=Data
[progress]: https://decomp.dev/denzi-gh/crashwoc-decomp
[Discord Badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[discord]: https://discord.gg/kmCPpW4KvJ

A work-in-progress decompilation of Crash Bandicoot: The Wrath of Cortex for GameCube.

This repository does **not** contain game assets. You must provide your own dumped game files.

Supported versions:

- `GCBE7D`: Rev 0 (USA)

Dependencies
============

Windows
-------

On Windows, native tooling is the intended local setup.

- Install [Python](https://www.python.org/downloads/) and add it to `PATH`.
- Install [ninja](https://github.com/ninja-build/ninja/releases) and add it to `PATH`.
  - Quick install: `pip install ninja`
- Install [Git](https://git-scm.com/downloads).

Most project tools are downloaded automatically on first configure:

- `decomp-toolkit`
- `objdiff-cli`
- binutils
- compiler packages

Linux / macOS
-------------

Local development is primarily tested on Windows. CI uses a private Linux container for protected build assets and compiler execution.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/denzi-gh/crashwoc-decomp.git
  cd crashwoc-decomp
  ```

- Provide the original game executable:

  ```text
  orig/GCBE7D/sys/main.dol
  ```

  Expected SHA-1:

  ```text
  c9cbd49a9eb0006f55533eb7d0fb5ebe2a73b72f
  ```

- Configure:

  ```sh
  python configure.py --version GCBE7D --toolchain prodg35
  ```

- Build:

  ```sh
  ninja
  ```

Project layout
==============

- `config/GCBE7D/splits.txt`: object and section ownership for the retail DOL
- `config/GCBE7D/symbols.txt`: validated symbol names
- `config/GCBE7D/ldscript.ld`: checked-in ProDG linker script
- `src/`: recovered source tree
- `orig/GCBE7D/`: required original game files

Diffing
=======

Once the initial build succeeds, `objdiff.json` will be generated in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Set the project directory to this repository and the configuration should load automatically.

AI-assisted workflows
=====================

Repository AI guidance lives in:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- `.github/instructions/*.instructions.md`
- `.github/prompts/match-unit.prompt.md`

Repo-local helper commands include:

- `python tools/ai_context.py <symbol-or-unit>`
- `python tools/ai_lookup_symbol.py <symbol-or-address>`
- `python tools/ai_lookup_unit.py <unit-or-path>`
- `python tools/ai_match_plan.py <unit-or-path>`



Please always review AI generated Code. The workflows are designed in way that prevents the AI to generate slop like inline Assembly and compiler specific patterns that no human would write but you never know!

Pull Requests
=============

Pull requests are welcome.

For build or decomp changes, keep them focused and include:

- what changed
- what was verified locally
- whether matching, build behavior, or tooling behavior changed

Community
=========

Discord: https://discord.gg/kmCPpW4KvJ
 

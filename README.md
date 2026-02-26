# opencrashwoc

this repo is for decomp work on **crash bandicoot: the wrath of cortex** (gamecube, ntsc-u `GCBE7D`).

it uses:
- `decomp-toolkit`
- dtk-template style workflow
- prodg 3.5 profile (`ngccc` / `ngcld`)

important: no game assets are included here. you need your own dumped files.

## current status

- `ninja` runs progress by default
- `ninja progress` and `ninja report` work
- full matching / full rebuild is still wip

## what you need (windows)

- python 3.11+ in `PATH`
- ninja in `PATH`
- git

most tools (`dtk`, `objdiff`, compilers package) get downloaded automatically on first run.

## quick setup

1. clone:
```powershell
git clone https://github.com/denzi-gh/crashwoc-decomp.git
cd crashwoc-decomp
```

2. put your original dol here:
`orig/GCBE7D/sys/main.dol`

expected sha1:
`c9cbd49a9eb0006f55533eb7d0fb5ebe2a73b72f`

3. configure:
```powershell
py -3 configure.py --version GCBE7D --toolchain prodg35
```

4. run progress:
```powershell
ninja progress
```

you can also just run `ninja` (same default target right now).

## common commands

reconfigure:
```powershell
py -3 configure.py --version GCBE7D --toolchain prodg35
```

generate progress report json:
```powershell
ninja report
```

build all currently configured source units:
```powershell
ninja
```

## symbols + splits workflow

- `config/GCBE7D/splits.txt` = which address range belongs to which source unit/section
- `config/GCBE7D/symbols.txt` = validated symbol names at concrete addresses
- `src/dump_alphaNGCport_DWARF.txt` = strong hint source, but still validate against the retail dol layout

draft helper:
```powershell
py -3 tools/extract_symbol_drafts.py --version GCBE7D
``

## community

discord: https://discord.gg/kmCPpW4KvJ


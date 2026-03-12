# Build Flow

## Configure

Run configure after changing repo config or split inputs:

```sh
python configure.py --version GCBE7D --toolchain prodg35
```

## Main Targets

```sh
ninja
ninja progress
ninja baseline
ninja changes
```

## Narrow Iteration

Build a single object:

```sh
ninja build/GCBE7D/src/<unit>.o
```

Build a single context target:

```sh
ninja build/GCBE7D/src/<unit>.ctx
```

Use the lookup helpers to find the normalized unit name first.

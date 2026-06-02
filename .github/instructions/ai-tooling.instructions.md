---
applyTo: "AGENTS.md,README.md,.github/**/*.md,tools/skills/**/*.md,tools/ai_*.py,tools/mcp_decomp.py,tools/validate_ai_tooling.py"
---

# AI Tooling Instructions

- The provider-neutral task-pack workflow is the primary architecture: `tools/ai_task.py` generates task packs and `tools/ai_run.py` runs them against a backend. Copilot-specific entry points (`tools/ai_launch_copilot.py`, prompt files) remain only as compatibility.
- Keep `AGENTS.md`, `README.md`, `.github/copilot-instructions.md`, `.github/instructions/*.instructions.md`, prompt files, and repo-local skills aligned when workflow commands change.
- Keep SKILL frontmatter concise and descriptive; move detailed procedures into reference files when needed.
- Prefer helper scripts over repeating long procedural text when the same lookup or planning step happens often.
- Reuse the `ai_common.py` Python API (and existing `ai_context.py` / `ai_decompme_zip.py` helpers) instead of re-parsing repo metadata in new scripts.
- When you add or rename an AI helper script, update the docs that mention the helper and extend `tools/validate_ai_tooling.py`. Only add a path to `REQUIRED_FILES` once that file actually exists, or validation will fail.
- README and/or `AGENTS.md` must keep describing the task-pack workflow (`tools/ai_task.py`, `tools/ai_run.py`) and the Copilot wrapper; `validate_ai_tooling.py` checks for these mentions.
- The MCP scaffold (`tools/mcp_decomp.py`) is read-only: do not expose build/write or arbitrary-shell tools without explicit, non-interactive gating.
- Validate AI tooling changes with `python tools/validate_ai_tooling.py`.
- Keep instructions explicit about non-interactive commands, single-object iteration, and stop/skip rules so agents do not hang on open-ended loops.

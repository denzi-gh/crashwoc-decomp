---
applyTo: "AGENTS.md,README.md,.github/**/*.md,tools/skills/**/*.md,tools/ai_*.py,tools/validate_ai_tooling.py"
---

# AI Tooling Instructions

- Keep `AGENTS.md`, `.github/copilot-instructions.md`, `.github/instructions/*.instructions.md`, prompt files, and repo-local skills aligned when workflow commands change.
- Keep SKILL frontmatter concise and descriptive; move detailed procedures into reference files when needed.
- Prefer helper scripts over repeating long procedural text when the same lookup or planning step happens often.
- When you add or rename an AI helper script, update the docs that mention the helper and extend `tools/validate_ai_tooling.py`.
- Validate AI tooling changes with `python tools/validate_ai_tooling.py`.
- Keep instructions explicit about non-interactive commands, single-object iteration, and stop/skip rules so agents do not hang on open-ended loops.

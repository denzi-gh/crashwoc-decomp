# Task Pack Backends

`python tools/ai_run.py --backend <backend> <task-dir>` runs a generated pack.

- `print` (default): emits a copy/paste-friendly prompt + context + verify block.
  Best for browser ChatGPT, Codex Web, or any agent without a local CLI.
- `copilot`: launches the GitHub Copilot CLI (`copilot`, falling back to
  `gh copilot --`). The legacy `python tools/ai_launch_copilot.py <unit>` command
  is a thin wrapper that builds a unit pack and then runs this backend.
- `codex` / `claude` / `aider` / `gemini`: best-effort local CLIs. If the CLI is
  not on PATH, the prompt is printed instead of failing, so the run never crashes.

## Browser workflow

1. `python tools/ai_task.py unit <file>`
2. `python tools/ai_run.py --backend print .ai/tasks/<task>`
3. Paste the combined prompt/context into the browser agent.
4. Apply the suggested edits locally.
5. Verify with the task's `verify.sh` / `verify.ps1`.

## Optional MCP access

`python tools/mcp_decomp.py serve` exposes the read-only repo-index tools
(`lookup_symbol`, `lookup_unit`, `context`, `match_plan`, `decompme_inspect`)
over a stdlib JSON-RPC stdio MCP server for clients that prefer tool calls over
task packs.

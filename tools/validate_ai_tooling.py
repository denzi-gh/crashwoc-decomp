#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
REQUIRED_FILES = [
    "AGENTS.md",
    "tools/ai_common.py",
    "tools/ai_lookup_symbol.py",
    "tools/ai_lookup_unit.py",
    "tools/ai_context.py",
]
REQUIRED_SKILLS = [
    "project-onboarding",
    "symbol-lookup",
    "split-maintenance",
    "build-triage",
    "match-workflow",
]
MARKDOWN_LINK_RE = re.compile(r"\[[^\]]+\]\(([^)]+)\)")


def parse_frontmatter(path: Path) -> tuple[dict[str, str], str]:
    text = path.read_text(encoding="utf-8")
    if not text.startswith("---\n"):
        raise ValueError("missing YAML frontmatter")

    end = text.find("\n---\n", 4)
    if end == -1:
        raise ValueError("frontmatter terminator not found")

    frontmatter: dict[str, str] = {}
    for line in text[4:end].splitlines():
        if not line.strip():
            continue
        if ":" not in line:
            raise ValueError(f"invalid frontmatter line: {line}")
        key, value = line.split(":", 1)
        frontmatter[key.strip()] = value.strip().strip('"')
    return frontmatter, text


def check_markdown_links(path: Path, errors: list[str]) -> None:
    text = path.read_text(encoding="utf-8")
    for target in MARKDOWN_LINK_RE.findall(text):
        if "://" in target or target.startswith("#"):
            continue
        if target.startswith("mailto:"):
            continue
        target_path = (path.parent / target).resolve()
        if not target_path.exists():
            errors.append(f"{path.relative_to(ROOT).as_posix()}: missing link target {target}")


def validate_openai_yaml(path: Path, skill_name: str, errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"{path.relative_to(ROOT).as_posix()}: missing openai metadata")
        return

    text = path.read_text(encoding="utf-8")
    required_keys = ["interface:", "display_name:", "short_description:", "default_prompt:"]
    for key in required_keys:
        if key not in text:
            errors.append(f"{path.relative_to(ROOT).as_posix()}: missing {key}")
    prompt_match = re.search(r'default_prompt:\s*"([^"]+)"', text)
    if prompt_match is None:
        errors.append(f"{path.relative_to(ROOT).as_posix()}: default_prompt must be a quoted string")
        return
    if f"${skill_name}" not in prompt_match.group(1):
        errors.append(
            f"{path.relative_to(ROOT).as_posix()}: default_prompt must mention ${skill_name}"
        )


def main() -> int:
    errors: list[str] = []

    for rel_path in REQUIRED_FILES:
        path = ROOT / rel_path
        if not path.is_file():
            errors.append(f"Missing required file: {rel_path}")

    agents_path = ROOT / "AGENTS.md"
    if agents_path.is_file():
        check_markdown_links(agents_path, errors)

    for skill_name in REQUIRED_SKILLS:
        skill_dir = ROOT / "tools" / "skills" / skill_name
        skill_md = skill_dir / "SKILL.md"
        if not skill_md.is_file():
            errors.append(f"Missing skill file: {skill_md.relative_to(ROOT).as_posix()}")
            continue

        try:
            frontmatter, _ = parse_frontmatter(skill_md)
        except ValueError as exc:
            errors.append(f"{skill_md.relative_to(ROOT).as_posix()}: {exc}")
            continue

        name = frontmatter.get("name")
        description = frontmatter.get("description")
        if name != skill_name:
            errors.append(
                f"{skill_md.relative_to(ROOT).as_posix()}: expected name '{skill_name}', found '{name}'"
            )
        if not description:
            errors.append(f"{skill_md.relative_to(ROOT).as_posix()}: missing description")
        elif description.startswith("[TODO"):
            errors.append(f"{skill_md.relative_to(ROOT).as_posix()}: unresolved TODO description")

        check_markdown_links(skill_md, errors)
        validate_openai_yaml(skill_dir / "agents" / "openai.yaml", skill_name, errors)

    if errors:
        for error in errors:
            print(f"[ERROR] {error}")
        return 1

    print("AI tooling validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

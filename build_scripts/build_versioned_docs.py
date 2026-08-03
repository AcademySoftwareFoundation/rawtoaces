#!/usr/bin/env python3

from __future__ import annotations

import argparse
import html
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path


RELEASE_TAG_EXCLUSIONS = ("RC", "BETA")


def run(command: list[str], cwd: Path, env: dict[str, str] | None = None) -> None:
    subprocess.run(command, cwd=cwd, env=env, check=True)


def capture(command: list[str], cwd: Path) -> str:
    return subprocess.check_output(command, cwd=cwd, text=True).strip()


def ref_exists(repo_root: Path, ref: str) -> bool:
    return subprocess.run(
        ["git", "rev-parse", "--verify", "--quiet", ref],
        cwd=repo_root,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0


def ref_has_docs(repo_root: Path, ref: str) -> bool:
    return subprocess.run(
        ["git", "cat-file", "-e", f"{ref}:src/docs/conf.py"],
        cwd=repo_root,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0


def release_display_name(tag: str) -> str:
    if re.match(r"^v\d", tag):
        return tag[1:]
    return tag


def should_publish_tag(tag: str) -> bool:
    upper = tag.upper()
    return all(exclusion not in upper for exclusion in RELEASE_TAG_EXCLUSIONS)


def release_tags(repo_root: Path) -> list[str]:
    tags = capture(["git", "tag", "--sort=-v:refname"], cwd=repo_root).splitlines()
    return [tag for tag in tags if tag and should_publish_tag(tag) and ref_has_docs(repo_root, tag)]


def build_docs(worktree: Path, output_dir: Path, release_name: str | None) -> None:
    docs_dir = worktree / "src" / "docs"
    if not docs_dir.exists():
        raise FileNotFoundError(f"Documentation sources not found in {worktree}")

    output_dir.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()

    src_dir = worktree / "src"
    run(["doxygen", "docs/Doxyfile"], cwd=src_dir, env=env)

    sphinx_command = [
        "sphinx-build",
        "-b",
        "html",
        "-D",
        "breathe_projects.rawtoaces=../doxygen/xml",
    ]

    if release_name is not None:
        sphinx_command.extend(["-D", f"version={release_name}", "-D", f"release={release_name}"])

    sphinx_command.extend([".", str(output_dir)])
    run(sphinx_command, cwd=docs_dir, env=env)


def write_versions_index(output_root: Path, published_versions: list[tuple[str, str]]) -> None:
    versions_dir = output_root / "versions"
    versions_dir.mkdir(parents=True, exist_ok=True)
    versions_index = versions_dir / "index.html"

    items = [
        '<li><a href="../">latest (main)</a></li>',
        *[
            f'<li><a href="{html.escape(path)}/">{html.escape(label)}</a></li>'
            for path, label in published_versions
        ],
    ]

    versions_index.write_text(
        "\n".join(
            [
                "<!DOCTYPE html>",
                '<html lang="en">',
                "<head>",
                '  <meta charset="utf-8">',
                "  <title>rawtoaces documentation versions</title>",
                "</head>",
                "<body>",
                "  <h1>rawtoaces documentation versions</h1>",
                "  <ul>",
                *[f"    {item}" for item in items],
                "  </ul>",
                "</body>",
                "</html>",
            ]
        )
        + "\n",
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Build rawtoaces documentation for main and release tags.")
    parser.add_argument("--repo-root", default=".", help="Path to the repository root.")
    parser.add_argument("--output-dir", default="site", help="Directory where the built site will be written.")
    parser.add_argument(
        "--default-ref",
        default="origin/main",
        help="Git ref to build as the latest docs at the site root.",
    )
    parser.add_argument(
        "--list-only",
        action="store_true",
        help="Print the refs that would be built without running the documentation tools.",
    )
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    output_root = Path(args.output_dir).resolve()
    default_ref = args.default_ref

    if not ref_exists(repo_root, default_ref):
        if ref_exists(repo_root, "main"):
            default_ref = "main"
        else:
            raise ValueError(f"Could not resolve default ref '{args.default_ref}' or fallback 'main'.")

    published_tags = release_tags(repo_root)

    if args.list_only:
        print(f"latest={default_ref}")
        for tag in published_tags:
            print(f"release={tag}")
        return 0

    if output_root.exists():
        shutil.rmtree(output_root)
    output_root.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="rawtoaces-docs-") as temp_dir_name:
        temp_dir = Path(temp_dir_name)

        latest_worktree = temp_dir / "latest"
        run(["git", "worktree", "add", "--detach", str(latest_worktree), default_ref], cwd=repo_root)
        try:
            build_docs(latest_worktree, output_root, None)
        finally:
            run(["git", "worktree", "remove", "--force", str(latest_worktree)], cwd=repo_root)

        published_versions: list[tuple[str, str]] = []
        for tag in published_tags:
            tag_worktree = temp_dir / tag
            output_dir = output_root / "versions" / tag
            run(["git", "worktree", "add", "--detach", str(tag_worktree), tag], cwd=repo_root)
            try:
                build_docs(tag_worktree, output_dir, release_display_name(tag))
                published_versions.append((tag, tag))
            finally:
                run(["git", "worktree", "remove", "--force", str(tag_worktree)], cwd=repo_root)

    write_versions_index(output_root, published_versions)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

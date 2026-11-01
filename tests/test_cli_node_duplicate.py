#!/usr/bin/env python3
"""CLI integration test against a disposable copy of CherryTree's existing CTB fixture."""

import os
from pathlib import Path
import re
import shutil
import sqlite3
import subprocess
import sys
import tempfile


def check(condition, message):
    if not condition:
        raise AssertionError(message)


def main(executable, fixture):
    with tempfile.TemporaryDirectory(prefix="cherrytree-cli-test-") as temp:
        temp = Path(temp)
        document = temp / "test.ctb"
        shutil.copyfile(fixture, document)

        # Isolate user configuration; CTest runs this script under dbus-run-session.
        env = os.environ.copy()
        env["XDG_CONFIG_HOME"] = str(temp / "config")
        env["XDG_CACHE_HOME"] = str(temp / "cache")

        def run(*args):
            return subprocess.run(
                [str(executable), "-S", *map(str, args)],
                capture_output=True, text=True, env=env, timeout=60,
                check=False,
            )

        with sqlite3.connect(document) as db:
            row = db.execute("""
                SELECT n.node_id, n.txt, n.syntax
                FROM node n JOIN children c ON c.node_id = n.node_id
                WHERE c.master_id = 0
                ORDER BY (n.is_richtxt & 1), n.node_id LIMIT 1
            """).fetchone()
            check(row is not None, "Fixture contains no ordinary node")
            source_id, original_text, original_syntax = row
            initial_count = db.execute("SELECT COUNT(*) FROM node").fetchone()[0]

        invalid = run("--duplicate")
        check(invalid.returncode == 2, "Missing --node-id must exit with status 2")
        check(not invalid.stdout, "Invalid command produced stdout")

        not_found = run(document, "--node-id", source_id + 1000000, "--duplicate")
        check(not_found.returncode == 1, "Unknown node ID must exit with status 1")
        check(not not_found.stdout, "Unknown node ID produced stdout")

        created = run(document, "--node-id", source_id, "--duplicate", "--name", "CLI integration test")
        check(created.returncode == 0, f"Duplicate failed: {created.stderr.strip()}")
        check(re.fullmatch(r"[1-9][0-9]*\n", created.stdout) is not None,
              f"Expected only a numeric ID on stdout; got {created.stdout!r}")
        new_id = int(created.stdout.strip())
        check(new_id != source_id, "Duplicate returned the original node ID")

        with sqlite3.connect(document) as db:
            new_row = db.execute("SELECT name, txt, syntax FROM node WHERE node_id = ?", (new_id,)).fetchone()
            check(new_row is not None, "Duplicate was not persisted")
            check(new_row == ("CLI integration test", original_text, original_syntax),
                  "Duplicate name, text or syntax differs")
            check(db.execute("SELECT COUNT(*) FROM node").fetchone()[0] == initial_count + 1,
                  "Unexpected change in node count")
            check(db.execute("PRAGMA quick_check").fetchone()[0] == "ok",
                  "SQLite integrity check failed")

    print("CLI node duplication: OK")


def run_in_private_bus(dbus_runner, executable, fixture):
    # D-Bus activated desktop services can outlive the test and inherit CTest's
    # stdout/stderr pipes. Redirect them to a file so CTest can finish promptly.
    with tempfile.TemporaryFile(mode="w+t", encoding="utf-8") as log:
        try:
            result = subprocess.run(
                [str(dbus_runner), "--", sys.executable, str(Path(__file__).resolve()),
                 "--inside-session", str(executable), str(fixture)],
                stdout=log, stderr=subprocess.STDOUT, timeout=75, check=False,
            )
            return result.returncode
        finally:
            log.seek(0)
            sys.stdout.write(log.read())


if __name__ == "__main__":
    try:
        if len(sys.argv) == 4 and sys.argv[1] == "--inside-session":
            main(Path(sys.argv[2]).resolve(), Path(sys.argv[3]).resolve())
        elif len(sys.argv) == 4:
            sys.exit(run_in_private_bus(sys.argv[1], sys.argv[2], sys.argv[3]))
        else:
            sys.exit("Usage: test_cli_node_duplicate.py DBUS_RUN_SESSION CHERRYTREE_EXECUTABLE TEST_FIXTURE.ctb")
    except (AssertionError, OSError, sqlite3.Error, subprocess.TimeoutExpired) as exc:
        sys.exit(f"CLI node duplication test failed: {exc}")

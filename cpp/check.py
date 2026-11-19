"""Independent exact-integer checks; standard library only, no third-party packages."""
import argparse
import csv
import pathlib
import subprocess
import tempfile


def prime(n):
    if n < 2:
        return False
    d = 2
    while d * d <= n:
        if n % d == 0:
            return False
        d += 1
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=pathlib.Path)
    args = parser.parse_args()
    executable = str(args.executable.resolve())
    subprocess.run([executable, "--self-test"], check=True)
    # Keep subprocess output in the same workspace: some Windows hosts virtualize
    # TEMP differently for Python and native executables.
    with tempfile.TemporaryDirectory(prefix="weak-goldbach-check-", dir=pathlib.Path.cwd()) as temporary:
        path = pathlib.Path(temporary).resolve() / "witnesses.csv"
        run = subprocess.run(
            [executable, "--limit", "10001", "--mode", "scalar", "--cross-check", "--csv", str(path)],
            check=True, text=True, capture_output=True,
        )
        rows = list(csv.DictReader(path.open(encoding="utf-8", newline="")))
        assert len(rows) == 4998, "wrong interval coverage"
        for expected, row in zip(range(7, 10002, 2), rows):
            n, a, b, c = (int(row[name]) for name in ("n", "a", "b", "c"))
            assert n == expected and n == a + b + c and a == 3
            assert all(prime(p) for p in (a, b, c)), (n, a, b, c)
        print("python_independent_csv_checks=4998 PASS")
        print(run.stdout, end="")
    bad_inputs = [
        ["--limit", "0"], ["--limit", "6"], ["--limit", "-1"],
        ["--limit", "10000001"], ["--limit", "4294967296"],
        ["--limit", "1e6"], ["--limit", "7x"], ["--limit"],
        ["--mode", "neon"], ["--nonsense"],
    ]
    for flags in bad_inputs:
        run = subprocess.run([executable, *flags], text=True, capture_output=True)
        assert run.returncode == 1, (flags, run.returncode)
        assert "error:" in run.stderr
    print("invalid_input_checks=10 PASS")
    for limit, count in [(7, 1), (8, 1), (9, 2), (10, 2), (10000, 4997)]:
        run = subprocess.run([executable, "--limit", str(limit)], check=True, text=True, capture_output=True)
        assert f"checked_odds={count}\n" in run.stdout
        assert "result=PASS" in run.stdout
    print("boundary_cli_checks=5 PASS")


if __name__ == "__main__":
    main()

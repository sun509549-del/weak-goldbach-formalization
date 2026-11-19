"""Reject proof placeholders or extra axioms in this project's Lean sources."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
files = [root / 'WeakGoldbach.lean', *sorted((root / 'WeakGoldbach').glob('*.lean'))]
bad = []
for path in files:
    # Conservative gate: even a forbidden token in a comment requires review.
    for line_no, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
        if re.search(r'\b(sorry|admit|axiom|native_decide|unsafe)\b', line):
            bad.append(f'{path.relative_to(root)}:{line_no}: {line.strip()}')
if bad:
    raise SystemExit('\n'.join(bad))
print(f'PASS: {len(files)} Lean source files contain no placeholders, extra axioms, or native proof shortcuts.')

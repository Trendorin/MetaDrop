# Contributing to MetaDrop

Contributions should improve correctness, verifiability, accessibility, or Linux integration without widening claims beyond what tests can prove.

## Before coding

1. Search existing issues and discussions.
2. Open an issue before adding a dependency, format family, destructive mode, network capability, or persistent file history.
3. Never attach a real personal file. Create a synthetic fixture with obviously fake metadata.

## Development setup

Install the dependencies listed in the README, then run:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Before opening a pull request, also run the hardened configuration:

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

Format C++ changes with the repository `.clang-format`. New warnings are treated as defects even when a compiler does not promote them to errors.

## Backend requirements

A new cleaner must provide all of the following:

- content-aware format recognition, not suffix-only success;
- inspection of every field the cleaner promises to remove;
- non-destructive output to a staging path;
- a deterministic post-clean inspection result;
- synthetic fixtures covering sensitive, empty, malformed, and oversized inputs;
- a precise README boundary describing content that is not removed.

If a backend cannot verify a format, report it as unsupported. Do not trade a reliable refusal for a false clean result.

## Pull request checklist

- [ ] The source file is never modified by the change.
- [ ] Tests cover success and failure paths.
- [ ] No personal data, secrets, build products, or generated user settings are committed.
- [ ] User-visible behavior and format boundaries are documented.
- [ ] English UI text is clear; Russian translation is updated when applicable.
- [ ] `ctest --preset asan` passes.

By contributing, you agree that your work is licensed under GPL-3.0-or-later and that project participation follows the [Code of Conduct](CODE_OF_CONDUCT.md).

# Project Context (Server Health Monitor)

## Mission
Server Health Monitor is a production-focused Linux CLI written in C that reports CPU and RAM usage from `/proc`. It supports interactive and non-interactive modes and is designed for safe, defensive defaults.

## Key Commands

### Build
```bash
cmake -S . -B build
cmake --build build
```

### Test
```bash
ctest --test-dir build
```

## Agentic Workflow Guidance
- Keep changes minimal, testable, and consistent with existing C11 conventions.
- Prefer explicit error handling and input validation.
- Document any new workflow or tooling in README.md.

## Release Notes Expectations
- Describe user-facing changes succinctly.
- Call out new configuration or deployment steps explicitly.

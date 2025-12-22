# Server Health Monitor

Server Health Monitor is a small, production-focused C CLI that reports Linux CPU and RAM usage from `/proc`. It ships as a terminal menu monitor and supports a non-interactive mode for scripts or cron jobs.

## Scope

- ✅ **Current**: CPU/RAM usage reporting via a CLI (interactive menu or non-interactive mode).
- 🚧 **Future**: OS research ideas are listed in the [Roadmap](#roadmap) and are not part of the delivered software.

## Features

- Real CPU + memory usage sampling from `/proc`.
- Interactive menu with clear status output.
- Non-interactive mode for automation.
- Configurable interval/duration via flags or environment.
- Defensive defaults, input validation, and structured errors.

## Prerequisites

- Linux (uses `/proc/stat` and `/proc/meminfo`).
- CMake 3.16+
- GCC/Clang with C11 support.

## Build

```bash
cmake -S . -B build
cmake --build build
```

### Sanitizers (ASan/UBSan)

```bash
cmake -S . -B build -DENABLE_SANITIZERS=ON
cmake --build build
```

### Static analysis (cppcheck)

```bash
cmake -S . -B build -DENABLE_CPPCHECK=ON
cmake --build build
```

## Run

### Interactive menu

```bash
./build/server_monitor
```

### Non-interactive mode

```bash
./build/server_monitor --non-interactive --interval-ms 1000 --duration-ms 60000
```

### Run a fixed number of samples

```bash
./build/server_monitor --non-interactive --iterations 3 --interval-ms 2000
```

### Environment configuration

```bash
export SHM_SERVER_NAME=prod-01
export SHM_INTERVAL_MS=2000
export SHM_DURATION_MS=120000
./build/server_monitor
```

### Help

```bash
./build/server_monitor --help
```

## Examples

```text
Server Health Monitor
GitHub: https://github.com/kvnbbg
[INFO] Running in non-interactive mode.
Server Health Report for: prod-01
CPU Usage: 12.84%
RAM Usage: 45.72% (7.30 GB / 15.96 GB)
----------------------------------
Health monitoring completed for server: prod-01
```

## Tests

```bash
ctest --test-dir build
```

## Troubleshooting

- **"Failed to read CPU usage"**: Ensure `/proc/stat` is readable. This tool requires Linux.
- **"Failed to read memory usage"**: Ensure `/proc/meminfo` is readable.
- **Build warnings as errors**: Disable with `-DENABLE_WERROR=OFF` if needed.

## Security Notes

- No network access is performed.
- Input parsing is strict to avoid undefined behavior.
- Prefer running with least privilege; the tool only needs read access to `/proc`.

## Roadmap

These ideas are exploratory and **not** part of the current CLI deliverable:

- OS/kernel experimentation in C++.
- VM or hardware bootstrapping prototypes.
- Additional system telemetry modules.

## License

MIT. See [LICENSE](LICENSE).

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

CMake (v3.20+) with Ninja. The build directory is `b/`.

```bash
# Configure (first time or after CMakeLists changes)
cmake -S . -B b -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja

# Build
cmake --build b

# Build and run all tests
cmake --build b --target run_all_tests
```

## Running Tests

Tests use the [Unity](https://github.com/ThrowTheSwitch/Unity) framework. Test executables are built into `b/`.

```bash
# Run individual test suites
./b/test_project
./b/test_container
./b/test_selfma

# Run all tests via CMake
cmake --build b --target run_all_tests
```

Test sources are in `test/` and their Unity runner entry points are in `test/runner/trun_*.c`.

## Architecture

Selfma is a C++17 task management library with a layered architecture:

```
selfma_api.h (C API, opaque selfma_ctx_t)
    └── selfma.h/.cpp (C++ wrapper, event/callback system)
            └── Container (manages Projects via AVL tree + IDs pool)
                    └── Project (manages Tasks via AVL tree)
                            └── Task (data + timer)
```

**Key components:**

- **`core/inc/selfma_api.h`** — The public C API. Callers use an opaque `selfma_ctx_t`. All CRUD operations and serialization live here.
- **`core/inc/selfma.h`** / `core/selfma.cpp` — C++ wrapper. Owns the `Container`, manages a callback/event system (`NotifyCode` enum: `EVENT_MAX_TIME_SLEEP`, `TASK_TIME_ELAPSED`).
- **`core/inc/container.h`** / `core/container.cpp` — Top-level manager. Stores `Project` instances in an AVL tree keyed by ID. Delegates task operations to the relevant `Project`. Owns an `IdsPool`.
- **`core/inc/project.h`** / `core/project.cpp` — Stores `Task` instances in an AVL tree. Has a `worker()` for threaded update callbacks and a `clean()` for RAII teardown.
- **`core/inc/task.h`** — POD-like struct with a built-in timer. Packed for binary serialization (`MAX_NAME_LENGTH=256`, `MAX_DESCRIPTION_LENGTH=1024`).
- **`core/inc/selfma_file.h`** / `core/selfma_file.cpp` — Binary persistence layer. Files have a header with magic bytes, version, and CRC. Uses `FileGuard`/`HeaderGuard` RAII wrappers. Storage path is set at CMake configure time via `STORAGE_PATH`.
- **`core/inc/error_handler.h`** — `Result<T>` / `VoidResult` return types with `Ok()` / `Err()` helpers. `ErrorHandler` uses callbacks. `DisasterRecoveryPlan` registers recovery actions.
- **`core/inc/ids_pool.h`** — Stack-backed integer ID allocator. Supports `next()`, `release()`, and `available_ids()`.

**External dependencies (git submodules in `extern/`):**

- `extern/stuff` — Custom utilities: `qwistys_alloc` (custom allocator), `qwistys_avltree` (AVL tree), `qwistys_stack`, `qwistys_flexa` (flexible array), `qwistys_macros` (logging/debug macros).
- `extern/Unity` — C unit testing framework.

## Conventions

- Return errors via `Result<T>` / `VoidResult` (never throw). Use `Ok()` and `Err()` at call sites.
- Use `qwistys_macros.h` logging macros (`LOG_INFO`, `LOG_ERR`, etc.) rather than raw `printf`/`std::cout`.
- Struct packing for serializable types uses `GNU_PACKED_ATTRIBUTE` / `MSVC_PRAGMA_PACK` macros (cross-platform support including Windows via `clang-windows.cmake`).
- IDs are `uint32_t`; always acquire via `IdsPool::next()` and release via `IdsPool::release()` on removal.

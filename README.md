# Hotel parking management system

This C++17 application supports validated vehicle CRUD, counts, duration and fee queries, role sessions, and local persistence.

## Build and run

Install a C++17 compiler (GCC, Clang or an equivalent). From the repository root in PowerShell, using GCC:

```powershell
New-Item -ItemType Directory -Force build
g++ -std=c++17 -Wall -Wextra -pedantic -I. src/*.cpp -o build/parking.exe
./build/parking.exe
```

Select `1` for ordinary user, `2` for administrator, or `0` to exit. In a session select `0` to log out. End-of-file exits safely.

## Tests

The independent runner uses only the standard library. From the repository root:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. tests/test_main.cpp src/menus.cpp src/ordinary_user.cpp src/admin_user.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/tests.exe
./build/tests.exe
```

`vehicle` validates fields and local minute-resolution timestamps; `parking_service` keeps records sorted by plate and enforces admin-only changes. It reports distinct results for invalid input, duplicates, missing plates and permission failures. Fields trim only outer ASCII spaces and reject ASCII controls everywhere. `fee_calculator` normalizes both timestamps to local minutes and charges 20 MOP per started hour, capped at 400 MOP per successive 24-hour block (24h01m costs 420 MOP). Negative elapsed time is rejected. `ordinary_user` and `admin_user` contain separate session functions; `menus` routes sessions. `storage` handles local loading, saving and recovery.

## Local persistence

On startup the program loads `parking_data.txt` in the current working directory. The UTF-8 file contains a `# PARKING_SYSTEM_V1` header and one tab-separated record per line: plate, category, color, and local entry time (`YYYY-MM-DD HH:MM`). Blank lines and comments are ignored. Records are saved in plate order. The storage API accepts a custom path for testing.

A missing file starts an empty lot. Invalid rows or duplicate plates report their line and prevent saving; preserve and repair the file before restarting. A missing target with a valid `.bak` is recovered on startup; an invalid or inaccessible backup is preserved and blocks saving. Saves write and close a `.tmp` file before moving the previous file to `.bak` and replacing it; failed replacement attempts restore the backup. This multi-step protocol is recoverable, not an atomic transaction. Check disk permissions and retain any `.bak` if recovery fails.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. tests/test_storage.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_storage.exe
./build/test_storage.exe
```


## Console workflows

The role selector opens ordinary and administrator sessions. Both can list vehicles, find by plate and view the current count; list and lookup include duration and the fee. Administrators can add, replace or delete records. Each successful change is saved immediately; a failed save is reported as an in-memory change that has not been saved. Logout returns to role selection. Input ends cleanly on EOF. Entry timestamps use local `YYYY-MM-DD HH:MM`.

To run the menu integration tests after creating `build`:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. tests/test_menus.cpp src/menus.cpp src/ordinary_user.cpp src/admin_user.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_menus.exe
./build/test_menus.exe
```



The multi-process acceptance test runs against `build/parking.exe` in a new isolated directory under `build` and retains its test data there for inspection:

```powershell
./tests/test_e2e.ps1
```

## Google C++ Style compliance and documented exception

First-party C++ sources (`src/*.h`, `src/*.cpp`) and C++ test runners (`tests/*.cpp`) follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) (checked on `2026-11-02`), subject to the fixed project `C++17` language constraint and one explicitly approved project-level style exception:

- **Compliance status**: Google C++ Style with the documented `<filesystem>` exception (not unconditional zero-exception compliance).
- **Approved `<filesystem>` exception (Option A, approved on `2026-11-02`)**: Standard `<filesystem>` (`std::filesystem`) is retained alongside the fixed `C++17` language requirement to preserve local persistence, `.bak` recovery, parent-directory creation, and error-reporting semantics without custom OS-specific filesystem wrappers. The four affected files are `src/storage.h` (`L16`), `src/storage.cpp` (`L17`), `tests/test_menus.cpp` (`L13`), and `tests/test_storage.cpp` (`L14`).
- **Fixed tool versions**:
  - Compiler driver: `g++` (`clang version 23.1.1`, `x86_64-w64-windows-gnu`)
  - Formatter: `clang-format version 23.1.1` (using repository `.clang-format`: `BasedOnStyle: Google`)
  - Style linter: `cpplint 2.0.2` (run unfiltered without `NOLINT` or suppression config)
- **Reproducible check commands**:

```powershell
clang-format --dry-run --Werror (Get-ChildItem src/*.h, src/*.cpp, tests/*.cpp)
g++ -std=c++17 -Wall -Wextra -pedantic -I. -x c++-header -fsyntax-only src/*.h
uvx --from cpplint==2.0.2 cpplint --recursive src tests
```

- **Raw unfiltered `cpplint` results (22 total notices, 0 other rule violations)**:
  - `4` `[build/c++17]` notices (`<filesystem> is an unapproved C++17 header`) in `src/storage.h:16`, `src/storage.cpp:17`, `tests/test_menus.cpp:13`, and `tests/test_storage.cpp:14` — retained under the approved Option A project-level exception.
  - `18` `[legal/copyright]` notices (`No copyright message found`) across the 18 C++ files — disclosed tool-policy mismatches, not style exceptions: each file carries the standard Apache 2.0 license boilerplate matching `LICENSE` ([File Comments](https://google.github.io/styleguide/cppguide.html#File_Comments)), while omitting a `Copyright [year] <owner>` line so as not to invent a copyright holder not named in `LICENSE`.
- **Verified testing limits**:
  - Clean C++17 compilation (`0` warnings) and the four verification suites (`build/tests.exe`, `build/test_storage.exe`, `build/test_menus.exe`, and `tests/test_e2e.ps1`) were verified on Windows (`x86_64-w64-windows-gnu`, UCRT) in the local timezone; POSIX (Linux/macOS) builds and daylight-saving-time (DST) transition edges were not executed on target hardware.
  - Persistence tests dynamically cover missing files on first run, `.bak` startup recovery, malformed lines, duplicate plates, invalid `.bak` preservation, and parent paths blocked by a regular file (`blocker / "data.txt"`). Mid-write stream/disk-full failures on `.tmp`, OS-level rename/rollback failures (`path_ -> .bak`, `.tmp -> path_`, `.bak -> path_`), and ACL/file-lock errors were verified by static comparison against the baseline rather than dynamic fault injection.

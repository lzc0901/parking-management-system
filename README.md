# Hotel parking management system

This C++17 application supports validated vehicle CRUD, counts, duration and fee queries, role sessions, and local persistence.

## Build and run

### Prebuilt Windows executable (`bin/parking.exe`)

A prebuilt 64-bit Windows console executable is available at [bin/parking.exe](bin/parking.exe), alongside [SHA256SUMS.txt](bin/SHA256SUMS.txt) and [BUILD_INFO.txt](bin/BUILD_INFO.txt). Open the executable link on GitHub and download the file, or clone the repository. From a writable repository root containing `bin/`, verify and run it with:

```powershell
Get-FileHash bin/parking.exe -Algorithm SHA256
Get-Content bin/SHA256SUMS.txt
./bin/parking.exe
```

- **Expected SHA-256**: `b7b54c3bc548dabc7f6d6fb62d8cf77ad24e0a67fde3b729a22344c753d51ed8` (matches `bin/SHA256SUMS.txt` and the `bin/parking.exe` bundled in the release ZIP).
- **Runtime requirements and compatibility boundaries**:
  - Built with `clang 23.1.1` (`LLVM-MinGW UCRT`, target `x86_64-w64-windows-gnu`, `-std=c++17 -Wall -Wextra -pedantic -static`) as a Windows x86-64 console executable (PE subsystem version `6.0`).
  - Requires **64-bit Windows (x64)** and the **Windows Universal C Runtime (UCRT)**. Only `KERNEL32.dll` and UCRT API set DLLs (`api-ms-win-crt-*-l1-1-0.dll`) are imported; no separate compiler runtime DLL is required (see `bin/BUILD_INFO.txt`).
  - Verified on the build machine (`Windows NT 10.0.26300.0` x64) across all six C++ test runners, `tests/e2e/test_e2e.ps1`, and an isolated multi-process E2E smoke test directly against `bin/parking.exe` (ordinary queries, administrator CRUD, fee output, clean exit, and restart persistence). Clean-OS installations and other Windows releases have not been tested; the PE subsystem version (`6.0`) is a header metadata field and does not by itself prove compatibility across unverified Windows versions.
- **Default data file and working directory**:
  - On startup, the program reads and writes `parking_data.txt` in the **current working directory** (the directory from which `parking.exe` is launched, not the directory containing the `.exe`).
  - Always launch `parking.exe` from a writable directory; during saves it creates temporary `parking_data.txt.tmp` and backup `parking_data.txt.bak` files in the same directory. Personal vehicle data and runtime `.tmp`/`.bak` files are ignored by Git and excluded from repository delivery.

### Build from source

Install a C++17 compiler (GCC, Clang or an equivalent). From the repository root in PowerShell, using GCC:

```powershell
New-Item -ItemType Directory -Force build
g++ -std=c++17 -Wall -Wextra -pedantic -I. src/*.cpp -o build/parking.exe
./build/parking.exe
```

Select `1` for ordinary user, `2` for administrator, or `0` to exit. In a session select `0` to log out. End-of-file exits safely.

## Project structure

```text
bin/                         Tracked Windows x64 executable (parking.exe), SHA256SUMS.txt and BUILD_INFO.txt
src/                         C++17 application sources (unchanged)
tests/unit/                  vehicle, time and fee runners
tests/integration/           parking service, menus and storage runners
tests/e2e/test_e2e.ps1        isolated multi-process acceptance test
docs/                        guides and development report (MD/HTML/PDF)
docs/diagrams/               Mermaid source and four SVG diagrams
docs/evidence/               historical review evidence
scripts/check_report_assets.py
scripts/build_release.py
scripts/archive/build_juli18_candidate.py
build/                       ignored build outputs
dist/                        ignored release ZIP and SHA256SUMS.txt
dist/archive/                original historical ZIPs
archive/                     ignored snapshots, patches, records and one-off scripts
```

The tool directory `.multica/`, videos in `docs/demo/` and the assignment in
`docs/references/` remain local and ignored. Historical snapshots and checksums
retain their original bytes and describe their original execution environment.

- [User guide](docs/juli11_user_guide_zh.md)
- [Development report](docs/juli12_development_report_zh.md) ([HTML](docs/juli12_development_report_zh.html), [PDF](docs/juli12_development_report_zh.pdf))
- [Demo script](docs/juli13_demo_script_and_checklist_zh.md)
- [Viewing guide](docs/juli14_viewing_guide_and_alignment_zh.md)

## Tests

From the repository root in PowerShell, build and run all six independent C++
standard-library runners, then the end-to-end test:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
$sources = Get-ChildItem src/*.cpp | Where-Object Name -ne 'main.cpp'
Get-ChildItem tests/unit/*.cpp, tests/integration/*.cpp | ForEach-Object {
    g++ -std=c++17 -Wall -Wextra -pedantic -I. $_.FullName $sources.FullName -o "build/$($_.BaseName).exe"
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $($_.Name)" }
    & "./build/$($_.BaseName).exe"
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $($_.Name)" }
}
./tests/e2e/test_e2e.ps1
```

Unit tests cover vehicle fields, local timestamps and minute normalization, and
12 fee boundaries. Integration tests cover permissions, CRUD, sorted queries,
menu sessions, persistence and recovery. E2E retains its isolated data under
`build/` for inspection. Build `build/parking.exe` before running E2E.

## Local persistence

On startup the program loads `parking_data.txt` in the current working directory. The UTF-8 file contains a `# PARKING_SYSTEM_V1` header and one tab-separated record per line: plate, category, color, and local entry time (`YYYY-MM-DD HH:MM`). Blank lines and comments are ignored. Records are saved in plate order. The storage API accepts a custom path for testing.

A missing file starts an empty lot. Invalid rows or duplicate plates report their line and prevent saving; preserve and repair the file before restarting. A missing target with a valid `.bak` is recovered on startup; an invalid or inaccessible backup is preserved and blocks saving. Saves write and close a `.tmp` file before moving the previous file to `.bak` and replacing it; failed replacement attempts restore the backup. This multi-step protocol is recoverable, not an atomic transaction. Check disk permissions and retain any `.bak` if recovery fails.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. tests/integration/test_storage.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_storage.exe
./build/test_storage.exe
```


## Console workflows

The role selector opens ordinary and administrator sessions. Both can list vehicles, find by plate and view the current count; list and lookup include duration and the fee. Administrators can add, replace or delete records. Each successful change is saved immediately; a failed save is reported as an in-memory change that has not been saved. Logout returns to role selection. Input ends cleanly on EOF. Entry timestamps use local `YYYY-MM-DD HH:MM`.

To run the menu integration tests after creating `build`:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. tests/integration/test_menus.cpp src/menus.cpp src/ordinary_user.cpp src/admin_user.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_menus.exe
./build/test_menus.exe
```



The multi-process acceptance test runs against `build/parking.exe` in a new isolated directory under `build` and retains its test data there for inspection:

```powershell
./tests/e2e/test_e2e.ps1
```

## Google C++ Style compliance and documented exception

First-party C++ sources (`src/*.h`, `src/*.cpp`) and C++ test runners (`tests/unit/*.cpp, tests/integration/*.cpp`) follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) (checked on `2026-11-02`), subject to the fixed project `C++17` language constraint and one explicitly approved project-level style exception:

- **Compliance status**: Google C++ Style with the documented `<filesystem>` exception (not unconditional zero-exception compliance).
- **Approved `<filesystem>` exception (Option A, approved on `2026-11-02`)**: Standard `<filesystem>` (`std::filesystem`) is retained alongside the fixed `C++17` language requirement to preserve local persistence, `.bak` recovery, parent-directory creation, and error-reporting semantics without custom OS-specific filesystem wrappers. The four affected files are `src/storage.h` (`L16`), `src/storage.cpp` (`L17`), `tests/integration/test_menus.cpp` (`L13`), and `tests/integration/test_storage.cpp` (`L14`).
- **Fixed tool versions**:
  - Compiler driver: `g++` (`clang version 23.1.1`, `x86_64-w64-windows-gnu`)
  - Formatter: `clang-format version 23.1.1` (using repository `.clang-format`: `BasedOnStyle: Google`)
  - Style linter: `cpplint 2.0.2` (run unfiltered without `NOLINT` or suppression config)
- **Reproducible check commands**:

```powershell
clang-format --dry-run --Werror (Get-ChildItem src/*.h, src/*.cpp, tests/unit/*.cpp, tests/integration/*.cpp)
g++ -std=c++17 -Wall -Wextra -pedantic -I. -x c++-header -fsyntax-only src/*.h
uvx --from cpplint==2.0.2 cpplint --recursive src tests
```

- **Historical unfiltered `cpplint` results before test splitting (22 total notices, 0 other rule violations)**:
  - `4` `[build/c++17]` notices (`<filesystem> is an unapproved C++17 header`) in `src/storage.h:16`, `src/storage.cpp:17`, `tests/integration/test_menus.cpp:13`, and `tests/integration/test_storage.cpp:14` — retained under the approved Option A project-level exception.
  - `18` `[legal/copyright]` notices (`No copyright message found`) across the original 18 C++ files before test splitting — disclosed tool-policy mismatches, not style exceptions: each file carries the standard Apache 2.0 license boilerplate matching `LICENSE` ([File Comments](https://google.github.io/styleguide/cppguide.html#File_Comments)), while omitting a `Copyright [year] <owner>` line so as not to invent a copyright holder not named in `LICENSE`.
- **Verified testing limits**:
  - Clean C++17 compilation (`0` warnings) and the four verification suites (`build/test_parking_service.exe`, `build/test_storage.exe`, `build/test_menus.exe`, and `tests/e2e/test_e2e.ps1`) were verified on Windows (`x86_64-w64-windows-gnu`, UCRT) in the local timezone; POSIX (Linux/macOS) builds and daylight-saving-time (DST) transition edges were not executed on target hardware.
  - Persistence tests dynamically cover missing files on first run, `.bak` startup recovery, malformed lines, duplicate plates, invalid `.bak` preservation, and parent paths blocked by a regular file (`blocker / "data.txt"`). Mid-write stream/disk-full failures on `.tmp`, OS-level rename/rollback failures (`path_ -> .bak`, `.tmp -> path_`, `.bak -> path_`), and ACL/file-lock errors were verified by static comparison against the baseline rather than dynamic fault injection.

## Release package

```powershell
python scripts/build_release.py
python scripts/check_report_assets.py docs
python scripts/check_report_assets.py dist/parking-management-system-v1.0.zip
Get-FileHash dist/parking-management-system-v1.0.zip -Algorithm SHA256
```

The release contains `src/`, `tests/`, maintained `docs/`, `bin/parking.exe`,
README, LICENSE and `.clang-format` under `parking-management-system-v1.0/`.
Historical evidence, video, assignment references, scripts and old archives are
excluded. The Windows executable uses static compiler runtime linking. For the
tracked release, the validated `build/parking.exe` was manually copied to
`bin/parking.exe`, matching the executable in that release ZIP. The script
does not update `bin/`; a subsequent build may produce different bytes.
`bin/SHA256SUMS.txt` and `bin/BUILD_INFO.txt` describe the tracked executable;
`dist/SHA256SUMS.txt` records the ZIP checksum.

`scripts/archive/build_juli18_candidate.py` reproduces a historical package,
requires original Git objects and local archives, and refuses to overwrite
archived outputs. It does not build the current release.

## Structure migration verification (2026-11-02)

The six C++ runners and E2E passed on Windows with C++17 and
`-Wall -Wextra -pedantic`. Application source hashes are unchanged.
The recursive formatting check passed with `clang-format 21.1.8`:

```powershell
uvx --from clang-format==21.1.8 clang-format --dry-run --Werror (Get-ChildItem src/*.h, src/*.cpp, tests/unit/*.cpp, tests/integration/*.cpp).FullName
```

Unfiltered `cpplint 2.0.2` reports 25 existing-policy notices after splitting:
21 missing copyright-owner lines and four approved `<filesystem>` notices;
there are no other rule violations. Historical tool versions above describe
previous verification, not this run.

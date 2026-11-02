> 历史验收记录：正文结论保留原执行时间。本次目录迁移仅调整路径；当前验证结果见项目 README。

# JULI-11 (D02) User Guide Verification & Isolated Walkthrough Evidence

## 1. Reviewed Application Baseline

- **Reviewed Snapshot**: `e06-source.zip` (SHA-256: `844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e`), formally approved in [JULI-10](mention://issue/01a1980f-535c-7574-8051-a505815a9bdd) (`01a1990c-ff23-78b4-a7ba-6f38269ac101`); subsequent Google C++ Style cleanup commit `df3a71477b3635c93c64305f2e941f0ccb5f88b2` (`refs/juli17/style`, approved in [JULI-17](mention://issue/01a19a63-9796-78ca-a8ed-d26785df7071)) preserves identical runtime behavior and console prompts.
- **Build Command Verified**:
  ```powershell
  New-Item -ItemType Directory -Force build
  g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc src/*.cpp -o build/parking.exe
  ```
- **Isolation**: All walkthrough runs below were executed in isolated temporary directories (`$env:TEMP\juli11-...`) so that the repository root, git index, checkpoints (`refs/checkpoints/e01`, `refs/checkpoints/e02`), and snapshot archives (`e03-snapshot.patch`, `e04-snapshot.patch`, `e05-source.zip`, `e06-source.zip`) remain 100% untouched.

---

## 2. Walkthrough Run 1: First Run, Empty Lot, Ordinary Session, Admin Validations, Add/Modify & Role Switch

### Input Sequence
```text
9
1
1
2
MA-12-34
3
4
0
2
4
MA 12
Sedan
White
2026-11-01 08:00
4
MA-12-34

White
2026-11-01 08:00
4
MA-12-34
Sedan
White
2026-02-29 08:00
4
MA-12-34
Sedan
White
2099-01-01 00:00
4
MA-12-34
轎車
黑色
2026-11-01 08:00
4
MB-56-78
SUV
白色
2026-11-02 04:00
4
MA-12-34
SUV
白色
2026-11-02 04:00
5
UNKNOWN
MA-12-34
SUV
白色
2026-11-02 04:00
5
MA-12-34
MB-56-78
轎車
銀色
2026-11-01 08:00
5
MA-12-34
MA-12-34
轎車
銀色
2026-11-01 08:00
0
1
1
2
MA-12-34
3
0
0
```

### Actual Console Output (`ExitCode: 0`)
```text
First run: no parking data file.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Invalid choice.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> No vehicles.
Current count: 0
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: Vehicle not found.
Current count: 0
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Current count: 0
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Invalid choice.
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Invalid required field.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Invalid required field.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Invalid entry time.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Entry time is in the future.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Plate already exists.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Vehicle not found.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Plate already exists.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
Plate: MB-56-78 | Category: SUV | Color: 白色 | Entry: 2026-11-02 04:00 | Duration: 0 days 1 hours 40 minutes | Fee: 40 MOP
Current count: 2
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
Current count: 2
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Current count: 2
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Parking data saved.
```

### Persisted `parking_data.txt` (UTF-8) After Run 1
```text
# PARKING_SYSTEM_V1
MA-12-34	轎車	銀色	2026-11-01 08:00
MB-56-78	SUV	白色	2026-11-02 04:00
```

---

## 3. Walkthrough Run 2: Process Restart Reload & Administrator Deletion

### Input Sequence
```text
2
1
6
UNKNOWN
6
MB-56-78
3
0
0
```

### Actual Console Output (`ExitCode: 0`)
```text
Loaded parking_data.txt.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
Plate: MB-56-78 | Category: SUV | Color: 白色 | Entry: 2026-11-02 04:00 | Duration: 0 days 1 hours 40 minutes | Fee: 40 MOP
Current count: 2
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Vehicle not found.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Current count: 1
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Parking data saved.
```

---

## 4. Walkthrough Run 3: Startup `.bak` Backup Recovery

### Setup & Input
Moved `parking_data.txt` to `parking_data.txt.bak` and launched `./build/parking.exe` with inputs `1`, `1`, `0`, `0`.

### Actual Console Output (`ExitCode: 0`)
```text
Recovered parking data from parking_data.txt.bak.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
Current count: 1
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Parking data saved.
```

---

## 5. Walkthrough Run 4: In-Memory-Only Warning on Failed Save & Corrupted File Rejection

### 5.1 Active-Session Save Failure (`Updated in memory, but save failed: ...`)
Created a directory at `parking_data.txt.tmp` so staging `.tmp` fails while `parking_data.txt` remains intact, then ran administrator `4 = add` followed by `1 = list`:

```text
Loaded parking_data.txt.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated in memory, but save failed: Cannot write temporary file parking_data.txt.tmp; check permissions.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: MA-12-34 | Category: 轎車 | Color: 黑色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
Plate: MB-56-78 | Category: SUV | Color: 白色 | Entry: 2026-11-02 04:00 | Duration: 0 days 1 hours 40 minutes | Fee: 40 MOP
Current count: 2
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Cannot write temporary file parking_data.txt.tmp; check permissions.
```
*(ExitCode: `1`; in-memory update was visible during the session while disk file `parking_data.txt` remained untouched.)*

### 5.2 Startup Malformed Line Rejection
Injected `bad_line` on line 3 of `parking_data.txt` and started `./build/parking.exe`:
```text
Line 3: expected four tab-separated fields.
```
*(ExitCode: `1`; program refused partial load and preserved the file.)*

---

## 6. Discrepancy Check

- **Discrepancies Found**: **None (`0` discrepancies)**. Every instruction, menu prompt, fee calculation, validation message, and recovery behavior in `juli11_user_guide_zh.md` matches the reviewed `E06` executable byte-for-byte. No Engineer fixes are required.

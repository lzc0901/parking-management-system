> 历史验收记录：正文结论保留原执行时间。本次目录迁移仅调整路径；当前验证结果见项目 README。

# JULI-14 video coverage checklist

Recording: `juli14-demo.mp4`, live E06 console capture, 1498 × 770, 10 fps, duration 02:57.6. No narration/audio. English sample category/color values (`Sedan`/`Black` -> `Gold`, `SUV`/`White`, `MPV`/`Silver`, `Sedan`/`Blue`, `Sports`/`Red`) were used in place of the script's Chinese examples; actions and validation sequence follow JULI-13 v2. Times below mark the start of each action group; results appear shortly afterward.

- 00:00 Initial empty lot (`First run: no parking data file.`), fee rule (`20 MOP per started hour, capped at 400 MOP per successive 24-hour block.`), role selector.
- 00:03 Ordinary user: empty list (`No vehicles.`, `Current count: 0`) and count (`0`), rejected admin menu choice (`4` -> `Invalid choice.`), logout (`0`).
- 00:15 Administrator: add `MA-10-01` (`Sedan`, `Black`, `2026-11-02 04:39`) -> `Updated and saved. Parking data saved.`.
- 00:30 Add `MB-20-02` (`SUV`, `White`, `2026-11-01 09:14`); 00:43 add `MC-30-03` (`MPV`, `Silver`, `2026-11-01 05:09`).
- 00:56 Duplicate plate rejection (`MA-10-01` -> `Plate already exists.`); 01:09 future entry rejection (`MD-40-04`, `2026-12-02 06:14` -> `Entry time is in the future.`).
- 01:22 Administrator list (`1`), lookup (`2` -> `MB-20-02`) and count (`3` -> `Current count: 3`), showing durations (`0d 1h 37m`, `0d 21h 2m`, `1d 1h 7m`) and fee tiers (`40 MOP`, `400 MOP`, `440 MOP`).
- 01:32 Modify `MA-10-01` (`5` -> `Existing plate: MA-10-01`, color changed from `Black` to `Gold`), query the updated record (`2` -> `MA-10-01`), and administrator logout (`0`).
- 01:54 Ordinary list (`1`), lookup hit (`2` -> `MA-10-01`, duration advances 1 min with live clock to `0d 1h 38m`, fee `40 MOP`) and miss (`2` -> `MZ-99-99` -> `Vehicle not found.`, `Current count: 3`), count (`3` -> `Current count: 3`) and logout (`0`).
- 02:15 Exit (`0` -> `Parking data saved.`) and restart `..\parking.exe`; 02:19 reload (`Loaded parking_data.txt.`), ordinary list (`1` showing all 3 persisted vehicles with `0d 1h 38m` / `40 MOP`, `0d 21h 3m` / `400 MOP`, `1d 1h 8m` / `440 MOP`), and logout (`0`).
- 02:28 Administrator deletes `MB-20-02` (`6` -> `Existing plate: MB-20-02` -> `Updated and saved. Parking data saved.`), checks count (`3` -> `Current count: 2`) and logs out (`0`).
- 02:41 Ordinary final list (`1` showing `MA-10-01` and `MC-30-03`, `Current count: 2`), count (`3` -> `Current count: 2`), logout (`0`) and exit (`0` -> `Parking data saved.`); saved file contains `MA-10-01` and `MC-30-03`.

Build baseline: E06 source archive SHA-256 `844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e` (subsequent JULI-17 Google C++ Style revision `df3a71477b3635c93c64305f2e941f0ccb5f88b2` preserves identical console prompts, fee tiers, and persistence behavior). The live clock advances displayed durations by 1 minute during the recording (`06:16` -> `06:17`) without changing the `40 MOP`, `400 MOP`, and `440 MOP` fee tiers. Long vehicle rows wrap at the right edge (`| Fee:` / `40 MOP`, `| Fee: 4` / `0 MOP`, `| Fee: 40` / `0 MOP`, `| Fee: 44` / `0 MOP`); all characters and fee values are verified readable across sampled frames. See `juli14_viewing_guide_and_alignment_zh.md` for the full Chinese viewing guide and script-alignment verification.

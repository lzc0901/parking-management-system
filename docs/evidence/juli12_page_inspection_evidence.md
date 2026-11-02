> 历史验收记录：正文结论保留原执行时间。本次目录迁移仅调整路径；当前验证结果见项目 README。

# JULI-12 (D01) & JULI-18 Chinese Development Report PDF, Markdown Image Link Repair & Page Inspection Evidence

## 1. Delivered Report & Diagram Artifacts

| Artifact File | Role / Description | Format & Verification |
| :--- | :--- | :--- |
| `juli12_development_report_zh.pdf` | Final exported Chinese development report and user instructions (`系統開發報告與使用說明書`) | 11-page A4 PDF rendered via headless Microsoft Edge (`msedge --headless=old --print-to-pdf`), visually inspected page-by-page at 150 DPI (`1240x1754` PNGs). |
| `juli12_development_report_zh.md` | Editable Markdown source of the complete Chinese development report and integrated `JULI-11` user guide | UTF-8 Markdown source covering Chapters 1–6 (`需求分析`, `架構與模塊劃分`, `流程圖與開發邏輯說明`, `測試結果與質量審計`, `局限性與優化討論`, `系統使用說明書`). Uses valid relative SVG paths (`./juli12_flowchart_1_overall.svg` through `./juli12_flowchart_4_storage.svg`). |
| `juli12_flowcharts.mmd` | Editable Mermaid flowchart definitions for all 4 system and module diagrams | Covers Overall System Flowchart (`圖 1`), Admin CRUD & Immediate Save (`圖 2`), Deterministic Duration & Fee Calculation (`圖 3`), and Local File Load/Recovery/Safe Save (`圖 4`). |
| `juli12_flowchart_1_overall.svg`<br>`juli12_flowchart_2_crud.svg`<br>`juli12_flowchart_3_fee.svg`<br>`juli12_flowchart_4_storage.svg` | Editable vector SVG diagrams referenced by `juli12_development_report_zh.md` and embedded into `juli12_development_report_zh.html` / `.pdf` | Vector diagrams verified against `src/*` in `e06-source.zip` (`844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e`) and `JULI-17` Google C++ Style commit `df3a71477b3635c93c64305f2e941f0ccb5f88b2`. All 4 SVGs pass `xml.etree.ElementTree` XML validation (`invalid_svg=0`). |

---

## 2. JULI-18 Errata & Remediation Record (2026-11-02)

1. **Errata on Previous Review Coverage (`JULI-16` / `JULI-1` comment `01a1993f-c3b5-7b8a-b916-e1196c04051e`)**:
   - In the original `juli15-review-package.zip` (`deea16db6b84944106c989c89d994b12f62deed4f1394f80c61b1e0870bf1364`), `docs/juli12_development_report_zh.md` contained 4 broken image references (`./flowchart_1_overall.png`, `./flowchart_2_crud.png`, `./flowchart_3_fee.png`, `./flowchart_4_storage.png` at lines 96, 112, 128, and 146), while the package actually delivered `docs/juli12_flowchart_1_overall.svg` through `docs/juli12_flowchart_4_storage.svg`.
   - Furthermore, the working-directory copies of `juli12_flowchart_2_crud.svg`, `juli12_flowchart_3_fee.svg`, and `juli12_flowchart_4_storage.svg` still had unescaped `&` on line 8 (`&` had only been escaped inside the packaged copies).
   - Because `juli12_development_report_zh.html` embedded inline `<svg>` blocks, the exported PDF displayed the 4 diagrams normally, and the previous `JULI-16` review checked PDF page renders, packaged SVG XML parsing, and 41/41 SHA-256 manifest hashes, but omitted checking relative Markdown image path existence and rendered Markdown preview. File hash integrity and PDF visual inspection do not substitute for Markdown relative link verification.
2. **Fixes Applied in `JULI-18` (Revision 3)**:
   - Updated the 4 Markdown image links in `juli12_development_report_zh.md` to exact relative SVG paths: `./juli12_flowchart_1_overall.svg`, `./juli12_flowchart_2_crud.svg`, `./juli12_flowchart_3_fee.svg`, and `./juli12_flowchart_4_storage.svg`.
   - Fixed the unescaped `&` (`&amp;`) in the working-directory SVG files (`juli12_flowchart_2_crud.svg`, `juli12_flowchart_3_fee.svg`, `juli12_flowchart_4_storage.svg`), ensuring working directory, HTML, PDF, and packaged copies share one identical, XML-valid source.
   - Aligned internal C++ symbol names and line references across `juli12_development_report_zh.md`, `juli12_flowcharts.mmd`, the 4 SVG diagrams, `juli12_development_report_zh.html`, and `juli12_development_report_zh.pdf` with the `JULI-17` Google C++ Style verified source (`df3a71477b3635c93c64305f2e941f0ccb5f88b2`, e.g., `AddVehicle`, `ModifyVehicle`, `DeleteVehicle`, `ListWithFees`, `FindWithFee`, `CalculateParkingFee`, `Storage::Load`, `Storage::Save`, `ParseFile`, `RunMenu`, `RunReadChoice`, `RunOrdinaryUserSession`, `RunAdminUserSession`, `Role::kAdmin`, `VehicleResult::kOk`, `entry_time_text`, `total_minutes`, and `-I. -Isrc` build compatibility).
   - Verified in a clean isolated temporary directory (`verify_md_preview.py`) that `python check_report_assets.py <dir>` exits `0` with `image_refs=4 missing=0 invalid_svg=0`, and visually verified the rendered Markdown `<img src="./juli12_flowchart_...svg">` preview alongside the re-exported 11-page PDF.

---

## 3. Iterative Page-by-Page Visual Inspection Log (Pages 1–11)

All 11 pages of `juli12_development_report_zh.pdf` were rendered to PNG (`150 DPI`, `1240x1754`) using `pymupdf` and visually inspected:

### Iteration 1 Findings & Fixes Applied (Original JULI-12 Render)
1. **Literal `<br>` tags inside Markdown tables (`Pages 2, 3, 7, 8`)**: Mistune escaped raw `<br>` inside table cells; replaced with clean `/` separators and natural wrapping.
2. **Literal `$O(\log N)$` / `$O(N)$` LaTeX math delimiters (`Pages 3, 8, 9`)**: Replaced with clean `O(log N)`, `O(N)`, and `O(1)` code/text notation.
3. **Unparsed `**...**` bold markers adjacent to full-width parentheses (`Pages 1, 6`)**: Normalized bold tags and added post-processing so all bold Chinese phrases render cleanly as `<strong>`.
4. **Diagram box/diamond text clipping in `圖 1`, `圖 2`, and `圖 3` (`Pages 4, 5, 6`)**: Widened the SVG viewBox to `910px`, widened the decision diamonds and CRUD/fee boxes, and repositioned branch labels so zero characters overlap borders or arrows.
5. **Horizontal scrollbar on long `<pre>` console output in Chapter 6 (`Page 10`)**: Added `white-space: pre-wrap; overflow-wrap: break-word; overflow-x: hidden;` and replaced `word-break: break-all` with `overflow-wrap: break-word` plus `th { white-space: nowrap; }` so tables and code blocks wrap at word boundaries without scrollbars or mid-word splits.

### Final Page-by-Page Inspection Summary (Revision 3 — All 11 Pages Verified PASS)
- **Page 1**: Title banner, metadata header (`e06-source.zip` SHA-256 and `JULI-17` Google C++ Style commit `df3a71477b3635c93c64305f2e941f0ccb5f88b2`), Chapter 1 (`項目背景與需求分析`), and `M01–M05` of the traceability table. All Chinese characters, code paths, and borders are sharp and properly aligned.
- **Page 2**: `M06–M10` of the traceability table, Section 1.3 (`已批准的設計決策與範圍邊界`), and Chapter 2 (`系統整體架構與模塊劃分`) table rows 1–2 (`src/main.cpp`, `src/menus.*`).
- **Page 3**: Remaining rows of the 15-file architecture table (`src/ordinary_user.*`, `src/admin_user.*`, `src/vehicle.*`, `src/parking_service.*`, `src/fee_calculator.*`, `src/storage.*`), Section 2.2 (`VehicleRecord`, `ParkingFee` struct definitions, and `std::map` complexity analysis), and Chapter 3 heading.
- **Page 4**: Section 3.1 logic explanation and **`圖 1：系統整體運行與角色會話路由流程圖 (Overall System Flowchart)`** — all nodes, decision diamonds, `0 = logout` loop arrows, and exit codes (`0` / `1`) are complete and legible on a single page without page-break splitting.
- **Page 5**: Section 3.2 logic explanation and **`圖 2：管理員增刪改（CRUD）與即時持久化流程圖 (CRUD & Persistence Flowchart)`** — complete diagram showing `Role::kAdmin` check, `Validated()` 3-layer checks, `std::map` uniqueness/collision checks, `storage.Save()`, and both `Updated and saved. Parking data saved.` and `Updated in memory, but save failed: <reason>`.
- **Page 6**: Section 3.3 logic explanation, **`圖 3：確定性停車時長與費用計算流程圖 (Deterministic Duration & Fee Flowchart)`**, and Section 3.4 text explanation (`Storage::Load` & `Storage::Save` multi-step save and rollback protocol, including `.tmp` staging, existing file verification, `.bak` rotation, stale `.bak` blocking, and rollback failure preservation).
- **Page 7**: **`圖 4：本地文件載入、備份恢復與安全保存流程圖 (Load, Recovery & Safe Save Flowchart)`** (side-by-side Subgraphs A and B, with Subgraph B explicitly detailing the multi-step save & rollback protocol) and Chapter 4 (`測試驗證結果與質量審計`) rows 1–2 (`tests.exe`, `test_storage.exe`).
- **Page 8**: Chapter 4 rows 3–4 (`test_menus.exe`, `tests/e2e/test_e2e.ps1`), verification boundary disclosure callout (`clang-format 23.1.1`, `cpplint 2.0.2`, Windows/UCRT vs unverified POSIX/DST and static vs dynamic low-level I/O fault injection), and Chapter 5 (`系統局限性與可優化的邏輯及代碼討論`) Section 5.1 (`O(N)` full-rewrite I/O, non-atomic multi-rename vs OS transactional replace, single-process file locking, and local DST boundary).
- **Page 9**: Section 5.2 (`可優化的代碼與邏輯方向`: Append-Only WAL/Compaction, removing redundant `ParkingService checked` copy during save, `std::unordered_map` secondary index, C++20 `<chrono>`), and Chapter 6 (`系統使用說明書`) Sections 6.1–6.2.
- **Page 10**: Chapter 6 Sections 6.2 (startup messages), 6.3 (`普通用戶操作步驟`, explicitly noting `Current count: N` is printed after every `2 = find` hit or miss), and 6.4 (`管理員操作步驟`), with wrapped console blocks and zero scrollbars.
- **Page 11**: Chapter 6 Section 6.5 (`停車費計算規則與邊界對照表`: `0m` to `48h`) and Section 6.6 (`字段校驗、本地文件格式與異常恢復速查表`, including `Updated in memory, but save failed: <reason>`, `Line N: ...`, and `Save blocked: ...`).

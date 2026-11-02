> 历史验收记录：正文结论保留原执行时间。本次目录迁移仅调整路径；当前验证结果见项目 README。

# JULI-13 完整演示腳本隔離環境排練實證報告（修訂版 v2 — 原始捕獲輸出）

**對應議題**：[JULI-13](mention://issue/01a1980f-6878-713b-9e89-a8be7590f771)（D03: Prepare and rehearse the complete demonstration script）
**下游錄製議題**：[JULI-14](mention://issue/01a1980f-6fa0-7a86-8493-7c59850c53b5)（D04: Capture and verify the required full-process screen recording）
**排練基準版本**：經 [JULI-10](mention://issue/01a1980f-535c-7574-8051-a505815a9bdd) 審查通過之 `E06` 產品代碼（Git `HEAD` = `2cfc1cfa78a6fa80ca91f4d210cbdf963d819c38`，`e06-source.zip` SHA-256 = `844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e`；後續 [JULI-17](mention://issue/01a19a63-9796-78ca-a8ed-d26785df7071) Google C++ Style 整理版 `df3a71477b3635c93c64305f2e941f0ccb5f88b2` 保持完全相同的控制台提示與執行行為）
**排練執行時間**：`2026-11-02 06:05`（本地系統時間）
**隔離排練目錄**：系統臨時隔離目錄 `$env:TEMP\juli13-rehearsal-utf8-<GUID>`（排練完成後已自動清理，未在倉庫目錄留下任何殘留文件）

> **更正與廢止聲明（Explicit Supersession Notice）**：
> 本報告（修訂版 v2）正式**取代並廢止**上一版本 `juli13_rehearsal_evidence.md`（附件 ID `01a19921-67df-72b5-968d-7d982ff515cf`）中不準確的排練輸出宣稱。上一版本因手動整理轉錄而誤入了早期草稿格式的提示詞（例如將頂層菜單寫作 `=== Hotel Parking Management System ===`、將字段提示寫作 `Type:` / `New type:` / `Plate to modify:` / `Plate to delete:`、將重複車牌與未來時間提示加上了不存在的 `Add failed: ` 前綴，以及遺漏了 `parking_data.txt` 首行的 `# PARKING_SYSTEM_V1` 標頭）。以下所有輸入與輸出均為在隔離目錄中直接重定向 `build/parking.exe` 捕獲的**未經任何手動篡改的原始二進制 UTF-8 輸出（Raw Captured Output）**。

---

## 一、排練編譯與動態時間戳基準設定

### 1.1 編譯指令與結果
```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc src/*.cpp -o build/parking.exe
# Exit Code: 0（零警告、零錯誤）
```

### 1.2 排練時刻（`2026-11-02 06:05`）生成之相對入場時間戳
| 變數名稱 | 實際輸入時間字串 | 相對當前時刻差值 | 實測系統計算時長 (`Duration`) | 實測系統計算費用 (`Fee`) |
| :--- | :--- | :--- | :--- | :--- |
| `$t95m` | `2026-11-02 04:30` | `-95` 分鐘 | `0 days 1 hours 35 minutes` | `40 MOP` |
| `$t21h` | `2026-11-01 09:05` | `-1260` 分鐘（21 小時） | `0 days 21 hours 0 minutes` | `400 MOP`（24h 週期封頂） |
| `$t25h` | `2026-11-01 05:00` | `-1505` 分鐘（25 小時 5 分） | `1 days 1 hours 5 minutes` | `440 MOP`（跨日計費） |
| `$tFuture` | `2026-12-02 06:05` | `+30` 天（未來時間） | —（系統攔截） | —（`Entry time is in the future.`） |

---

## 二、階段一（Process 1）原始標準輸入與真實捕獲輸出（Raw Captured Output）

### 2.1 階段一（Process 1）標準輸入序列（`p1_in.txt`）
*(註：第 35–40 行對應管理員 `5 = modify`，依次輸入 `5`、`Existing plate` = `MA-10-01`、`Plate` = `MA-10-01`、`Category` = `轎車`、`Color` = `香檳金`、`Entry time` = `2026-11-02 04:30`)*
```text
1
1
3
4
0
2
4
MA-10-01
轎車
黑色
2026-11-02 04:30
4
MB-20-02
SUV
白色
2026-11-01 09:05
4
MC-30-03
MPV
銀色
2026-11-01 05:00
4
MA-10-01
轎車
藍色
2026-11-02 04:30
4
MD-40-04
跑車
紅色
2026-12-02 06:05
1
2
MB-20-02
3
5
MA-10-01
MA-10-01
轎車
香檳金
2026-11-02 04:30
2
MA-10-01
0
1
1
2
MA-10-01
2
MZ-99-99
3
0
0
```

### 2.2 階段一（Process 1）原始捕獲控制台輸出（`p1_out.txt`，`ExitCode: 0`）
```text
First run: no parking data file.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> No vehicles.
Current count: 0
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Current count: 0
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Invalid choice.
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Plate already exists.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Entry time is in the future.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: MA-10-01 | Category: 轎車 | Color: 黑色 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
Current count: 3
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
Current count: 3
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Current count: 3
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Plate: Category: Color: Entry time (YYYY-MM-DD HH:MM): Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Current count: 3
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
Current count: 3
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Current count: 3
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: Vehicle not found.
Current count: 3
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Current count: 3
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Parking data saved.
```

### 2.3 階段一結束後 `parking_data.txt` 原始落盤內容
```text
# PARKING_SYSTEM_V1
MA-10-01	轎車	香檳金	2026-11-02 04:30
MB-20-02	SUV	白色	2026-11-01 09:05
MC-30-03	MPV	銀色	2026-11-01 05:00
```

---

## 三、階段二（Process 2）跨進程重啟、刪除與普通用戶確認原始捕獲實錄

### 3.1 階段二（Process 2）標準輸入序列（`p2_in.txt`）
```text
2
1
6
MB-20-02
1
3
0
1
1
3
0
0
```

### 3.2 階段二（Process 2）原始捕獲控制台輸出（`p2_out.txt`，`ExitCode: 0`）
```text
Loaded parking_data.txt.
Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
Current count: 3
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Existing plate: Updated and saved. Parking data saved.
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
Current count: 2
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Current count: 2
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
Current count: 2
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Current count: 2
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> Parking system: 1 = ordinary, 2 = administrator, 0 = exit
> Parking data saved.
```

### 3.3 階段二結束後 `parking_data.txt` 原始落盤內容
```text
# PARKING_SYSTEM_V1
MA-10-01	轎車	香檳金	2026-11-02 04:30
MC-30-03	MPV	銀色	2026-11-01 05:00
```

---

## 四、排練結論與跨文檔一致性核查

1. **與 `E06` 實機輸出 100% 對齊**：修正後的 `juli13_demo_script_and_checklist_zh.md` 與本實證報告完全一致地反映了 `E06` 的真實控制台提示與輸出格式。
2. **跨文檔審計（[JULI-11](mention://issue/01a1980f-5a5e-7242-a2e1-95a8df245fa4) & [JULI-12](mention://issue/01a1980f-6160-72bb-8ebf-219082ae42ed)）**：已同步核查 `juli11_user_guide_zh.md`、`juli12_development_report_zh.md`、`juli12_flowcharts.mmd` 及重新渲染後的 11 頁 `juli12_development_report_zh.pdf`，確認所有菜單/校驗提示詞及多步驟持久化與回滾協議（Multi-step save & rollback protocol）描述完全統一。
3. **倉庫與檢查點完整性**：排練全程於系統 `$env:TEMP` 隔離目錄執行並已清除臨時目錄，倉庫內已跟蹤產品源碼（`src/*`、`tests/*`）、Git 索引及 `E01`–`E06` 檢查點均未受任何改動。

# 酒店停車場管理系統（Hotel Parking Management System）完整功能演示腳本與錄製核對清單（修訂版 v2）

> 目錄更新：本文件位於 `docs/`；歷史視頻和時間戳位於本地 `docs/demo/`，舊 ZIP 位於 `dist/archive/`。這些歷史材料不隨當前交付包發布。命令從項目根目錄執行。

**項目名稱**：高級程式設計與軟件工程期末專題 — 酒店停車場管理系統（Hotel Parking Management System）  
**對應議題**：[JULI-13](mention://issue/01a1980f-6878-713b-9e89-a8be7590f771)（D03: Prepare and rehearse the complete demonstration script）  
**下游錄製議題**：[JULI-14](mention://issue/01a1980f-6fa0-7a86-8493-7c59850c53b5)（D04: Capture and verify the required full-process screen recording）  
**基準版本**：經獨立審查通過（[JULI-10](mention://issue/01a1980f-535c-7574-8051-a505815a9bdd) PASS）之 `E06` 完整實現版本（`e06-source.zip`，SHA-256: `844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e`）、`JULI-17` Google C++ Style 已審提交 `df3a71477b3635c93c64305f2e941f0ccb5f88b2`（`refs/juli17/style`）及已驗證中文用戶指南（[JULI-11](mention://issue/01a1980f-5a5e-7242-a2e1-95a8df245fa4)）  
**版本聲明（Supersession Notice）**：本修訂版（v2）正式取代並廢止初版 `juli13_demo_script_and_checklist_zh.md` 中與 `E06` 源代碼不一致的提示詞描述（如初版誤寫的 `Add failed:` 前綴、`New type:`/`New color:`、`Plate to modify:`/`Plate to delete:`、缺少 `Plate:`/`Category:`/`Color:` 標籤的輸出行，以及缺少 `# PARKING_SYSTEM_V1` 標頭的文件示例）。本文件所有菜單提示、輸入序列與預期輸出均百分之百對應 `src/menus.cpp`、`src/ordinary_user.cpp`、`src/admin_user.cpp`、`src/parking_service.cpp` 與 `src/storage.cpp` 的真實執行輸出。

---

## 一、演示前環境準備、動態時間戳生成與安全重置指南（Safe Reset Instructions）

為保證演示錄製（[JULI-14](mention://issue/01a1980f-6fa0-7a86-8493-7c59850c53b5)）在完全隔離、乾淨的環境下進行，且**絕對不影響**倉庫根目錄下的產品源碼（`src/*`、`tests/*`）及任何檢查點文件，請一律在獨立子目錄 `build/demo-recording` 中執行演示，並將最終錄製的視頻文件保存在該臨時目錄之外。

### 1.1 編譯與建立隔離演示目錄

在倉庫根目錄開啟 PowerShell 終端並執行：

```powershell
# 1. 設定終端為 UTF-8 編碼，確保中英文車牌、品類、顏色正常顯示與輸入
chcp 65001
[Console]::InputEncoding  = [System.Text.UTF8Encoding]::new($false)
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
$OutputEncoding           = [System.Text.UTF8Encoding]::new($false)

# 2. 編譯最新審查通過的 C++17 可執行檔（如已編譯可跳過）
if (!(Test-Path build)) { New-Item -ItemType Directory -Path build | Out-Null }
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc src/*.cpp -o build/parking.exe

# 3. 建立並進入隔離演示目錄 build/demo-recording
if (!(Test-Path build/demo-recording)) { New-Item -ItemType Directory -Path build/demo-recording | Out-Null }
Set-Location build/demo-recording
```

### 1.2 演示數據安全重置指令（Safe Reset Command）

每次重新排練或重新開始錄製前，只需在 `build/demo-recording` 目錄內執行以下安全重置指令，即可清除本地持久化數據檔與臨時/備份檔，將系統恢復至首次啟動（`First run: no parking data file.`）的空白狀態：

```powershell
# 安全重置：僅清除當前隔離演示目錄下的持久化數據與臨時/備份文件
Remove-Item -Path .\parking_data.txt, .\parking_data.txt.tmp, .\parking_data.txt.bak -Force -ErrorAction SilentlyContinue
```

> **安全保證**：上述指令僅作用於 `build/demo-recording` 內部，不會觸碰倉庫根目錄、Git 索引或任何快照文件。

### 1.3 免等待動態入場時間戳生成腳本（Zero-Wait Timestamp Setup）

由於系統使用當前系統時間（歸一化至本地整分鐘，`tm_sec = 0`）動態計算停放時長與費用，在正式開始錄製前，請先在 PowerShell 執行以下 6 行輔助指令，自動計算出對應於**當前錄製時刻**的 4 個測試時間戳，讓你無需等待即可在畫面上立即展示「不足 24 小時按小時進位（40 MOP）」、「單個 24 小時週期封頂（400 MOP）」、「跨 24 小時週期累加（440 MOP）」以及「未來時間攔截（`Entry time is in the future.`）」四種核心計費與校驗場景：

```powershell
$nowMin  = (Get-Date).AddSeconds(-(Get-Date).Second)
$t95m    = $nowMin.AddMinutes(-95).ToString("yyyy-MM-dd HH:mm")    # 停放 1小時35分 -> 進位 2 小時 = 40 MOP
$t21h    = $nowMin.AddMinutes(-1260).ToString("yyyy-MM-dd HH:mm")  # 停放 21小時0分 -> 單日封頂 = 400 MOP
$t25h    = $nowMin.AddMinutes(-1505).ToString("yyyy-MM-dd HH:mm")  # 停放 1天1小時5分 -> 400 + 40 = 440 MOP
$tFuture = $nowMin.AddDays(30).ToString("yyyy-MM-dd HH:mm")        # 未來時間 -> 觸發 Entry time is in the future.
Write-Host "請在演示中使用以下時間戳：`n  MA-10-01 ($t95m)  -> 40 MOP`n  MB-20-02 ($t21h)  -> 400 MOP (24h封頂)`n  MC-30-03 ($t25h)  -> 440 MOP (跨日計費)`n  未來時間 ($tFuture) -> Entry time is in the future."
```

### 1.4 預設演示樣本車輛一覽表

| 樣本序號 | 車牌號碼 (`Plate`) | 汽車品類 (`Category`) | 汽車顏色 (`Color`) | 入場時間設定 (`Entry time (YYYY-MM-DD HH:MM)`) | 預期停放時長 (`Duration`) | 預期停車費用 (`Fee`) | 演示目的 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **樣本 1** | `MA-10-01` | `轎車` | `黑色`（後修改為 `香檳金`） | `$t95m`（當前時間前 95 分鐘，如 `2026-11-02 04:30`） | `0 days 1 hours 35 minutes` | `40 MOP` | 驗證基礎小時進位計費（$\lceil 95/60 \rceil = 2\text{h} \times 20 = 40\text{ MOP}$）及管理員修改功能 |
| **樣本 2** | `MB-20-02` | `SUV` | `白色` | `$t21h`（當前時間前 21 小時，如 `2026-11-01 09:05`） | `0 days 21 hours 0 minutes` | `400 MOP` | 驗證 24 小時週期封頂規則（$21 \times 20 = 420 > 400 \rightarrow 400\text{ MOP}$）及重啟後刪除功能 |
| **樣本 3** | `MC-30-03` | `MPV` | `銀色` | `$t25h`（當前時間前 25 小時 5 分鐘，如 `2026-11-01 05:00`） | `1 days 1 hours 5 minutes` | `440 MOP` | 驗證跨日計費規則（首個 24h 週期封頂 $400 + \lceil 65/60 \rceil \times 20 = 440\text{ MOP}$） |
| **異常 A** | `MA-10-01` | `轎車` | `藍色` | `$t95m` | —（攔截） | —（攔截） | 驗證車牌唯一性約束（精確輸出：`Plate already exists.`） |
| **異常 B** | `MD-40-04` | `跑車` | `紅色` | `$tFuture`（未來日期，如 `2026-12-02 06:05`） | —（攔截） | —（攔截） | 驗證未來入場時間攔截（精確輸出：`Entry time is in the future.`） |

---

## 二、三大必備功能模塊與演示畫面映射核對清單（Function-to-Screen Mapping Checklist）

下表將專題要求之三大模塊（`1.1 汽車信息模塊`、`1.2 普通用戶模塊`、`1.3 管理員用戶模塊`）及持久化要求逐項映射至本演示腳本的具體步驟與 `E06` 真實控制台輸出畫面，供錄製（[JULI-14](mention://issue/01a1980f-6fa0-7a86-8493-7c59850c53b5)）與最終提交包審查（[JULI-15](mention://issue/01a1980f-76d1-7db1-8163-10e63d199302) / [JULI-16](mention://issue/01a1980f-7def-741f-87e1-b754b62c477d)）逐項核對：

| 模塊分類 | 功能子項 | 對應演示步驟 | 操作輸入摘要 | 預期螢幕輸出證據（Exact E06 Console Evidence） | 排練驗證狀態 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1.1 汽車信息模塊** | 1.1.1 車牌號碼（`Plate`）存儲、顯示與唯一性校驗 | 階段一 Step 3.1–3.4 | 按提示 `Plate: ` 錄入 `MA-10-01`、`MB-20-02`、`MC-30-03`；重複錄入 `MA-10-01` | 正常顯示 `Plate: MA-10-01 \| ...`；重複車牌精確輸出 `Plate already exists.` | [x] 已驗證 |
| **1.1 汽車信息模塊** | 1.1.2 汽車品類（`Category`）存儲、顯示與更新 | 階段一 Step 3.1–3.3, Step 5 | 按提示 `Category: ` 錄入 `轎車`、`SUV`、`MPV`；修改時再次輸入 `Category: ` | 列表與單車查詢輸出 `\| Category: 轎車 \|`、`\| Category: SUV \|`、`\| Category: MPV \|` | [x] 已驗證 |
| **1.1 汽車信息模塊** | 1.1.3 汽車顏色（`Color`）存儲、顯示與修改 | 階段一 Step 3.1, Step 5 | 按提示 `Color: ` 錄入 `黑色`，後於管理員 `5 = modify` 改為 `香檳金` | 修改前顯示 `\| Color: 黑色 \|`，修改後單行輸出 `Updated and saved. Parking data saved.` 並顯示 `\| Color: 香檳金 \|` | [x] 已驗證 |
| **1.1 汽車信息模塊** | 1.1.4 入場時間（`Entry time (YYYY-MM-DD HH:MM)`）解析與未來時間攔截 | 階段一 Step 3.1–3.3, Step 3.5 | 錄入 `YYYY-MM-DD HH:MM`；嘗試錄入未來時間 `$tFuture` | 正常顯示 `\| Entry: YYYY-MM-DD HH:MM \|`；未來時間精確輸出 `Entry time is in the future.` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.1 顯示所有停放車輛信息（`1 = list`） | 階段一 Step 2.1（空場）、Step 7.1（3 輛車）；階段二 Step 10.1（2 輛車） | 普通用戶選單 `> ` 輸入 `1` | 空場輸出 `No vehicles.` 與 `Current count: 0`；有車時按車牌升序輸出 `Plate: ... \| Category: ... \| Color: ... \| Entry: ... \| Duration: ... \| Fee: ... MOP` 並附帶 `Current count: N` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.2 車牌查詢指定車輛信息（`2 = find`） | 階段一 Step 7.2（命中與未命中） | 普通用戶選單輸入 `2` $\rightarrow$ `Plate: ` 輸入 `MA-10-01`；再次輸入 `2` $\rightarrow$ `Plate: ` 輸入 `MZ-99-99` | 命中輸出 `Plate: MA-10-01 \| Category: 轎車 \| Color: 香檳金 \| Entry: ... \| Duration: ... \| Fee: 40 MOP` 及 `Current count: 3`；未命中輸出 `Vehicle not found.` 及 `Current count: 3` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.3 停車費用自動計算與顯示（`Fee: X MOP`） | 階段一 Step 1（費率橫幅）、Step 7.1–7.2 | 查詢 `MA-10-01`、`MB-20-02`、`MC-30-03` | 啟動顯示 `Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.`；車輛行分別顯示 `Fee: 40 MOP`、`Fee: 400 MOP`、`Fee: 440 MOP` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.4 停放時長自動計算與顯示（`X days Y hours Z minutes`） | 階段一 Step 7.1–7.2 | 查詢 3 輛不同入場時間的車輛 | 分別顯示 `Duration: 0 days 1 hours 35 minutes`、`Duration: 0 days 21 hours 0 minutes`、`Duration: 1 days 1 hours 5 minutes` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.5 顯示當前場內車輛總數（`3 = count`） | 階段一 Step 2.2（`0`）、Step 7.3（`3`）；階段二 Step 10.2（`2`） | 普通用戶選單 `> ` 輸入 `3` | 獨立輸出 `Current count: 0`、`Current count: 3`、`Current count: 2` | [x] 已驗證 |
| **1.2 普通用戶模塊** | 1.2.6 權限隔離與退出登錄（`0 = logout`） | 階段一 Step 2.3（輸入 `4` 越權測試）、Step 2.4、Step 7.4 | 普通用戶選單輸入 `4`；輸入 `0` 登出 | 選單僅有 `Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout`，輸入 `4` 輸出 `Invalid choice.`；輸入 `0` 返回頂層選單 `Parking system: 1 = ordinary, 2 = administrator, 0 = exit` | [x] 已驗證 |
| **1.3 管理員用戶模塊** | 1.3.1 管理員共享普通用戶全部查詢能力（`1 = list`, `2 = find`, `3 = count`） | 階段一 Step 4.1–4.3 | 管理員選單依次輸入 `1`、`2`（查 `MB-20-02`）、`3` | 管理員無需切回普通用戶即可直接查看全部車輛列表、單車詳情（附 `Current count: 3`）與場內總數 `Current count: 3` | [x] 已驗證 |
| **1.3 管理員用戶模塊** | 1.3.2 增加車輛信息（`4 = add`）及即時持久化 | 階段一 Step 3.1–3.3 | 管理員選單輸入 `4`，按 `Plate: `、`Category: `、`Color: `、`Entry time (YYYY-MM-DD HH:MM): ` 錄入 3 輛樣本車 | 每次成功均單行輸出 `Updated and saved. Parking data saved.` | [x] 已驗證 |
| **1.3 管理員用戶模塊** | 1.3.3 修改車輛信息（`5 = modify`）及即時持久化 | 階段一 Step 5.1–5.2 | 管理員選單輸入 `5`，按提示依次輸入 `Existing plate: `（`MA-10-01`）、`Plate: `（`MA-10-01`）、`Category: `（`轎車`）、`Color: `（`香檳金`）、`Entry time (YYYY-MM-DD HH:MM): `（`$t95m`） | 單行輸出 `Updated and saved. Parking data saved.`；再以 `2 = find` 查詢顯示 `Color: 香檳金` 及 `Current count: 3` | [x] 已驗證 |
| **1.3 管理員用戶模塊** | 1.3.4 刪除車輛信息（`6 = delete`）及即時持久化 | 階段二 Step 9.2–9.3 | 管理員選單輸入 `6`，按提示 `Existing plate: ` 輸入 `MB-20-02` | 單行輸出 `Updated and saved. Parking data saved.`；隨後列表與計數降為 `Current count: 2` | [x] 已驗證 |
| **1.3 管理員用戶模塊** | 1.3.5 管理員退出登錄（`0 = logout`） | 階段一 Step 6；階段二 Step 9.4 | 管理員選單 `> ` 輸入 `0` | 安全退回頂層選單 `Parking system: 1 = ordinary, 2 = administrator, 0 = exit` | [x] 已驗證 |
| **持久化與重啟恢復** | 本地文本文件 `parking_data.txt` 持久化與跨進程重啟載入 | 階段一 Step 8（退出並檢查文件）；階段二 Step 9.1（重啟載入）、Step 11 | 頂層選單輸入 `0` 退出程式；查看 `parking_data.txt`；重新執行 `..\parking.exe` | 退出輸出 `Parking data saved.`；`parking_data.txt` 首行為 `# PARKING_SYSTEM_V1`；重啟首行輸出 `Loaded parking_data.txt.` 且完整保留修改後的記錄 | [x] 已驗證 |

---

## 三、分步演示腳本與旁白指引（Step-by-Step Demonstration Script）

本腳本分為**階段一（Process 1：初次啟動、空場檢查、管理員新增/校驗/共享查詢/修改、普通用戶全功能查詢、退出存檔）**與**階段二（Process 2：重啟載入驗證、管理員刪除、普通用戶確認刪除結果、最終存檔）**。

---

### 階段一（Process 1）：初次啟動、新增、修改、多梯度計費與普通用戶查詢

#### Step 1：首次啟動與系統計費規則展示
- **執行動作**：在 `build/demo-recording` 目錄（已執行安全重置，無 `parking_data.txt`）執行：
  ```powershell
  ..\parking.exe
  ```
- **預期精確畫面（`src/main.cpp` & `src/menus.cpp`）**：
  ```text
  First run: no parking data file.
  Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
  Parking system: 1 = ordinary, 2 = administrator, 0 = exit
  > 
  ```
- **演示要點說明**：指出首行 `First run: no parking data file.` 證明當前為首次全新啟動；第二行清楚列出澳門元（MOP）計費標準（每開始 1 小時收費 20 MOP，每連續 24 小時週期封頂 400 MOP）。

#### Step 2：普通用戶空場查詢、計數與權限邊界驗證（模組 1.2）
- **執行動作與輸入序列**：
  1. 在頂層選單 `> ` 輸入 `1`（進入普通用戶會話）。
  2. 輸入 `1`（查看所有車輛 `list`）。
  3. 輸入 `3`（查看當前車輛總數 `count`）。
  4. 輸入 `4`（測試普通用戶是否能越權調用管理員的第 4 項新增功能）。
  5. 輸入 `0`（普通用戶退出登錄 `logout`）。
- **預期精確畫面（`src/ordinary_user.cpp` & `src/menus.cpp`）**：
  ```text
  Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
  > 1
  No vehicles.
  Current count: 0
  Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
  > 3
  Current count: 0
  Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
  > 4
  Invalid choice.
  Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
  > 0
  Parking system: 1 = ordinary, 2 = administrator, 0 = exit
  > 
  ```

#### Step 3：管理員登錄、新增 3 輛樣本車與異常輸入攔截（模組 1.1 & 1.3）
- **執行動作與輸入序列**：
  1. 在頂層選單 `> ` 輸入 `2`（進入管理員會話 `Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout`）。
  2. **Step 3.1 新增樣本 1（短時停放 1h35m $\rightarrow$ 40 MOP）**：
     - 輸入 `4`（`add`）
     - `Plate: ` 輸入 `MA-10-01`
     - `Category: ` 輸入 `轎車`
     - `Color: ` 輸入 `黑色`
     - `Entry time (YYYY-MM-DD HH:MM): ` 輸入 `$t95m` 的值（例如 `2026-11-02 04:30`）
  3. **Step 3.2 新增樣本 2（長時停放 21h $\rightarrow$ 單日封頂 400 MOP）**：
     - 輸入 `4`（`add`）
     - `Plate: ` 輸入 `MB-20-02`
     - `Category: ` 輸入 `SUV`
     - `Color: ` 輸入 `白色`
     - `Entry time (YYYY-MM-DD HH:MM): ` 輸入 `$t21h` 的值（例如 `2026-11-01 09:05`）
  4. **Step 3.3 新增樣本 3（跨日停放 1d 1h 5m $\rightarrow$ 跨日累加 440 MOP）**：
     - 輸入 `4`（`add`）
     - `Plate: ` 輸入 `MC-30-03`
     - `Category: ` 輸入 `MPV`
     - `Color: ` 輸入 `銀色`
     - `Entry time (YYYY-MM-DD HH:MM): ` 輸入 `$t25h` 的值（例如 `2026-11-01 05:00`）
  5. **Step 3.4 驗證重複車牌攔截**：
     - 輸入 `4`（`add`） $\rightarrow$ `Plate: ` 輸入 `MA-10-01`、`Category: ` 輸入 `轎車`、`Color: ` 輸入 `藍色`、`Entry time (YYYY-MM-DD HH:MM): ` 輸入 `$t95m`
  6. **Step 3.5 驗證未來入場時間攔截**：
     - 輸入 `4`（`add`） $\rightarrow$ `Plate: ` 輸入 `MD-40-04`、`Category: ` 輸入 `跑車`、`Color: ` 輸入 `紅色`、`Entry time (YYYY-MM-DD HH:MM): ` 輸入 `$tFuture`（例如 `2026-12-02 06:05`）
- **預期精確畫面（`src/admin_user.cpp`）**：
  - 前 3 輛車每次錄入成功後均單行輸出：
    ```text
    Updated and saved. Parking data saved.
    ```
  - 重複車牌 `MA-10-01` 輸出：
    ```text
    Plate already exists.
    ```
  - 未來入場時間輸出：
    ```text
    Entry time is in the future.
    ```

#### Step 4：管理員直接使用共享查詢功能（模組 1.3 共享 1.2 能力）
- **執行動作與輸入序列**：
  1. 在管理員選單 `> ` 輸入 `1`（`list` 顯示所有車輛）。
  2. 輸入 `2`（`find` 按車牌查找） $\rightarrow$ `Plate: ` 輸入 `MB-20-02`。
  3. 輸入 `3`（`count` 顯示總數）。
- **預期精確畫面**：
  ```text
  Plate: MA-10-01 | Category: 轎車 | Color: 黑色 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
  Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
  Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
  Current count: 3
  ```
  單車查詢 `MB-20-02` 輸出：
  ```text
  Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
  Current count: 3
  ```
  輸入 `3` 輸出：
  ```text
  Current count: 3
  ```

#### Step 5：管理員修改車輛信息並即時驗證（模組 1.3 `5 = modify`）
- **執行動作與輸入序列（注意 `5 = modify` 共需輸入 5 行：先輸入 `Existing plate: `，再輸入完整的 `Plate: `、`Category: `、`Color: `、`Entry time (YYYY-MM-DD HH:MM): `）**：
  1. 在管理員選單 `> ` 輸入 `5`（`modify`）。
  2. `Existing plate: ` 輸入 `MA-10-01`。
  3. `Plate: ` 輸入 `MA-10-01`（保持原車牌號，或可修改為其他未佔用車牌）。
  4. `Category: ` 輸入 `轎車`。
  5. `Color: ` 輸入 `香檳金`（將原本的 `黑色` 改為 `香檳金`）。
  6. `Entry time (YYYY-MM-DD HH:MM): ` 輸入相同的 `$t95m`（例如 `2026-11-02 04:30`）。
  7. 輸入 `2`（`find`） $\rightarrow$ `Plate: ` 輸入 `MA-10-01` 驗證修改結果。
- **預期精確畫面（`src/admin_user.cpp:58-92`）**：
  ```text
  Existing plate: MA-10-01
  Plate: MA-10-01
  Category: 轎車
  Color: 香檳金
  Entry time (YYYY-MM-DD HH:MM): 2026-11-02 04:30
  Updated and saved. Parking data saved.
  ```
  隨後單車查詢 `MA-10-01` 輸出：
  ```text
  Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
  Current count: 3
  ```

#### Step 6：管理員退出登錄（模組 1.3 `0 = logout`）
- **執行動作**：在管理員選單 `> ` 輸入 `0`。
- **預期精確畫面**：退回頂層選單 `Parking system: 1 = ordinary, 2 = administrator, 0 = exit`。

#### Step 7：普通用戶登錄並驗證完整查詢、計費、時長與計數（模組 1.2）
- **執行動作與輸入序列**：
  1. 在頂層選單 `> ` 輸入 `1`（進入 `Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout`）。
  2. **Step 7.1 顯示全部車輛（`1 = list`）**：輸入 `1`。
  3. **Step 7.2 車牌精確查詢（`2 = find`）**：
     - 輸入 `2` $\rightarrow$ `Plate: ` 輸入 `MA-10-01`（存在）。
     - 輸入 `2` $\rightarrow$ `Plate: ` 輸入 `MZ-99-99`（不存在）。
  4. **Step 7.3 場內車輛計數（`3 = count`）**：輸入 `3`。
  5. **Step 7.4 普通用戶登出（`0 = logout`）**：輸入 `0`。
- **預期精確畫面**：
  - 輸入 `1` 清晰展示三種計費梯度與修改後的顏色：
    ```text
    Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
    Plate: MB-20-02 | Category: SUV | Color: 白色 | Entry: 2026-11-01 09:05 | Duration: 0 days 21 hours 0 minutes | Fee: 400 MOP
    Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
    Current count: 3
    ```
  - 查 `MA-10-01` 輸出單車詳情與 `Current count: 3`；查 `MZ-99-99` 輸出 `Vehicle not found.` 與 `Current count: 3`。
  - 輸入 `3` 輸出 `Current count: 3`。
  - 輸入 `0` 返回頂層選單 `Parking system: 1 = ordinary, 2 = administrator, 0 = exit`。

#### Step 8：退出第一階段程式並檢查本地持久化數據檔
- **執行動作**：
  1. 在頂層選單 `> ` 輸入 `0`（`exit`）。
  2. 在 PowerShell 終端執行 `Get-Content .\parking_data.txt -Encoding UTF8` 展示落盤文件內容。
- **預期精確畫面**：
  - 程式退出時輸出 `Parking data saved.`。
  - `parking_data.txt` 內容包含首行版本標記 `# PARKING_SYSTEM_V1` 及製表符（Tab）分隔的 3 行升序記錄：
    ```text
    # PARKING_SYSTEM_V1
    MA-10-01	轎車	香檳金	2026-11-02 04:30
    MB-20-02	SUV	白色	2026-11-01 09:05
    MC-30-03	MPV	銀色	2026-11-01 05:00
    ```

---

### 階段二（Process 2）：跨進程重啟載入、管理員刪除車輛與最終確認

#### Step 9：重啟程式驗證自動載入與管理員刪除操作（模組 1.3 `6 = delete`）
- **執行動作與輸入序列**：
  1. **Step 9.1 重新啟動程式**：在同一個 `build/demo-recording` 目錄再次執行 `..\parking.exe`。
  2. 在頂層選單 `> ` 輸入 `2`（進入管理員會話）。
  3. 輸入 `1`（`list`）確認重啟後 3 輛車（含 `香檳金` 修改）完整恢復。
  4. **Step 9.2 刪除指定車輛**：輸入 `6`（`delete`） $\rightarrow$ 按提示 `Existing plate: ` 輸入 `MB-20-02`。
  5. **Step 9.3 驗證刪除後列表與總數**：輸入 `1`（`list`），再輸入 `3`（`count`）。
  6. **Step 9.4 管理員登出**：輸入 `0`（`logout`）。
- **預期精確畫面**：
  - 啟動首行輸出 `Loaded parking_data.txt.`（與首次啟動的 `First run: no parking data file.` 形成鮮明對比）。
  - 刪除 `MB-20-02` 後單行輸出：
    ```text
    Updated and saved. Parking data saved.
    ```
  - 隨後列表僅剩 `MA-10-01` 與 `MC-30-03`，且輸出 `Current count: 2`。

#### Step 10：普通用戶登錄確認刪除同步生效
- **執行動作與輸入序列**：
  1. 在頂層選單 `> ` 輸入 `1`（進入普通用戶會話）。
  2. 輸入 `1`（`list`）查看剩餘車輛。
  3. 輸入 `3`（`count`）查看剩餘總數。
  4. 輸入 `0`（`logout`）退回頂層選單。
- **預期精確畫面**：
  ```text
  Plate: MA-10-01 | Category: 轎車 | Color: 香檳金 | Entry: 2026-11-02 04:30 | Duration: 0 days 1 hours 35 minutes | Fee: 40 MOP
  Plate: MC-30-03 | Category: MPV | Color: 銀色 | Entry: 2026-11-01 05:00 | Duration: 1 days 1 hours 5 minutes | Fee: 440 MOP
  Current count: 2
  ```

#### Step 11：最終退出程式並再次驗證 `parking_data.txt`
- **執行動作**：
  1. 在頂層選單 `> ` 輸入 `0`（`exit`）。
  2. 在終端執行 `Get-Content .\parking_data.txt -Encoding UTF8`。
- **預期精確畫面**：
  - 程式輸出 `Parking data saved.` 並正常結束（Exit code `0`）。
  - `parking_data.txt` 包含 `# PARKING_SYSTEM_V1` 標頭及剩餘的 2 輛車記錄：
    ```text
    # PARKING_SYSTEM_V1
    MA-10-01	轎車	香檳金	2026-11-02 04:30
    MC-30-03	MPV	銀色	2026-11-01 05:00
    ```

---

## 四、錄製前最終核對清單（Final Recording Checklist）

1. [x] **編碼設定**：終端已執行 `chcp 65001` 並設定 UTF-8 Input/Output Encoding。
2. [x] **目錄隔離與視頻保留位置**：演示工作目錄位於 `build/demo-recording`，而錄屏生成的視頻文件請保存於該臨時目錄之外（例如 `build/` 或獨立提交目錄），以便清理演示數據時不誤刪視頻。
3. [x] **安全重置**：錄製開始前已刪除 `build/demo-recording/parking_data.txt*`，確保 Process 1 首行顯示 `First run: no parking data file.`。
4. [x] **動態時間戳就緒**：已執行 1.3 節 PowerShell 時間戳計算命令，確保無需等待即可呈現 `40 MOP`、`400 MOP`、`440 MOP` 三種梯度。
5. [x] **修改與刪除輸入格式對齊**：管理員 `5 = modify` 嚴格依次輸入 5 項（`Existing plate:`、`Plate:`、`Category:`、`Color:`、`Entry time (YYYY-MM-DD HH:MM):`）；管理員 `6 = delete` 嚴格按 `Existing plate:` 輸入目標車牌。
6. [x] **全功能覆蓋**：完整涵蓋 `1.1 汽車信息模塊`、`1.2 普通用戶模塊`、`1.3 管理員用戶模塊` 及跨進程持久化重啟驗證。

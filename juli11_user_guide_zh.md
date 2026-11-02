# 酒店停車場管理系統 — 使用說明書（普通用戶與管理員操作指南）

## 1. 系統簡介與功能範圍

本系統為《高級編程與軟件工程》期末項目開發的**澳門本地酒店停車場管理系統**，基於 **C++17 標準庫**構建，採用命令行交互界面（Console UI）與本地 UTF-8 文本文件持久化。

### 1.1 必選功能模組（已完整實現並通過驗證）
系統嚴格按照課程作業要求劃分為三個獨立的核心功能模組：
1. **汽車信息模塊（1.1）**：
   - 支持手動錄入車牌號（Plate）、汽車品類（Category，如 `轎車`、`SUV`）、汽車顏色（Color，如 `黑色`、`白色`）及停車時間點（Entry time，格式 `YYYY-MM-DD HH:MM`）。
   - 支持按車牌號顯示（List）、查找（Find）、修改（Modify）和刪除（Delete）已停放車輛資訊。
   - 支持統計每輛車的停車停留時長（`X days Y hours Z minutes`）以及當前停車場內的車輛總數（`Current count`）。
   - 支持將汽車資訊自動保存至本地文件 `parking_data.txt`，並在程序重啟時自動載入與恢復。
2. **普通用戶模塊（1.2）**：
   - 支持查詢並顯示所有在場汽車資訊、停留時長及實時計算的停車費用（MOP）。
   - 支持按車牌號單獨查詢指定車輛的詳細資訊、停留時長與停車費用。
   - 支持統計當前在場車輛總數。
   - 支持顯式退出普通用戶登錄（`0 = logout`），返回角色選擇主菜單。
   - **權限邊界**：普通用戶僅具備只讀查詢與統計權限，無法調用任何增、刪、改接口。
3. **管理員用戶模塊（1.3）**：
   - 具備普通用戶模塊的所有查詢與統計功能（`1 = list`、`2 = find`、`3 = count`）。
   - 額外具備添加車輛（`4 = add`）、修改車輛（`5 = modify`）、刪除車輛（`6 = delete`）的完整管理權限，每次修改成功後立即同步保存至本地數據文件，並提供显式退出登錄（`0 = logout`）。

### 1.2 範圍邊界與排除的可選功能
為保持架構簡潔與測試確定性，本系統**不包含**以下未經要求的可選擴展功能：
- **不包含 OpenCV 自動圖像車牌識別**：作業允許「手動輸入車牌號」或「使用 opencv 識別」，本項目採用**手動輸入車牌號**方案。
- **不包含圖形界面（GUI）、Web/網絡服務或外部數據庫**（如 SQLite/MySQL）：純 C++17 標準庫實現，直接讀寫本地 TSV 文件。
- **不包含賬號密碼註冊子系統、車位容量上限攔截、預約系統或歷史離場歸檔表**。

---

## 2. 環境要求、編譯與啟動步驟

### 2.1 運行環境要求
- **操作系統**：Windows（PowerShell 5.1 / PowerShell 7+）、Linux 或 macOS。
- **編譯器**：支持 **C++17** 標準的 C++ 編譯器（如 `g++` (MinGW-w64 / GCC) 或 `clang++`）。
- **字符編碼建議**：在 Windows PowerShell 中輸入或顯示中文品類/顏色（如 `轎車`、`白色`）時，建議先執行以下命令將終端輸入輸出編碼設為 UTF-8：
  ```powershell
  [Console]::InputEncoding = [System.Text.UTF8Encoding]::new($false)
  [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
  $OutputEncoding = [System.Text.UTF8Encoding]::new($false)
  ```

### 2.2 編譯指令
在項目根目錄下打開 PowerShell，依次執行以下命令創建 `build` 目錄並編譯主程序：

```powershell
New-Item -ItemType Directory -Force build
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc src/*.cpp -o build/parking.exe
```

### 2.3 啟動指令
在項目根目錄（或任意希望存放 `parking_data.txt` 的工作目錄）下運行：

```powershell
./build/parking.exe
```

### 2.4 自動化測試編譯與執行指令（可選核驗）
如需運行系統自帶的單元測試、持久化恢復測試、菜單交互測試與多進程端到端測試，可在項目根目錄執行：

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc tests/test_main.cpp src/menus.cpp src/ordinary_user.cpp src/admin_user.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/tests.exe
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc tests/test_storage.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_storage.exe
g++ -std=c++17 -Wall -Wextra -pedantic -I. -Isrc tests/test_menus.cpp src/menus.cpp src/ordinary_user.cpp src/admin_user.cpp src/vehicle.cpp src/parking_service.cpp src/fee_calculator.cpp src/storage.cpp -o build/test_menus.exe
./build/tests.exe
./build/test_storage.exe
./build/test_menus.exe
./tests/test_e2e.ps1
```

---

## 3. 啟動載入與角色登錄 / 退出流程

### 3.1 啟動時的數據載入提示
程序啟動時會自動檢查當前工作目錄下的 `parking_data.txt`：
- **首次運行（無數據文件）**：
  ```text
  First run: no parking data file.
  Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
  Parking system: 1 = ordinary, 2 = administrator, 0 = exit
  > 
  ```
- **已有合法數據文件**：
  ```text
  Loaded parking_data.txt.
  Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.
  Parking system: 1 = ordinary, 2 = administrator, 0 = exit
  > 
  ```
- **主文件缺失但存在有效備份文件（`parking_data.txt.bak`）**：
  ```text
  Recovered parking data from parking_data.txt.bak.
  ```

### 3.2 頂層角色選擇菜單
在頂層提示符 `> ` 後輸入對應數字並按回車：
- 輸入 `1`：進入**普通用戶會話（Ordinary user session）**。
- 輸入 `2`：進入**管理員會話（Administrator session）**。
- 輸入 `0`：**安全退出程序**。退出前系統會執行一次最終保存並輸出：
  ```text
  Parking data saved.
  ```
- 輸入其他無效字符（如 `9` 或 `abc`）：系統輸出 `Invalid choice.` 並重新顯示頂層菜單，不會崩潰或死循環。
- 輸入流結束（`EOF`，如 `Ctrl+Z` / `Ctrl+D`）：程序安全終止退出。

---

## 4. 普通用戶操作指南（Ordinary User Session）

在頂層菜單輸入 `1` 後，進入普通用戶菜單：

```text
Ordinary user session: 1 = list, 2 = find, 3 = count, 0 = logout
> 
```

### 4.1 顯示所有汽車資訊、停留時長與停車費（`1 = list`）
- **操作**：輸入 `1` 並回車。
- **顯示結果（當車庫為空時）**：
  ```text
  No vehicles.
  Current count: 0
  ```
- **顯示結果（當車庫有車時，按車牌字典序升序排列）**：
  ```text
  Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
  Plate: MB-56-78 | Category: SUV | Color: 白色 | Entry: 2026-11-02 04:00 | Duration: 0 days 1 hours 40 minutes | Fee: 40 MOP
  Current count: 2
  ```

### 4.2 按車牌號查詢單輛汽車資訊與費用（`2 = find`）
- **操作**：輸入 `2` 並回車，系統提示 `Plate: `，輸入目標車牌號（如 `MA-12-34`）並回車。
- **找到車輛時的輸出**：
  ```text
  Plate: MA-12-34 | Category: 轎車 | Color: 銀色 | Entry: 2026-11-01 08:00 | Duration: 0 days 21 hours 40 minutes | Fee: 400 MOP
  Current count: 2
  ```
- **車牌不存在時的輸出**：
  ```text
  Vehicle not found.
  Current count: 2
  ```
- **車牌格式非法（如為空、含內部空格或控制字符）時的輸出**：
  ```text
  Unable to retrieve vehicle or calculate duration.
  Current count: 2
  ```

### 4.3 停車時長與車輛總數統計（`3 = count`）
- **操作**：輸入 `3` 並回車。
- **輸出**：
  ```text
  Current count: 2
  ```
  *(註：每輛車的具體停留時長與總數亦可在 `1 = list` 與 `2 = find` 中一併查看。)*

### 4.4 退出普通用戶登錄（`0 = logout`）與權限限制
- **退出登錄**：輸入 `0` 並回車，立即退出普通用戶會話並返回頂層菜單 `Parking system: 1 = ordinary, 2 = administrator, 0 = exit`。
- **權限保護**：若普通用戶嘗試輸入管理員指令（如 `4`、`5`、`6`），系統會拒絕並輸出：
  ```text
  Invalid choice.
  ```

---

## 5. 管理員操作指南（Administrator Session）

在頂層菜單輸入 `2` 後，進入管理員菜單：

```text
Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout
> 
```

### 5.1 共享查詢與統計功能（`1 = list`、`2 = find`、`3 = count`）
管理員可直接輸入 `1`（顯示全部車輛、時長與費用）、`2`（按車牌查找）、`3`（查看在場車輛總數），交互格式與普通用戶完全一致。

### 5.2 添加汽車信息（`4 = add`）
- **操作**：輸入 `4` 並回車，按提示依次輸入四個必填屬性：
  ```text
  Plate: MA-12-34
  Category: 轎車
  Color: 黑色
  Entry time (YYYY-MM-DD HH:MM): 2026-11-01 08:00
  ```
- **成功並落盤保存時的輸出**：
  ```text
  Updated and saved. Parking data saved.
  ```

### 5.3 修改汽車資訊（`5 = modify`）
- **操作**：輸入 `5` 並回車，先輸入場內已存在的原車牌號（`Existing plate: `），再輸入修改後的新車牌號（可保持原車牌不變或改為新車牌）、品類、顏色及入場時間：
  ```text
  Existing plate: MA-12-34
  Plate: MA-12-34
  Category: 轎車
  Color: 銀色
  Entry time (YYYY-MM-DD HH:MM): 2026-11-01 08:00
  ```
- **成功並落盤保存時的輸出**：
  ```text
  Updated and saved. Parking data saved.
  ```
- **保護機制**：若原車牌不存在，輸出 `Vehicle not found.`；若修改後的新車牌與場內另一輛現存車牌衝突，輸出 `Plate already exists.`；若任一新字段或時間不合法，輸出對應錯誤提示，且**原車輛記錄保持不變**。

### 5.4 刪除汽車資訊（`6 = delete`）
- **操作**：輸入 `6` 並回車，在 `Existing plate: ` 後輸入要刪除的車牌號（如 `MB-56-78`）。
- **成功並落盤保存時的輸出**：
  ```text
  Updated and saved. Parking data saved.
  ```
- **若車牌不存在**：輸出 `Vehicle not found.`。

### 5.5 退出管理員登錄（`0 = logout`）
- **操作**：輸入 `0` 並回車，返回頂層角色選擇菜單，可隨時切換至普通用戶會話核對修改結果，或再次輸入 `0` 退出系統。

---

## 6. 停車時長與收費計算規則（Approved Fee Policy）

本系統採用經批准的確定性澳門元（`MOP`）整數計費規則，在程序啟動時顯式提示：
> `Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.`

### 6.1 分鐘級時間戳歸一化（Minute Normalization）
- 系統將車輛入場時間（`Entry`）與當前查詢參考時間（`Reference`）統一歸一化至本地整分鐘（`tm_sec = 0`），計算精確停留總分鐘數 $\Delta m$。
- 若 $\Delta m < 0$（即入場時間晚於當前系統時間），視為無效未來時間並拒絕。

### 6.2 計費公式與封頂規則
1. **0 分鐘停留**（同一分鐘內入場並查詢，$\Delta m = 0$）：收費 **0 MOP**。
2. **按連續 24 小時（1440 分鐘）週期分段計費**：
   - 完整天數 $D = \lfloor \Delta m / 1440 \rfloor$，每個完整 24 小時週期固定收費 **400 MOP**。
   - 剩餘不滿 24 小時的部分 $R = \Delta m \bmod 1440$（分鐘）：
     - 按「不足 1 小時按 1 小時計算（per started hour）」向上取整計費小時數：$H_{\text{started}} = \lceil R / 60 \rceil = \lfloor (R + 59) / 60 \rfloor$。
     - 剩餘部分費用為 $\min(400, H_{\text{started}} \times 20)\text{ MOP}$。
   - **總費用公式**：
     $$\text{Fee (MOP)} = D \times 400 + \min\left(400,\ \left\lceil \frac{R}{60} \right\rceil \times 20\right)$$

### 6.3 典型計費示例對照表
| 停留總時長 | 顯示時長 (`Duration`) | 計算過程 | 應收停車費 (`Fee`) |
| :--- | :--- | :--- | :--- |
| 0 分鐘 | `0 days 0 hours 0 minutes` | 0 小時 | **0 MOP** |
| 1 分鐘 | `0 days 0 hours 1 minutes` | 第 1 個計費小時 | **20 MOP** |
| 1 小時 0 分鐘（60m） | `0 days 1 hours 0 minutes` | 剛好 1 個計費小時 | **20 MOP** |
| 1 小時 1 分鐘（61m） | `0 days 1 hours 1 minutes` | 進入第 2 個計費小時 | **40 MOP** |
| 19 小時 59 分鐘（1199m） | `0 days 19 hours 59 minutes` | 20 個計費小時（達到單日封頂） | **400 MOP** |
| 20 小時 0 分鐘（1200m） | `0 days 20 hours 0 minutes` | 20 個計費小時（達到單日封頂） | **400 MOP** |
| 23 小時 59 分鐘（1439m） | `0 days 23 hours 59 minutes` | 24 個計費小時，受 400 MOP 封頂限制 | **400 MOP** |
| 24 小時 0 分鐘（1440m） | `1 days 0 hours 0 minutes` | 1 個完整 24 小時週期 | **400 MOP** |
| 24 小時 1 分鐘（1441m） | `1 days 0 hours 1 minutes` | 第 1 週期 400 MOP + 第 2 週期第 1 小時 20 MOP | **420 MOP** |
| 48 小時 0 分鐘（2880m） | `2 days 0 hours 0 minutes` | 2 個完整 24 小時週期 | **800 MOP** |
| 48 小時 1 分鐘（2881m） | `2 days 0 hours 1 minutes` | 2 週期 800 MOP + 第 3 週期第 1 小時 20 MOP | **820 MOP** |

---

## 7. 字段格式要求、本地數據文件與異常恢復指南

### 7.1 四項必填字段校驗規則
1. **車牌號（`Plate`）**：
   - 必填，自動去除首尾半角空格（`' '`）。
   - **禁止包含內部空格**（如 `MA 12` 會被拒絕，推薦使用 `MA-12-34`、`MB-56-78` 等格式）。
   - **禁止包含任何 ASCII 控制字符**（如 Tab `\t`、回車 `\r`、換行 `\n` 等 `0x00–0x1F` 及 `0x7F`）。
2. **汽車品類（`Category`）與汽車顏色（`Color`）**：
   - 必填，自動去除首尾半角空格（`' '`），允許包含內部半角空格（如 `Model Y`、`Pearl White`）及 UTF-8 中文（如 `轎車`、`SUV`、`黑色`、`白色`）。
   - 禁止純空格或包含任何 ASCII 控制字符（如 `\t`）。
3. **停車時間點（`Entry time`）**：
   - 必須嚴格為 **16 字符的 24 小時制本地時間格式 `YYYY-MM-DD HH:MM`**（例如 `2026-11-01 08:00`）。
   - 系統會嚴格校驗公曆月份天數與閏年規則（如平年 `2026-02-29` 或 `2026-04-31` 會被拒絕；閏年 `2024-02-29` 合法）。
   - **禁止未來時間**：若輸入的入場時間（精確到分）晚於當前系統時間（精確到分），系統拒絕錄入並提示 `Entry time is in the future.`。

### 7.2 本地持久化文件格式（`parking_data.txt`）
- **文件位置**：默認位於程序運行時的**當前工作目錄**下，文件名為 `parking_data.txt`。
- **文件結構**：UTF-8 編碼文本文件。首行為版本標記 `# PARKING_SYSTEM_V1`（空行及 `#` 開頭的注釋行在載入時自動忽略），隨後每行對應一輛在場汽車，共 4 列，以單個制表符（Tab `\t`）分隔，並按車牌號字典序升序保存：
  ```text
  # PARKING_SYSTEM_V1
  MA-12-34	轎車	銀色	2026-11-01 08:00
  MB-56-78	SUV	白色	2026-11-02 04:00
  ```
- **多步驟安全保存與回滾協議（`.tmp` 與 `.bak`）**：
  每次保存時，系統採用**非單步原子的多步驟持久化與回滾協議（Multi-step persistence & rollback protocol）**：先在內存校驗完整快照，寫入臨時文件 `parking_data.txt.tmp` 並檢查 `flush()`/`close()`；若原 `parking_data.txt` 存在，先調用 `ParseFile()` 解析確認其未被外部破壞（若已損壞則設 `blocked_ = true` 並刪除 `.tmp` 中止保存），再移除舊 `.bak` 並將原文件重命名備份為 `parking_data.txt.bak`（若原文件不存在但發現殘留 `.bak`，則設 `blocked_ = true` 拒絕覆蓋）；最後將 `.tmp` 重命名為 `parking_data.txt`。若替換失敗，系統自動嘗試將 `.bak` 重命名回滾為 `parking_data.txt`（若回滾也失敗則保留 `.bak` 並設 `blocked_ = true`）；替換成功後清理 `.bak`。

### 7.3 常見校驗提示、僅內存更新警告與故障恢復步驟

| 場景分類 | 終端精確英文提示信息 | 原因說明與用戶恢復步驟 |
| :--- | :--- | :--- |
| **菜單輸入錯誤** | `Invalid choice.` | 輸入了當前菜單不支持的選項編號（如在普通用戶菜單輸入 `4`）。請按提示重新輸入有效數字。 |
| **必填字段非法** | `Invalid required field.` | 車牌、品類或顏色為空，或車牌含空格，或輸入中含 Tab/控制字符。請檢查後重新執行添加/修改。 |
| **時間格式或日曆非法** | `Invalid entry time.` | 入場時間不符合 `YYYY-MM-DD HH:MM` 格式（如缺少前導零 `2026-1-2 8:00`）或日期不存在（如 `2026-02-29`）。請按 16 位格式重新輸入。 |
| **未來入場時間** | `Entry time is in the future.` | 輸入的入場時間晚於當前電腦系統時間。請核對當前時間後輸入過去或當前的時間點。 |
| **車牌重複或改名衝突** | `Plate already exists.` | 添加了已在場的車牌，或修改車牌時目標新車牌已被另一輛在場車輛佔用。請使用唯一車牌號。 |
| **查無此車** | `Vehicle not found.` | 查詢、修改或刪除時輸入的車牌不在當前停車場內。可先輸入 `1`（`list`）查看所有在場車牌。 |
| **保存失敗時的「僅內存更新」警告** | `Updated in memory, but save failed: <reason>` | **重要提示**：管理員的增/刪/改操作**已在當前內存中生效**（在當前會話中查詢可見），但因磁盤權限不足、臨時文件無法寫入、或外部損壞了數據文件導致落盤失敗。此時請勿直接強行關閉窗口，應先根據 `<reason>` 檢查當前目錄寫入權限、移除阻礙的只讀/目錄衝突，或修復被外部改壞的 `parking_data.txt`。 |
| **啟動時數據文件格式損壞** | `Line N: expected four tab-separated fields.`<br>`Line N: invalid timestamp.`<br>`Line N: duplicate plate.`<br>`Line N: invalid field or timestamp.` | `parking_data.txt` 第 `N` 行被外部手動編輯破壞。系統為保護已有數據，採用**全有或全無（All-or-Nothing）**載入策略並鎖定保存（退出碼 `1`）。**恢復步驟**：用文本編輯器打開 `parking_data.txt`，修正第 `N` 行的 Tab 分隔符、車牌或時間格式後重新啟動程序。 |
| **保存被安全鎖定** | `Save blocked: resolve the load or recovery error and reload first.`<br>或 `Existing file is invalid; save blocked. ...` | 當載入失敗、備份恢復失敗或保存前發現磁盤上的 `parking_data.txt` 被外部改壞時，系統自動鎖定寫入以防止覆蓋損壞前的有效數據或 `.bak` 備份。請先保留並檢查 `parking_data.txt` 及 `parking_data.txt.bak`，修復後重啟程序。 |

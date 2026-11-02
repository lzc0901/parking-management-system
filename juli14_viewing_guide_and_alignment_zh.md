# 酒店停車場管理系統（Hotel Parking Management System）實機演示錄屏中文觀看指南與時間戳核對清單

**對應議題**：[JULI-14](mention://issue/01a1980f-6fa0-7a86-8493-7c59850c53b5)（D04: Capture and verify the required full-process screen recording）  
**關聯演示腳本議題**：[JULI-13](mention://issue/01a1980f-6878-713b-9e89-a8be7590f771)（D03: Prepare and rehearse the complete demonstration script）  
**對應視頻文件**：`juli14-demo.mp4`（H.264 MP4，解析度 `1498 × 770`，幀率 `10 fps`，總時長 `02:57.6`，SHA-256：`d2930f896445e7f88a8155899585362cbe6ca140ca4341320a022e024cf3b109`）  
**驗證基準版本**：經 [JULI-10](mention://issue/01a1980f-535c-7574-8051-a505815a9bdd) 審查通過之 `E06` 產品源碼（`e06-source.zip`，SHA-256：`844645d20ca75de8d3b28a90815040f3e4d4be5da9a4a7904ae201ea6aec593e`；後續 [JULI-17](mention://issue/01a19a63-9796-78ca-a8ed-d26785df7071) Google C++ Style 整理版 `df3a71477b3635c93c64305f2e941f0ccb5f88b2` 保持完全相同的控制台提示、計費與持久化行為）  
**對齊審查結論（Script-Alignment Verdict）**：**PASS（無需重錄 / No Recapture Required）** — 實機錄屏完整覆蓋三大必選功能模塊（`1.1 汽車信息模塊`、`1.2 普通用戶模塊`、`1.3 管理員用戶模塊`）及跨進程重啟持久化驗證；所有菜單提示、校驗拒絕消息、停放時長與澳門元（MOP）計費梯度均在視頻畫面中清晰可讀。

---

## 一、實機無聲錄屏與排練腳本之差異對照說明（Reconciliation Notes）

為確保評審教師與獨立審查員（[JULI-16](mention://issue/01a1980f-7def-741f-87e1-b754b62c477d)）在觀看無聲實機錄屏 `juli14-demo.mp4` 時能夠準確對照 [JULI-13](mention://issue/01a1980f-6878-713b-9e89-a8be7590f771) 演示腳本，特此說明實機錄製環境下的四項客觀表現（不宣稱與排練文本完全逐字相同，而是精確記錄實機畫面）：

### 1.1 英文品類與顏色樣本值對照（English Category/Color Sample Values）
由於實機錄屏在自動化桌面終端（`cmd.exe`）中進行，為避免自動化按鍵注入受系統輸入法（IME）狀態干擾，錄屏中使用了與中文腳本一一對應的英文品類（`Category`）與顏色（`Color`）樣本值，車牌號與業務邏輯則完全一致：

| 車牌號碼 (`Plate`) | `JULI-13` 腳本中文樣本值 | `juli14-demo.mp4` 實機錄屏英文樣本值 | 錄屏設定入場時間 (`Entry`) | 預期計費梯度 (`Fee`) |
| :--- | :--- | :--- | :--- | :--- |
| **`MA-10-01`**（新增） | 品類：`轎車` / 顏色：`黑色` | `Category: Sedan` / `Color: Black` | `2026-11-02 04:39` | `40 MOP`（不足 2 小時按 2 小時計算） |
| **`MA-10-01`**（修改後） | 品類：`轎車` / 顏色：`香檳金` | `Category: Sedan` / `Color: Gold` | `2026-11-02 04:39` | `40 MOP`（顏色成功更新為 `Gold`） |
| **`MB-20-02`** | 品類：`SUV` / 顏色：`白色` | `Category: SUV` / `Color: White` | `2026-11-01 09:14` | `400 MOP`（單個 24 小時週期封頂） |
| **`MC-30-03`** | 品類：`MPV` / 顏色：`銀色` | `Category: MPV` / `Color: Silver` | `2026-11-01 05:09` | `440 MOP`（首日封頂 `400` + 次日 `40`） |
| **`MA-10-01`**（重複攔截） | 品類：`轎車` / 顏色：`藍色` | `Category: Sedan` / `Color: Blue` | `2026-11-02 04:39` | 拒絕錄入：`Plate already exists.` |
| **`MD-40-04`**（未來攔截） | 品類：`跑車` / 顏色：`紅色` | `Category: Sports` / `Color: Red` | `2026-12-02 06:14` | 拒絕錄入：`Entry time is in the future.` |

### 1.2 實時系統時鐘推進導致的分鐘數遞增（Live-Clock Duration Progression）
- 錄屏開始前（約本地時間 `06:14`）按相對偏移（`-95` 分鐘、`-1260` 分鐘、`-1505` 分鐘）生成了入場時間 `04:39`、`09:14` 與 `05:09`。
- 由於實機錄屏持續近 3 分鐘（`00:00` 至 `02:57.6`），系統時鐘從 `06:14` 自然推進至 `06:16`–`06:17`，因此畫面中顯示的停留分鐘數隨實時時鐘真實遞增：
  - 在 `01:22`–`01:52`（系統時間 `06:16`）：三輛車的顯示時長分別為 `0 days 1 hours 37 minutes`、`0 days 21 hours 2 minutes`、`1 days 1 hours 7 minutes`。
  - 在 `01:54`–`02:55`（系統時間跨入 `06:17`）：三輛車的顯示時長分別遞增 1 分鐘為 `0 days 1 hours 38 minutes`、`0 days 21 hours 3 minutes`、`1 days 1 hours 8 minutes`。
- **計費結果恆定不變**：由於小時進位與 24 小時週期封頂規則，上述分鐘數推進完全落在同一計費區間內，三輛車的停車費全程穩定保持為 **`40 MOP`**、**`400 MOP`** 與 **`440 MOP`**。

### 1.3 終端窗口右邊緣自動換行說明（Terminal Right-Edge Line Wrapping Verification）
在 `1498 × 770` 的終端窗口寬度下，包含完整標籤的單行車輛輸出（如 `Plate: ... | Category: ... | Color: ... | Entry: ... | Duration: ... | Fee: ... MOP`）在右邊緣發生了控制台自動折行（Line Wrapping），具體表現如下（經逐幀放大檢查確認**全部數字與單位完整無截斷、清晰可讀**）：
- 當 `Color: Black`（5 字符）時（見 `01:22` 管理員列表）：
  - 第 1 行末尾為 `| Fee:`，下一行開頭完整顯示 `40 MOP`。
- 當 `Color: Gold`（4 字符）時（見 `01:32` 修改後查詢、`01:54` 普通用戶列表、`02:19` 重啟後列表、`02:41` 最終列表）：
  - `MA-10-01` 行末尾顯示 `| Fee: 4`，緊接著下一行首字符顯示 `0 MOP`（合起來即為 `40 MOP`）。
- 對於 `MB-20-02`（`400 MOP`）與 `MC-30-03`（`440 MOP`）：
  - 行末分別顯示 `| Fee: 40` 與 `| Fee: 44`，緊接著下一行首字符均顯示 `0 MOP`（合起來即為 `400 MOP` 與 `440 MOP`）。

### 1.4 重啟後驗證順序微調說明
- 在 `02:19` 進程 2（Process 2）重啟載入 `Loaded parking_data.txt.` 後，錄屏先進入**普通用戶會話（`1 = ordinary` $\rightarrow$ `1 = list`）**直觀展示了重啟後完整恢復的 3 輛車記錄，隨後登出並切換至**管理員會話（`2 = administrator` $\rightarrow$ `6 = delete`）**刪除 `MB-20-02`、檢查數量 `Current count: 2`，最後再切換回**普通用戶會話（`1 = ordinary` $\rightarrow$ `1 = list` & `3 = count`）**確認剩餘 2 輛車（`MA-10-01` 與 `MC-30-03`）。此流程完整覆蓋了所有必選功能與重啟持久化要求。

---

## 二、`juli14-demo.mp4` 分段時間戳與中文解說觀看指南（Timestamped Viewing Guide）

評審教師或審查員在播放 `juli14-demo.mp4` 時，可參照以下精確時間軸與中文解說核對每一項操作：

| 視頻時間區間 | 演示階段與對應模塊 | 畫面實際輸入序列 | 畫面實際輸出與關鍵證據（Verified Screen Output） | 中文旁白 / 觀看解說指引 |
| :--- | :--- | :--- | :--- | :--- |
| **`00:00 – 00:03`** | **階段一啟動**：首次運行與收費標準橫幅 | 執行 `..\parking.exe` | `First run: no parking data file.`<br>`Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.`<br>`Parking system: 1 = ordinary, 2 = administrator, 0 = exit` | 系統在隔離空目錄下首次啟動，自動檢測無歷史數據文件，並清晰打印每小時 20 MOP、每連續 24 小時封頂 400 MOP 的收費規則。 |
| **`00:03 – 00:15`** | **普通用戶模塊（1.2）**：空車庫查詢、統計、越權攔截與登出 | `1` (ordinary) $\rightarrow$ `1` (list) $\rightarrow$ `3` (count) $\rightarrow$ `4` (invalid) $\rightarrow$ `0` (logout) | `No vehicles.` / `Current count: 0` $\rightarrow$ `Current count: 0` $\rightarrow$ 輸入 `4` 輸出 `Invalid choice.` $\rightarrow$ 輸入 `0` 返回頂層菜單 | 普通用戶登錄後查看空車庫與數量（均為 `0`）；嘗試輸入管理員編號 `4` 被系統拒絕（`Invalid choice.`），體現嚴格的權限隔離；隨後輸入 `0` 登出。 |
| **`00:15 – 00:30`** | **汽車信息模塊（1.1）& 管理員模塊（1.3）**：添加第 1 輛車 `MA-10-01` | `2` (admin) $\rightarrow$ `4` (add) $\rightarrow$ `MA-10-01` / `Sedan` / `Black` / `2026-11-02 04:39` | `Updated and saved. Parking data saved.` | 管理員登錄並選擇 `4 = add`，錄入短時停放車輛 `MA-10-01`（轎車、黑色），成功保存至本地文件。 |
| **`00:30 – 00:43`** | **汽車信息模塊（1.1）& 管理員模塊（1.3）**：添加第 2 輛車 `MB-20-02` | `4` (add) $\rightarrow$ `MB-20-02` / `SUV` / `White` / `2026-11-01 09:14` | `Updated and saved. Parking data saved.` | 管理員繼續添加長時停放車輛 `MB-20-02`（SUV、白色，停放約 21 小時，用於驗證單日 400 MOP 封頂）。 |
| **`00:43 – 00:56`** | **汽車信息模塊（1.1）& 管理員模塊（1.3）**：添加第 3 輛車 `MC-30-03` | `4` (add) $\rightarrow$ `MC-30-03` / `MPV` / `Silver` / `2026-11-01 05:09` | `Updated and saved. Parking data saved.` | 管理員添加跨日停放車輛 `MC-30-03`（MPV、銀色，停放約 1 天 1 小時，用於驗證跨 24 小時週期累加計費）。 |
| **`00:56 – 01:09`** | **汽車信息模塊（1.1）**：重複車牌唯一性攔截驗證 | `4` (add) $\rightarrow$ `MA-10-01` / `Sedan` / `Blue` / `2026-11-02 04:39` | `Plate already exists.` | 嘗試再次錄入已存在的車牌 `MA-10-01`，系統精確攔截並輸出 `Plate already exists.`，原記錄不受影響。 |
| **`01:09 – 01:22`** | **汽車信息模塊（1.1）**：未來入場時間攔截驗證 | `4` (add) $\rightarrow$ `MD-40-04` / `Sports` / `Red` / `2026-12-02 06:14` | `Entry time is in the future.` | 嘗試錄入未來時間 `2026-12-02 06:14`，系統精確攔截並輸出 `Entry time is in the future.`。 |
| **`01:22 – 01:32`** | **管理員模塊（1.3）**：管理員使用共享查詢與統計功能（`1/2/3`） | `1` (list) $\rightarrow$ `2` (find `MB-20-02`) $\rightarrow$ `3` (count) | 列表顯示 3 車：`MA-10-01` (`0d 1h 37m`, `40 MOP`)、`MB-20-02` (`0d 21h 2m`, `400 MOP`)、`MC-30-03` (`1d 1h 7m`, `440 MOP`) 及 `Current count: 3`；單查 `MB-20-02` 成功；統計總數為 `3` | 驗證管理員具備普通用戶的所有查詢與統計能力，直接查看全部車輛、單車查找與總數統計。 |
| **`01:32 – 01:54`** | **管理員模塊（1.3）**：修改車輛資訊（`5 = modify`）、單車復核與管理員登出 | `5` (modify) $\rightarrow$ `Existing plate: MA-10-01` $\rightarrow$ `MA-10-01` / `Sedan` / `Gold` / `2026-11-02 04:39` $\rightarrow$ `2` (find `MA-10-01`) $\rightarrow$ `0` (logout) | `Updated and saved. Parking data saved.`；隨後查詢顯示 `Plate: MA-10-01 \| Category: Sedan \| Color: Gold \| ... \| Fee: 40 MOP` 及 `Current count: 3`；輸入 `0` 返回頂層菜單 | 管理員選擇 `5 = modify` 將 `MA-10-01` 的顏色由 `Black` 修改為 `Gold` 並即時存盤；通過 `2 = find` 驗證修改生效後輸入 `0` 退出管理員登錄。 |
| **`01:54 – 02:15`** | **普通用戶模塊（1.2）**：普通用戶全功能查詢（列表、車牌命中/未命中、時長、費用、總數、登出） | `1` (ordinary) $\rightarrow$ `1` (list) $\rightarrow$ `2` (find `MA-10-01`) $\rightarrow$ `2` (find `MZ-99-99`) $\rightarrow$ `3` (count) $\rightarrow$ `0` (logout) | 列表顯示 3 車（`MA-10-01` 為 `Gold`, `1h 37m`, `40 MOP`；`MB-20-02` 為 `400 MOP`；`MC-30-03` 為 `440 MOP`）；單查 `MA-10-01` 命中（此時跨分顯示 `1h 38m`, `40 MOP`）；單查 `MZ-99-99` 輸出 `Vehicle not found.` 與 `Current count: 3`；統計 `Current count: 3`；`0` 登出 | 切換至普通用戶視角，完整驗證列表展示、按車牌查詢（命中與未命中）、三種計費梯度、時長顯示、總數統計及顯式登出。 |
| **`02:15 – 02:28`** | **持久化與重啟恢復（階段二啟動）**：退出進程 1 並重新啟動進程 2 | `0` (exit) $\rightarrow$ 重新運行 `..\parking.exe` $\rightarrow$ `1` (ordinary) $\rightarrow$ `1` (list) $\rightarrow$ `0` (logout) | 退出輸出 `Parking data saved.`；重啟首行輸出 `Loaded parking_data.txt.`；普通用戶列表完整顯示重啟前保存的 3 輛車（含 `Gold` 修改）與 `Current count: 3` | 退出第一個進程並重新啟動程序，首行 `Loaded parking_data.txt.` 證明本地文件成功載入，所有車輛屬性與修改跨進程完整保留。 |
| **`02:28 – 02:41`** | **管理員模塊（1.3）**：刪除指定車輛（`6 = delete`）、統計確認與登出 | `2` (admin) $\rightarrow$ `6` (delete) $\rightarrow$ `Existing plate: MB-20-02` $\rightarrow$ `3` (count) $\rightarrow$ `0` (logout) | `Updated and saved. Parking data saved.` $\rightarrow$ `Current count: 2` $\rightarrow$ 返回頂層菜單 | 管理員登錄並選擇 `6 = delete` 刪除 `MB-20-02`，系統即時更新並落盤保存，場內總數更新為 `Current count: 2`。 |
| **`02:41 – 02:57.6`** | **普通用戶模塊（1.2）& 最終保存退出**：確認刪除後列表與總數並正常退出系統 | `1` (ordinary) $\rightarrow$ `1` (list) $\rightarrow$ `3` (count) $\rightarrow$ `0` (logout) $\rightarrow$ `0` (exit) | 列表僅顯示 `MA-10-01` (`40 MOP`) 與 `MC-30-03` (`440 MOP`) 及 `Current count: 2`；單獨統計 `Current count: 2`；最終退出輸出 `Parking data saved.` | 普通用戶再次登錄確認 `MB-20-02` 已從列表中移除且總數為 `2`，最後登出並正常退出程序（`Parking data saved.`）。 |

---

## 三、三大必選模塊全功能覆蓋最終核對表（All-Function Video Coverage Matrix）

| 課程任務書模塊 | 必選功能點 | `juli14-demo.mp4` 對應時間戳 | 視覺可讀性與功能核驗結果 |
| :--- | :--- | :--- | :--- |
| **1.1 汽車信息模塊** | 添加汽車信息（車牌號、品類、顏色、停車時間點） | `00:15 – 00:56` | **PASS**（完整錄入 3 輛車，且於 `00:56`、`01:09` 驗證重複車牌與未來時間攔截） |
| **1.1 汽車信息模塊** | 顯示、查找、修改、刪除汽車資訊 | 顯示/查找：`01:22`, `01:54`<br>修改：`01:32`<br>刪除：`02:28` | **PASS**（按車牌精確查找、修改 `MA-10-01` 顏色為 `Gold`、刪除 `MB-20-02` 均完整呈現） |
| **1.1 汽車信息模塊** | 停車時長統計（停留時間與在場車輛總數） | `00:03`, `01:22`, `01:54`, `02:19`, `02:35`, `02:41` | **PASS**（`Duration: X days Y hours Z minutes` 與 `Current count: 0 / 3 / 2` 全程清晰可讀） |
| **1.1 汽車信息模塊** | 汽車資訊保存至本地文件並支持重啟載入 | 保存：`00:25`–`02:55`<br>重啟載入：`02:15 – 02:28` | **PASS**（每次寫操作輸出 `Updated and saved. Parking data saved.`，重啟輸出 `Loaded parking_data.txt.`） |
| **1.2 普通用戶模塊** | 查詢、顯示所有汽車資訊及停車費資訊 | `00:03`（空場）、`01:54`（3 車）、`02:19`（重啟後）、`02:41`（刪後 2 車） | **PASS**（`40 MOP`、`400 MOP`、`440 MOP` 三檔費用及單車命中/未命中查詢清晰可見） |
| **1.2 普通用戶模塊** | 停車時長統計与在場數量查詢 | `00:03`, `01:54`, `02:41` | **PASS**（列表內嵌時長與 `3 = count` 獨立計數均完整演示） |
| **1.2 普通用戶模塊** | 退出普通用戶登錄（`0 = logout`）及權限隔離 | `00:08`（輸入 `4` 攔截）、`00:12`, `02:12`, `02:26`, `02:50` | **PASS**（越權輸入 `4` 提示 `Invalid choice.`，輸入 `0` 正常退回角色選擇主菜單） |
| **1.3 管理員用戶模塊** | 具備普通用戶模塊的所有功能（`1 = list`, `2 = find`, `3 = count`） | `01:22 – 01:32`, `01:45`, `02:35` | **PASS**（管理員會話內直接執行全部列表、按車牌查找與總數統計） |
| **1.3 管理員用戶模塊** | 具有修改和刪除汽車資訊的功能及退出登錄 | 新增：`00:15 – 00:56`<br>修改：`01:32 – 01:50`<br>刪除：`02:28 – 02:40` | **PASS**（增、改、刪及 `0 = logout` 全部在實機畫面中完成並即時持久化） |

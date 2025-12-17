# level_crossing_model

## 平交道自動偵測與控制系統  
(Level Crossing Automatic Detection and Control System)

本專題以 PIC18 微控制器為核心，實作一套平交道自動偵測與控制系統。  
系統透過 **雙超音波感測器（Ultrasonic × 2）** 偵測火車進出方向，並整合  
**紅黃綠 LED、七段顯示器、蜂鳴器與伺服馬達（閘門）**，  
以狀態機（State Machine）方式模擬真實平交道的運作流程。

---

## 系統狀態說明（State Machine）

系統主要分為三個狀態：

### STATE_TRAFFIC（Normal Traffic Mode）
- 平交道正常運作狀態
- **綠燈亮起**
- 閘門維持升起
- 蜂鳴器關閉
- 七段顯示器顯示紅綠燈切換的倒數秒數  
  - 綠燈 → 黃燈：10 秒  
  - 黃燈 → 紅燈：3 秒  
  - 紅燈 → 綠燈：10 秒  
- 系統會持續監測超音波感測器是否偵測到火車

---

### STATE_BLOCKED（Train Detected / Gate Closing）
- 當 **任一顆超音波感測器偵測到火車距離小於 10 cm**
- 系統立即中斷紅綠燈倒數流程
- **紅燈亮起**
- 蜂鳴器啟動警示
- 七段顯示器關閉（顯示 idle 符號）
- 伺服馬達開始放下柵欄（閘門下降）
- 同時紀錄是哪一顆超音波先觸發（`last_trigger`）

---

### STATE_RELEASE（Train Leaving / Gate Opening）
- 系統持續監測「另一顆」超音波感測器
- 若火車由 Ultrasonic1 進入，則等待 Ultrasonic2 偵測到火車尾端
- 若火車由 Ultrasonic2 進入，則等待 Ultrasonic1 偵測到火車尾端
- 當確認火車完全通過平交道：
  - 蜂鳴器關閉
  - 伺服馬達反向轉動，升起柵欄
  - 系統回到 **STATE_TRAFFIC**

---

## 火車偵測機制（Train Detection）

### Ultrasonic Sensors × 2
- **Ultrasonic1 / Ultrasonic2** 分別設置於平交道兩側
- 用來判斷火車的「進站方向」與「離站時機」
- 利用 Timer1 量測 ECHO 訊號寬度，換算成距離（cm）

### last_trigger 設計
- `last_trigger` 用來記錄「哪一顆超音波最先偵測到火車」
  - `1`：Ultrasonic1 first
  - `2`：Ultrasonic2 first
- 避免要求兩顆感測器同時觸發（實務上不可能）
- release 條件改為「另一顆感測器觸發」，以正確判斷火車尾端離開

---

## 輸出裝置說明

### 🚦 LED（紅黃綠燈）
- 綠燈：正常通行
- 黃燈：警告倒數中
- 紅燈：火車通過或路口處於紅燈狀態，禁止通行

### ⏱ 七段顯示器（7-Segment Display）
- 顯示紅綠燈狀態切換前的剩餘秒數
- 火車通過期間顯示 idle 狀態

### 🔊 蜂鳴器（Buzzer）
- 在火車進入平交道後啟動
- 於閘門完全升起後關閉

### ⚙️ 伺服馬達（Gate Motor）
- 控制平交道柵欄升降
- 下降：火車進入
- 上升：火車完全通過
  
## 腳位配置
超音波
| 功能      | 腳位 | 說明           |
|-----------|------|----------------|
| US1_TRIG  | RB0  | 輸出 trigger   |
| US1_ECHO  | RB1  | 輸入 echo      |
| US2_TRIG  | RB2  | 輸出 trigger   |
| US2_ECHO  | RB3  | 輸入 echo      |
| Timer1    | TMR1 | 量 echo 時間   |

馬達
| 功能        | 腳位 |
|-------------|------|
| GATE_1  | CCP1 |      
| GATE_2  | CCP2  |        

紅黃綠LED
| LED    | 腳位 |
|--------|------|
| RED    | RA0  |
| YELLOW | RA1  |
| GREEN  | RA2  |

七段顯示器
| 功能              | 腳位             |
|-------------------|------------------|
| a b c d e f g     | RD0 ~ RD6        |


蜂鳴器
| 功能 | 腳位 |
|------|------|
| BUZZ | RA3  |


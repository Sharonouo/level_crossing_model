# level_crossing_model
## 平交道自動偵測與控制系統
- 使用 LED 作為紅綠燈顯示，正常情況下會是state 0(idle)，亮綠燈，閘門升起，七段顯示器及蜂鳴器關閉。
- 當火車進入平交道時，光感測器會偵測到火車的進入，狀態變為state 1(warning)，紅綠燈設為黃色，七段顯示器開始倒數還有幾秒閘門會開始放下，蜂鳴器發出警示聲。在倒數結束前，車輛應盡速通過或停下。
- 倒數結束後，控制伺服馬達開始放下柵欄，進入state 2(passing)，LED亮紅燈，蜂鳴器持續鳴叫並關閉七段顯示器。
- 當火車離開平交道時，光感測器會偵測到火車的離開，進入state 3(rising)，控制伺服馬達升起柵欄，紅綠燈持續為紅色，蜂鳴器持續鳴叫。
- 閘門上升完畢後，回到state 0(idle)。
- 七段顯示器顯示紅綠燈的倒數時間
## 1. Train Detection 火車偵測 (Ultrasonic × 2)
- Ultrasonic1&2：偵測火車是否在平交道警示範圍內
- boolean is_idle 若為1，代表目前無火車通過；火車一通過其中一個Ultrasonic後，is_idle由1轉0，並開始進入state 1&2的流程；當火車通過另一個Ultrasonic後，進入state 3，且is_idle變回1。
- 用 Timer1 計 echo 寬度 → 換算成距離 (distance cm)
## 2. Gate Control 閘門控制 (4 motors)
- 假設每個閘門只有「升起 / 降下」，不需要精細角度：
  - 用 DC 馬達 + H-bridge（L298N 之類）
  - 每個馬達 2 根控制線（正轉/反轉）or 四個閘門同一側共用控制線
- 控制邏輯：
  - gate_down()：全部下降
  - gate_up()：全部上升
  - 利用「固定轉動時間」控制（例如轉 1 秒剛好從 0° 到 90°）
## 3. Traffic Light 紅黃綠燈控制
- 平時：綠燈亮（車可以通行）
- 列車接近：黃燈閃爍 → 轉紅燈
- 列車通過中：紅燈恆亮
## 4. 7-Segment 秒數顯示 (2 digits)
- 顯示「倒數關閉」時間或「列車通過剩餘秒數」
- multiplex 驅動：7 個 segment 線 + 2 個位選線
- 用 Timer0 中斷每 2~5ms 更換顯示位
## 5. Buzzer 警報聲
- 平時關閉
- 門要降下 / 已關閉：
  - 可用 PWM 做「逼——逼——」的節奏
  - 或簡單 on/off 閃爍就好
## 腳位配置(暫定
超音波
| 功能      | 腳位 | 說明           |
|-----------|------|----------------|
| US1_TRIG  | RB0  | 輸出 trigger   |
| US1_ECHO  | RB1  | 輸入 echo      |
| US2_TRIG  | RA4  | 輸出 trigger   |
| US2_ECHO  | RA5  | 輸入 echo      |
| Timer1    | TMR1 | 量 echo 時間   |

馬達
| 功能        | 腳位 |
|-------------|------|
| GATE_L_IN1  | RC0  |      
| GATE_L_IN2  | RC1  |      
| GATE_R_IN1  | RC2  |     
| GATE_R_IN2  | RC3  |  

紅黃綠LED
| LED    | 腳位 |
|--------|------|
| RED    | RA0  |
| YELLOW | RA1  |
| GREEN  | RA2  |

七段顯示器
| 功能              | 腳位             |
|-------------------|------------------|
| a b c d e f g     | RE0 ~ RE6        |


蜂鳴器
| 功能 | 腳位 |
|------|------|
| BUZZ | RA3  |


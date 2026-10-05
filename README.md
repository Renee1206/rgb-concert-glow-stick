# 演唱會應援棒

以 **ESP32 / ESP8266** 實作演唱會應援棒控制系統。  
系統以 ESP32 作為中央控制 Server，透過 **Wi-Fi UDP Broadcast** 將 RGB 燈光指令傳送至多支應援棒，並整合 **Blynk** 手機控制介面，實現全體同步控制、個別應援棒控制、音樂燈光秀、Binary Counter 與呼吸燈等效果。

## Features

- 使用 ESP32 作為中央控制 Server
- 支援 ESP32 與 ESP8266 應援棒 Client
- 透過 Wi-Fi UDP Broadcast 傳送燈光控制訊息
- 使用 Client ID 區分不同應援棒
- 整合 Blynk 手機控制介面
- 支援全體 RGB 顏色同步控制
- 支援個別應援棒 RGB 顏色控制
- 支援音樂燈光秀
- 支援 Binary Counter 燈光模式
- 支援呼吸燈效果

---

## System Architecture

```text
                    ┌─────────────────┐
                    │    Blynk App    │
                    │  Mobile Control │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │   ESP32 Server  │
                    │                 │
                    │ RGB / Mode      │
                    │ Control         │
                    └────────┬────────┘
                             │
                       UDP Broadcast
                        Port 12345
                             │
           ┌─────────────────┼─────────────────┐
           │                 │                 │
     ┌─────▼─────┐     ┌─────▼─────┐     ┌─────▼──────┐
     │ ESP32 #1  │     │ ESP32 #2  │ ... │ ESP8266 #4│
     │ Client ID1│     │ Client ID2│     │ Client ID4 │
     └─────┬─────┘     └─────┬─────┘     └─────┬──────┘
           │                 │                 │
        RGB LED           RGB LED           RGB LED
```

所有裝置皆連接至相同的 Wi-Fi 網路。

ESP32 Server 將燈光控制訊息透過 UDP Broadcast 傳送至區域網路中的所有 Client。每支應援棒具有自己的 Client ID，收到封包後只會解析屬於自己 ID 的 RGB 資料。

---

## Project Structure

```text
rgb-concert-glow-stick/
│
├── clientESP32/
│   └── clientESP32.ino
│
├── clientESP8266/
│   └── clientESP8266.ino
│
├── server/
│   ├── server.ino
│   └── index.py
└── .gitignore
```

### `server/server.ino`

ESP32 中央控制端，主要負責：

- 連接 Wi-Fi
- 連接 Blynk
- 接收手機端控制指令
- 管理不同燈光模式
- 建立各支應援棒的 RGB 控制訊息
- 透過 UDP Broadcast 傳送資料至 Client

### `clientESP32/clientESP32.ino`

ESP32 應援棒接收端。

目前三支 ESP32 Client 分別使用：

```text
Glow Stick 1 → ID = 1
Glow Stick 2 → ID = 2
Glow Stick 3 → ID = 3
```

Client 持續監聽 UDP Port `12345`，收到 Server 廣播訊息後，解析與自身 ID 相符的 RGB 數值並控制 LED。

### `clientESP8266/clientESP8266.ino`

由於實作時 ESP32 數量不足，第 4 支應援棒改使用 ESP8266：

```text
Glow Stick 4 → ID = 4
```

其控制方式與 ESP32 Client 相同，同樣透過 Wi-Fi 接收 UDP Broadcast，並根據 Client ID 取得自己的 RGB 控制資料。

若有額外 ESP32，也可使用 `clientESP32.ino`，並將 Client ID 修改為 `4`。

---

## Communication Protocol

Server 與 Client 之間使用 **UDP Broadcast** 進行通訊。

```text
UDP Port    : 12345
Broadcast IP: 255.255.255.255
```

### Message Format

控制封包格式：

```text
[ID:R/G/B/Delay,ID:R/G/B/Delay,...]
```

例如：

```text
[1:255/0/0/0,2:0/255/0/0,3:0/0/255/0,4:255/255/255/0]
```

代表：

| Client | R | G | B | Color |
|---|---:|---:|---:|---|
| 1 | 255 | 0 | 0 | Red |
| 2 | 0 | 255 | 0 | Green |
| 3 | 0 | 0 | 255 | Blue |
| 4 | 255 | 255 | 255 | White |

所有 Client 都會收到相同的 Broadcast 封包，但每支 Client 只會取出與自身 ID 相符的 RGB 資料。

封包中的 `Delay` 欄位目前仍會被 Client 解析，但尚未實際用於燈光延遲控制。

---

## Lighting Modes

### 1. Music Light Show

Server 內預先建立 RGB 燈光序列與對應時間點，依照時間依序傳送不同 RGB 指令至四支應援棒，使燈光能配合音樂產生變化。

當音樂燈光秀開始時，Server 會透過 Serial 輸出：

```text
START
```

可供外部音樂播放程式接收訊號後同步開始播放音樂。

> 目前 Repository 中主要提供 ESP32 / ESP8266 控制程式，外部音樂播放程式與音樂檔案未包含於此 Repository。

---

### 2. Broadcast RGB Control

透過 Blynk 的 RGB Color Picker 選擇顏色後，Server 會將相同 RGB 數值設定至所有應援棒，達到全體同步變色效果。

```text
Blynk App
    ↓
ESP32 Server
    ↓
UDP Broadcast
    ↓
All Glow Sticks
```

---

### 3. Individual Glow Stick Control

個別應援棒控制模式透過 Blynk 的 `V103` 啟用，程式內部使用：

```cpp
mode = 6;
```

啟用後，可透過 `V1`～`V4` 分別設定四支應援棒的 RGB 顏色。

雖然各支應援棒可以分別設定顏色，但 Server 實際上仍使用 **UDP Broadcast** 傳送完整封包，再由各個 Client 根據自己的 ID 取得對應資料。

因此，此模式屬於 **基於 Client ID 的個別控制**，而非 UDP Unicast。

---

### 4. Binary Counter

利用四支應援棒表示四個 Binary Bit，依序呈現：

```text
0000
0001
0010
0011
...
1111
```

透過不同應援棒的亮滅組合形成二進位計數燈光效果。

---

### 5. Breathing Light

透過持續改變 RGB PWM 數值產生呼吸燈效果。

四支應援棒分為：

```text
Group A → Glow Stick 1、4
Group B → Glow Stick 2、3
```

兩組使用相反的亮度變化，使燈光呈現交替呼吸效果。

---

## Blynk Control

| Virtual Pin | Function |
|---|---|
| `V0` | 全部應援棒 RGB 同步控制 |
| `V1` | Glow Stick 1 RGB |
| `V2` | Glow Stick 2 RGB |
| `V3` | Glow Stick 3 RGB |
| `V4` | Glow Stick 4 RGB |
| `V6` | 啟動 Music Light Show |
| `V102` | 燈光模式選擇 |
| `V103` | 啟用個別應援棒控制 |

### V102 Mode

`V102` 負責選擇四種主要燈光模式：

```text
1 → Music Light Show
2 → Broadcast RGB Control
3 → Binary Counter
4 → Breathing Light
```

個別應援棒控制並不是 `V102` 的第 5 種燈光模式，而是透過 `V103` 啟用；啟用後程式內部將 `mode` 設為 `6`。

---

## Hardware

### Server

- ESP32 Development Board
- Wi-Fi Network
- Smartphone with Blynk

### Client

- ESP32 Development Board × 3
- ESP8266 Development Board × 1
- RGB LED
- 限流電阻（建議依 LED 規格加裝）
- 杜邦線
- 電源供應器

---

## Pin Configuration

### ESP32 Client

| RGB | GPIO |
|---|---:|
| Red | GPIO 5 |
| Green | GPIO 17 |
| Blue | GPIO 16 |

程式設定：

```cpp
const int redPin = 5;
const int greenPin = 17;
const int bluePin = 16;
```

### ESP8266 Client

| RGB | GPIO |
|---|---:|
| Red | GPIO 2 |
| Green | GPIO 4 |
| Blue | GPIO 5 |

程式設定：

```cpp
const int redPin1 = 2;
const int greenPin1 = 4;
const int bluePin1 = 5;
```

---

## Software Requirements

### Arduino IDE

需要安裝對應的：

- ESP32 Board Package
- ESP8266 Board Package
- Blynk Library

### Server Libraries

```cpp
#include <BlynkSimpleEsp32.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
```

### ESP32 Client Libraries

```cpp
#include <WiFi.h>
#include <WiFiUDP.h>
#include <esp_wifi.h>
#include <array>
#include <regex>
#include <string>
```

### ESP8266 Client Libraries

```cpp
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <array>
#include <regex>
#include <string>
```

---

## Getting Started

### 1. Clone Repository

```bash
git clone https://github.com/Renee1206/rgb-concert-glow-stick.git
cd rgb-concert-glow-stick
```

### 2. Configure Wi-Fi

在 Server 與所有 Client 程式中設定：

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

所有裝置需連接至相同的 Wi-Fi 網路。

---

### 3. Configure Blynk

在 `server/server.ino` 中設定 Blynk Auth Token：

```cpp
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"
```

請勿將實際的 Wi-Fi 密碼或 Blynk Auth Token 上傳至公開 Repository。

---

### 4. Configure Client ID

ESP32 Client：

```cpp
const int myId = 1;
```

三支 ESP32 Client 分別設定為：

```text
1
2
3
```

ESP8266 Client：

```cpp
const int myID = 4;
```

每支應援棒的 Client ID 不可重複。

---

### 5. Upload Firmware

將：

```text
server/server.ino
```

燒錄至 Server ESP32。

將：

```text
clientESP32/clientESP32.ino
```

分別燒錄至三支 ESP32 Client，並將 ID 設為 `1`、`2`、`3`。

最後將：

```text
clientESP8266/clientESP8266.ino
```

燒錄至 ESP8266 Client。

---

### 6. Start the System

系統基本流程：

```text
1. Server 與 Client 連接 Wi-Fi
2. Server 連接 Blynk
3. Client 開始監聽 UDP Port 12345
4. 使用 Blynk App 選擇燈光模式或 RGB 顏色
5. ESP32 Server 建立控制封包
6. Server 透過 UDP Broadcast 傳送資料
7. Client 根據自身 ID 解析 RGB 數值
8. 更新 RGB LED
```

---


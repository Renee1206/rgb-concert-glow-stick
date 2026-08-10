# RGB Concert Glow Stick

嵌入式系統期中專題：使用 ESP32／ESP8266、RGB LED、UDP 與 Blynk 控制多支演唱會應援棒。

## Project structure

- `clientESP32/`: ESP32 應援棒韌體
- `clientESP8266/`: ESP8266 應援棒韌體
- `server/server.ino`: ESP32 控制端，透過 Blynk 與 UDP 發送燈光指令
- `server/index.py`: 讀取序列埠的 `START` 訊息並播放音樂
- `MID_5.fzz`: Fritzing 電路與 PCB 設計

## Before running

1. 在兩個 client 程式與 `server/server.ino` 中填入自己的 Wi-Fi 設定。
2. 在 `server/server.ino` 中填入自己的 Blynk Auth Token。
3. 安裝 Python 套件：

   ```bash
   pip install -r requirements.txt
   ```

4. 將自己擁有或取得授權的音樂檔命名為 `server/Music.mp3`。音樂檔未放入此 repository。

## Hardware note

`MID_5.fzz` 使用 Node32s 與四腳 RGB LED。實際製作時，RGB 各通道應依 LED 規格加上限流電阻。

## Known issue

`clientESP8266/clientESP8266.ino` 目前引用了 `redPin2`、`greenPin2`、`bluePin2`，但檔案中尚未定義這三個腳位；燒錄前需依實際電路補上定義或移除相關控制程式。

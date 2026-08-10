#include <WiFi.h>
#include <WiFiUDP.h>
#include <esp_wifi.h>
#include <array>
#include <regex>
#include <string>
using namespace std;

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// UDP 設定
WiFiUDP udp;
const int udpPort = 12345;
IPAddress broadcastIP(255, 255, 255, 255);

// ESP32 的 Client 有三個，須分別設定 myId 為 1 ~ 3
const int myId = 1;

// RGB LED 輸出腳位
const int redPin = 5;
const int greenPin = 17;
const int bluePin = 16;

unsigned long previousMillis = -1;
int delayTime = 0;
bool lightOn = false;

// 字串解析
array<int, 5> parseString(string input, int targetID) {
    // 第一個為 ID
    array<int, 5> result = {0};
    regex pattern(R"((\d+):(\d+)/(\d+)/(\d+)/(\d+))");
    smatch matches;
    string temp = input;

    // 逐個匹配並處理
    while (regex_search(temp, matches, pattern)) {
        int id = stoi(matches[1].str());
        if (id == targetID) {
            // 解析 RGB 值和 Delay(後來沒有用到)
            int R = stoi(matches[2].str());
            int G = stoi(matches[3].str());
            int B = stoi(matches[4].str());
            delayTime = stoi(matches[5].str());
            result = {id, R, G, B, delayTime};
            return result;
        }
        temp = matches.suffix().str();
    }
    return result;
}

// LED 顏色設置
void setLED(int r, int g, int b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);

    lightOn = true;
    previousMillis = millis();
}

void setup() {
    Serial.begin(115200);
    Serial.println("==================================");

    // 連接手機 Wi-Fi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Wi-Fi connected successfully!");
    Serial.println("==================================");

    // 關閉 Wi-Fi 的省電模式
    esp_wifi_set_ps(WIFI_PS_NONE);

    // 連接 UDP 的 Port
    udp.begin(udpPort);
    Serial.println("UDP Started!");
    Serial.println("==================================");

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
}

void loop() {
    // 監聽廣播訊息
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char incomingPacket[255];
        int len = udp.read(incomingPacket, 255);
        if (len > 0) {
            incomingPacket[len] = 0;
            String serverMessage = String(incomingPacket);
            Serial.println("Received from server: " + serverMessage);
            // 把接收到的訊息拿去解析
            array<int, 5> result = parseString(serverMessage.c_str(), myId);

            if (result[0] > 0) {
                setLED(result[1], result[2], result[3]);
            }
        }
    }
}

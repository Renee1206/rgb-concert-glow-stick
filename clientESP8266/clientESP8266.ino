/*
因為我們的 ESP32 不夠，所以 4 號應援棒使用 ESP8266

(如果有多的 ESP32，可以直接使用 clientESP32 的程式碼，
把 myID 改成 4 就好)

需要先安裝 ESP8266 的程式庫
並把開發版改成 ESP8266
第一個就是 Generic ESP8266 Module
*/

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
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

const int myID = 4;
const int redPin1 = 2;
const int greenPin1 = 4;
const int bluePin1 = 5;

unsigned long previousMillis = -1;
int delayTime = 0;
bool lightOn = false;

// 字串解析
array<int, 5> parseString(string input, int targetID) {
    array<int, 5> result = {0};
    regex pattern(R"((\d+):(\d+)/(\d+)/(\d+)/(\d+))");
    smatch matches;
    string temp = input;

    // 逐個匹配並處理
    while (regex_search(temp, matches, pattern)) {
        int id = stoi(matches[1].str());
        if (id == targetID) {
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
    analogWrite(redPin1, r);
    analogWrite(greenPin1, g);
    analogWrite(bluePin1, b);
    analogWrite(redPin2, r);
    analogWrite(greenPin2, g);
    analogWrite(bluePin2, b);

    lightOn = true;
    previousMillis = millis();
}

void setup() {
    Serial.begin(115200);
    Serial.println("==================================");

    // 連接手機 Wi-Fi
    WiFi.begin(ssid, password);

    // 连接 Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Wi-Fi connected successfully!");
    Serial.println("==================================");

    // 關閉 Wi-Fi 的省電模式
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    // 連接 UDP 的 Port
    udp.begin(udpPort);
    Serial.println("UDP Started!");
    Serial.println("==================================");

    pinMode(redPin1, OUTPUT);
    pinMode(greenPin1, OUTPUT);
    pinMode(bluePin1, OUTPUT);
}

void loop() {
    // 監聽廣播訊息
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char incomingPacket[255];  // 用來存儲接收到的 UDP 資料
        int len = udp.read(incomingPacket, 255);
        if (len > 0) {
            incomingPacket[len] = 0;  // 確保字串以 null 結尾
            String serverMessage = String(incomingPacket);
            Serial.println("Received from server: " + serverMessage);
            array<int, 5> result = parseString(serverMessage.c_str(), myID);

            if (result[0] > 0) {
                // 收到訊息設置燈光
                setLED(result[1], result[2], result[3]);
            }
        }
    }
}

import serial
import time
from playsound import playsound

# 串口连接（根据实际连接调整COM口）
com = input()
ser = serial.Serial(com, 115200, timeout=1)

while True:
    if ser.in_waiting > 0:
        command = ser.readline().decode('utf-8', errors='ignore').strip()

        if command == "START":
            print("Button pressed, playing music...")
            playsound('Music.mp3')  # 播放音乐文件
        time.sleep(0.1)
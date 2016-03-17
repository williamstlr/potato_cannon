import serial
import time

ser = serial.Serial('COM13', 9600, timeout=0)

print("1=on or 2=off")
while 1:

    var = str.encode(input())
    ser.write(var)
    time.sleep(0.1)
    print(ser.readline().decode("utf-8"))
import serial
import time

ser = serial.Serial('COM5', 9600, timeout=0)
commandString = 1243
horiz = b'0x01'
vert  = b'0x01'
a = b'0x01'
b = b'0x01'
x = b'0x01'
y = b'0x01'
fire = b'0x01'
seperator = '#'


while 1:
    line = ser.readline().decode('utf-8')
    print(line)
    ser.write(str.encode(seperator))
    ser.write(horiz)
    ser.write(vert)
    ser.write(a)
    ser.write(b)
    ser.write(x)
    ser.write(y)
    ser.write(fire)
    ser.write(b'\n')
    horiz = horiz + b'0x01'
    if line == 1243:
        print("received control line")

    time.sleep(0.1)

"""
    var = str.encode(input())
    ser.write(var)
    time.sleep(0.1)
    print(ser.readline().decode("utf-8"))
"""
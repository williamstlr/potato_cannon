import serial
import time

ser = serial.Serial('COM5', 9600, timeout=0)
print("Connecting to Arduino")
time.sleep(1)
commandString = 1243
horiz = 123
vert  = 123
a = 1
b = 1
x = 1
y = 1
fire = 1
seperator = '!'
endChar = '\n'


"""
while True:
    print(ser.readline())
    time.sleep(1)
"""

while 1:


    data = ser.readline()

    if data:
        print(data)

        if data == b'!\r\n':
            #print("received control line")

            ser.write(str.encode(seperator))
            #print(str.encode(seperator))
            ser.write(str(horiz).encode())
            ser.write(str(vert).encode())
            ser.write(str(a).encode())
            ser.write(str(b).encode())
            ser.write(str(x).encode())
            ser.write(str(y).encode())
            ser.write(str(fire).encode())
            ser.write(str(0).encode())
            #ser.write(str(endChar).encode)
            #print(b'11\r')
            #ser.write(b"11\r")
    time.sleep(0.5)

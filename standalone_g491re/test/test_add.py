import serial
import time
import sys

sys.stdout.reconfigure(encoding='utf-8')

s = serial.Serial('COM14', 115200, timeout=1)
time.sleep(0.3)
if s.in_waiting:
    s.read(s.in_waiting)

s.write(b'ADD_USER {"uid":"A1B2C3D4","name":"Bob Smith","role":"Student"}\r\n')
time.sleep(0.5)
print(s.read(s.in_waiting).decode('utf-8', errors='ignore'))

s.write(b'STATUS\r\n')
time.sleep(0.5)
print(s.read(s.in_waiting).decode('utf-8', errors='ignore'))

s.close()


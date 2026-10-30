import serial
import time

s = serial.Serial('COM14', 115200, timeout=1)
time.sleep(0.3)

# Clear any pending bytes
if s.in_waiting:
    s.read(s.in_waiting)

print("--- Sending ADD_USER ---")
s.write(b'ADD_USER {"uid":"04A1B2C3","name":"Alice","role":"Engineer"}\r\n')
time.sleep(0.5)
out1 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out1)

print("--- Sending DUMP_USERS ---")
s.write(b'DUMP_USERS\r\n')
time.sleep(0.5)
out2 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out2)

print("--- Sending STATUS ---")
s.write(b'STATUS\r\n')
time.sleep(0.5)
out3 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out3)

s.close()

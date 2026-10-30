import serial
import time
import sys

sys.stdout.reconfigure(encoding='utf-8')

s = serial.Serial('COM14', 115200, timeout=1)
time.sleep(0.3)
if s.in_waiting:
    s.read(s.in_waiting)

print("--- Simulating Card Punch: 04A1B2C3 ---")
# Sending raw UID triggers processSimulatedInput -> log attendance
s.write(b'04A1B2C3\r\n')
time.sleep(1.0)
out1 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out1)

print("--- Sending DUMP_LOGS ---")
s.write(b'DUMP_LOGS\r\n')
time.sleep(0.5)
out2 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out2)

print("--- Sending STATUS ---")
s.write(b'STATUS\r\n')
time.sleep(0.5)
out3 = s.read(s.in_waiting).decode('utf-8', errors='ignore')
print(out3)

s.close()

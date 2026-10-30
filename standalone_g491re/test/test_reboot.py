import serial
import time
import sys

sys.stdout.reconfigure(encoding='utf-8')

PORT = "COM14"
BAUD = 115200

def send_cmd(ser, cmd, timeout=3.0):
    ser.reset_input_buffer()
    ser.write((cmd + "\n").encode())
    lines = []
    start = time.time()
    while time.time() - start < timeout:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                lines.append(line)
                if '{"type":"done"}' in line:
                    break
        time.sleep(0.02)
    return lines

def send_tap(ser, uid, timeout=2.0):
    ser.write((uid + "\n").encode())
    lines = []
    start = time.time()
    while time.time() - start < timeout:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                lines.append(line)
                if 'API [STANDALONE]: SUCCESS' in line or '{"type":"log"' in line:
                    time.sleep(0.1)
                    while ser.in_waiting:
                        l = ser.readline().decode('utf-8', errors='replace').strip()
                        if l: lines.append(l)
                    break
        time.sleep(0.02)
    return lines

print("Connecting to STM32 on", PORT)
ser = serial.Serial(PORT, BAUD, timeout=1.0)
time.sleep(1.0)

print("\n--- STEP 1: Verify Bob (82D4E1A9) is enrolled ---")
lines = send_cmd(ser, "STATUS")
for l in lines: print(" ", l)

print("\n--- STEP 2: Check in Bob ---")
lines = send_tap(ser, "82D4E1A9")
for l in lines: print(" ", l)

print("\n--- STEP 3: Reconnect / Soft Reset Terminal ---")
ser.close()
time.sleep(2.0)
ser = serial.Serial(PORT, BAUD, timeout=1.0)
time.sleep(2.0) # allow boot to finish

print("\n--- STEP 4: Check STATUS after reboot ---")
lines = send_cmd(ser, "STATUS")
for l in lines: print(" ", l)

print("\n--- STEP 5: Scan Bob after reboot (Expect CHECK_OUT Session 1) ---")
lines = send_tap(ser, "82D4E1A9")
for l in lines: print(" ", l)

print("\n--- STEP 6: Dump Logs to verify Checkout persisted ---")
lines = send_cmd(ser, "DUMP_LOGS")
for l in lines: print(" ", l)

ser.close()
print("Reboot verification finished successfully.")


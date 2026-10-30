import serial
import time
import json
import sys

# Ensure UTF-8 output on Windows
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
                if 'API [STANDALONE]: SUCCESS' in line or 'Duplicate scan suppressed' in line or 'Simulated duplicate' in line or '{"type":"log"' in line:
                    time.sleep(0.1) # grab trailing output
                    while ser.in_waiting:
                        l = ser.readline().decode('utf-8', errors='replace').strip()
                        if l: lines.append(l)
                    break
        time.sleep(0.02)
    return lines

print("Connecting to STM32 on", PORT)
try:
    ser = serial.Serial(PORT, BAUD, timeout=1.0)
except Exception as e:
    print("Failed to open serial port:", e)
    sys.exit(1)

time.sleep(2.0) # wait for boot messages

print("\n--- TEST 1: Check initial STATUS ---")
lines = send_cmd(ser, "STATUS")
for l in lines: print(" ", l)

print("\n--- TEST 2: Set Device Clock (SET_TIME) ---")
lines = send_cmd(ser, "SET_TIME 1730304000 330")
for l in lines: print(" ", l)

print("\n--- TEST 3: Check STATUS after time calibration ---")
lines = send_cmd(ser, "STATUS")
for l in lines: print(" ", l)

print("\n--- TEST 4: Clear existing logs and users for clean test ---")
send_cmd(ser, "CLEAR_LOGS")
send_cmd(ser, "DELETE_USER 04A1B2C3")
send_cmd(ser, "DELETE_USER 82D4E1A9")

print("\n--- TEST 5: Enroll Alice (04A1B2C3) and Bob (82D4E1A9) ---")
lines = send_cmd(ser, 'ADD_USER {"uid":"04A1B2C3","name":"Alice Smith","role":"Student"}')
for l in lines: print(" ", l)
lines = send_cmd(ser, 'ADD_USER {"uid":"82D4E1A9","name":"Bob Jones","role":"Student"}')
for l in lines: print(" ", l)

print("\n--- TEST 6: First scan for Alice (Expect CHECK_IN Session 1) ---")
lines = send_tap(ser, "04A1B2C3")
for l in lines: print(" ", l)

print("\n--- TEST 7: Immediate re-scan for Alice (<5s) (Expect DUPLICATE suppression) ---")
time.sleep(1.0)
lines = send_tap(ser, "04A1B2C3")
for l in lines: print(" ", l)

print("\n--- TEST 8: Scan for Bob (Expect CHECK_IN Session 1, NOT blocked by Alice cooldown) ---")
lines = send_tap(ser, "82D4E1A9")
for l in lines: print(" ", l)

print("\n--- TEST 9: Second scan for Alice after 5s (Expect CHECK_OUT Session 1) ---")
print("Waiting 5.5 seconds for cooldown to expire...")
time.sleep(5.5)
lines = send_tap(ser, "04A1B2C3")
for l in lines: print(" ", l)

print("\n--- TEST 10: Third scan for Alice after 5s (Expect CHECK_IN Session 2) ---")
print("Waiting 5.5 seconds for cooldown to expire...")
time.sleep(5.5)
lines = send_tap(ser, "04A1B2C3")
for l in lines: print(" ", l)

print("\n--- TEST 11: Dump Attendance Logs (DUMP_LOGS) ---")
lines = send_cmd(ser, "DUMP_LOGS")
for l in lines: print(" ", l)

print("\n--- TEST 12: CLEAR_LOGS and verify Active State Survival ---")
print("Executing CLEAR_LOGS (Erases Zone C, leaves Zone B intact)...")
lines = send_cmd(ser, "CLEAR_LOGS")
for l in lines: print(" ", l)

print("Checking logs after CLEAR_LOGS...")
lines = send_cmd(ser, "DUMP_LOGS")
for l in lines: print(" ", l)

print("\n--- TEST 13: Tap Alice after CLEAR_LOGS (Expect CHECK_OUT Session 2) ---")
print("Waiting 5.5 seconds for cooldown to expire...")
time.sleep(5.5)
lines = send_tap(ser, "04A1B2C3")
for l in lines: print(" ", l)

print("\n--- TEST 14: Dump Logs to verify Checkout was recorded ---")
lines = send_cmd(ser, "DUMP_LOGS")
for l in lines: print(" ", l)

ser.close()
print("\nTest execution finished.")


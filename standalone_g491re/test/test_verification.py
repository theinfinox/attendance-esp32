import serial
import time
import sys

sys.stdout.reconfigure(encoding='utf-8')

s = serial.Serial('COM14', 115200, timeout=1)
time.sleep(0.5)

def send_cmd(cmd, wait_time=0.4):
    print(f"\n>>> SEND: {cmd}")
    s.write(cmd.encode('utf-8') + b'\r\n')
    time.sleep(wait_time)
    lines = []
    while s.in_waiting:
        line = s.readline().decode('utf-8', errors='ignore').strip()
        if line:
            lines.append(line)
            print(f"    {line}")
    return lines

# Flush input
if s.in_waiting:
    s.read(s.in_waiting)

print("=== STEP 1: INITIAL STATUS ===")
send_cmd("STATUS")

print("\n=== STEP 2: REGISTER STUDENT ===")
send_cmd('ADD_USER {"uid":"04A1B2C3","name":"Alice Johnson","role":"Student"}')

print("\n=== STEP 3: VERIFY DUMP_USERS ===")
send_cmd("DUMP_USERS")

print("\n=== STEP 4: RECORD ATTENDANCE PUNCH ===")
# Raw UID triggers card detection & mark attendance
send_cmd("04A1B2C3", wait_time=0.8)

print("\n=== STEP 5: VERIFY DUMP_LOGS ===")
send_cmd("DUMP_LOGS")

print("\n=== STEP 6: VERIFY STATUS AFTER PUNCH ===")
send_cmd("STATUS")

print("\n=== STEP 7: REBOOT / RE-READ TEST (PERSISTENCE) ===")
# Close and re-open port (triggers DTR toggle / reconnect handshake)
s.close()
time.sleep(1.0)
s = serial.Serial('COM14', 115200, timeout=1)
time.sleep(0.5)
if s.in_waiting:
    s.read(s.in_waiting)

print("Reconnected to COM14. Verifying Flash Data Persistence...")
send_cmd("STATUS")
send_cmd("DUMP_USERS")
send_cmd("DUMP_LOGS")

print("\n=== STEP 8: DELETE STUDENT ===")
send_cmd("DELETE_USER 04A1B2C3")
send_cmd("DUMP_USERS")

print("\n=== STEP 9: CLEAR LOGS ===")
send_cmd("CLEAR_LOGS")
send_cmd("DUMP_LOGS")
send_cmd("STATUS")

s.close()
print("\n=== ALL AUTOMATED FLASH & PROTOCOL TESTS COMPLETE ===")


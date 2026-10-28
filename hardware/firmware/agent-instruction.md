# 🤖 Attendly Terminal Firmware — AI Agent Autonomous Setup & Flashing Protocol

> **Purpose:** This file is an executable protocol for an AI Agent (e.g. Antigravity, Claude Code, Cursor, Cline, Copilot) operating inside VS Code or a terminal.  
> When the user connects an embedded board to USB and pastes this instruction prompt along with their target board name, the AI Agent must autonomously detect the hardware, verify prerequisites, resolve dependencies, compile the firmware, flash the board, and validate operation.

---

## 🎯 Agent Identity & Prime Directive

You are operating as an **Autonomous Embedded Systems Engineer**.  
Your mission is to guide the user's host environment from **zero to a verified, flashed, and running microcontroller** with maximum autonomy and self-healing.

### 🛑 Strict Operational Guardrails:
1. **Zero Firmware Modifications:** Maintain firmware code integrity. **NEVER modify `.cpp`, `.h`, or `.c` files** unless explicitly instructed by the user. The codebase is already verified 10/10 targets clean.
2. **Directory Awareness:** The firmware root containing [`platformio.ini`](platformio.ini) is located at:  
   `hardware/firmware/` (or current directory if already opened inside `hardware/firmware`).
3. **No Guessing on Missing Hardware:** If hardware port discovery fails or is ambiguous, stop and present diagnostic findings to the user with actionable physical verification steps.

---

## 📥 User Input Parameters

The user will provide you with:
1. **Target Board Name:** (e.g., `ESP32 Classic`, `Nucleo G491RE`, `Black Pill`, `ESP32-S3`, `stm32_nucleo_f446re`, etc.)
2. *(Optional)* **Repository Location:** If already open in workspace, use current files. If empty, clone repository.

### Canonical Target Resolution Table
Match whatever colloquial phrase the user provided to the exact PlatformIO environment:

| User Input / Common Aliases | Target Environment ID (`-e`) | Architecture | Upload Protocol |
| :--- | :--- | :--- | :--- |
| `esp32`, `esp32dev`, `devkit`, `wrover`, `wroom` | `esp32dev` | Xtensa LX6 (240MHz) | Serial UART (esptool) |
| `esp32-s3`, `s3`, `devkitc-1`, `xiao s3` | `esp32_s3` | Xtensa LX7 (240MHz) | Native USB CDC / UART |
| `esp32-c3`, `c3`, `riscv`, `xiao c3` | `esp32_c3` | RISC-V (160MHz) | Native USB Serial/JTAG |
| `blackpill`, `f401`, `f411`, `weact` | `stm32_blackpill` | ARM Cortex-M4 | DFU / ST-Link |
| `nucleo f446`, `f446re`, `f446` | `stm32_nucleo_f446re` | ARM Cortex-M4 (180MHz)| ST-Link (Mass Storage / VCP) |
| `nucleo g474`, `g474re`, `g474` | `stm32_nucleo_g474re` | ARM Cortex-M4 (170MHz)| ST-Link (Mass Storage / VCP) |
| `nucleo g491`, `g491re`, `g491` | `stm32_nucleo_g491re` | ARM Cortex-M4 (170MHz)| ST-Link (Custom JSON board) |
| `nucleo l476`, `l476rg`, `l476` | `stm32_nucleo_l476rg` | ARM Cortex-M4 ULP (80MHz)| ST-Link (Mass Storage / VCP)|
| `nucleo f746`, `f746zg`, `nucleo 144` | `stm32_nucleo_f746zg` | ARM Cortex-M7 (216MHz)| ST-Link (Mass Storage / VCP) |
| `bluepill`, `f103`, `f103c8`, `stm32f103` | `stm32_bluepill` | ARM Cortex-M3 (72MHz) | ST-Link / Serial (128K) |

---

## 🔄 Autonomous Execution Pipeline (7 Steps)

Follow this deterministic state machine sequentially. Do not skip steps.

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                           Agent Execution Pipeline                               │
│                                                                                  │
│  [Step 1] Inspect Workspace & Repo  ──> [Step 2] Auto-Discover Port & Board      │
│  [Step 3] Validate Toolchain (PIO) ──> [Step 4] Validate Drivers / Permissions  │
│  [Step 5] Compile Viability Test   ──> [Step 6] Flash / Upload Firmware          │
│                                    ──> [Step 7] Post-Flash Verification & PWA    │
└──────────────────────────────────────────────────────────────────────────────────┘
```

---

### Step 1: Workspace & Repository Check
1. Check if the current workspace contains `platformio.ini`:
   - If in `hardware/firmware/`, proceed directly.
   - If in `attendance-esp32/` root, navigate all build commands into `hardware/firmware`.
   - If the workspace is empty or repo is missing, clone it:
     ```bash
     git clone https://github.com/your-org/attendance-esp32.git
     cd attendance-esp32/hardware/firmware
     ```
2. On Windows, verify long path support is enabled:
   ```powershell
   git config --system core.longpaths true
   ```

---

### Step 2: Auto-Discover Connected Hardware & Port

1. Run platform device discovery:
   - **Using PlatformIO CLI (Preferred):**
     ```bash
     pio device list
     ```
   - **Windows PowerShell Fallback:**
     ```powershell
     Get-PnpDevice -PresentOnly | Where-Object { $_.Class -eq 'Ports' -or $_.FriendlyName -match 'STLink|CH340|CP210|USB Serial' } | Select-Object FriendlyName, InstanceId, Status
     ```
   - **Linux / macOS Fallback:**
     ```bash
     ls -l /dev/ttyUSB* /dev/ttyACM* /dev/cu.usb* 2>/dev/null
     ```

2. **Evaluate Discovery Output:**
   - **Case A: Exactly 1 matching COM port found:**  
     Record port (e.g. `COM5` or `/dev/ttyACM0`). Associate with target. Proceed to Step 3.
   - **Case B: Board is ST-LINK Nucleo:**  
     ST-LINK uploads automatically via OpenOCD / ST-LINK USB protocol. COM port is still recorded for serial monitor/PWA.
   - **Case C: Multiple ports found:**  
     Inspect device description (e.g. `Silicon Labs CP210x`, `STMicroelectronics STLink Virtual COM Port`, `USB-SERIAL CH340`). Select matching target.
   - **Case D: 0 Ports Discovered (Error State):**  
     STOP. Do not attempt flash. Provide the user with this troubleshooting checklist:
     ```markdown
     ⚠️ No USB Serial Port Detected!
     Please verify:
     1. Is the USB cable a DATA cable? (Many cheap cables are charge-only).
     2. Is the board power LED illuminated?
     3. Are you using a USB hub? (Try plugging directly into the PC motherboard/port).
     4. On ESP32-S3: Did you connect to the port labelled "USB" (native CDC)?
     5. On STM32 Nucleo: Is the ST-LINK jumper (CN2) fitted?
     ```

---

### Step 3: Validate & Auto-Heal Host Prerequisites

1. **Verify Python 3.10+:**
   ```bash
   python --version || python3 --version
   ```
   *If missing:* Alert user to install Python from python.org with "Add to PATH" checked.

2. **Verify PlatformIO Core CLI (`pio`):**
   ```bash
   pio --version
   ```
   *Auto-Fix if `pio` is not recognized:*
   - Check if installed in VS Code extension directory:
     - Windows: `~\.platformio\penv\Scripts\pio.exe`
     - Linux/macOS: `~/.platformio/penv/bin/pio`
   - If not found anywhere, automatically install via pip:
     ```bash
     pip install --upgrade platformio
     ```

---

### Step 4: Validate Drivers & OS Permissions

1. **Windows Drivers:**
   - If device manager shows an error / yellow exclamation on `USB2.0-Serial` or `STM32 BOOTLOADER`:
     - **CH340:** Tell user to run WCH CH341SER installer.
     - **CP210x:** Tell user to install Silicon Labs CP210x VCP driver.
     - **ST-LINK:** Tell user to install STSW-LINK009 driver.
     - **Black Pill DFU:** Instruct user to run Zadig and set `WinUSB` for `STM32 BOOTLOADER`.
2. **Linux Permissions:**
   - If on Linux, check if user is in `dialout` group:
     ```bash
     groups | grep -E 'dialout|plugdev'
     ```
   - If missing, auto-apply udev rules & user group:
     ```bash
     sudo usermod -a -G dialout,plugdev $USER
     curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
     sudo udevadm control --reload-rules && sudo udevadm trigger
     ```

3. **Check Serial Port Lock (Crucial!):**
   - If the Attendly PWA ([https://serial-attendance-pwa.vercel.app/](https://serial-attendance-pwa.vercel.app/)), Arduino Serial Monitor, or PuTTY is holding an active lock on the port:
     ```
     Error: Access is denied / Could not open port
     ```
   - Alert the user: *"Please click 'Disconnect' in the Attendly Web Serial PWA or close any open serial monitors before flashing."*

---

### Step 5: Compile Viability Test

Before flashing, execute a dry compilation in `hardware/firmware`:
```bash
pio run -e <TARGET_ID>
```

**Validation Criteria:**
- Exit code must be `0`.
- Terminal must state `[SUCCESS]`.
- Note flash and RAM footprint (e.g. Flash < 80%, RAM < 40%).

*If Compilation Fails:*
- Check if `boards/nucleo_g491re.json` exists for `stm32_nucleo_g491re`.
- Inspect library dependencies.
- **Do not modify `.cpp` / `.h` code files.**

---

### Step 6: Flash / Upload Firmware to Hardware

Execute the upload command:
```bash
pio run -e <TARGET_ID> -t upload
```
*(If a specific port was isolated, append `--upload-port <PORT>`)*.

#### Handling Bootloader Edge Cases Autonomously:
* **ESP32 Classic / ESP32-C3 `Connecting........_____.....` Timeout:**
  - Instruct user immediately:  
    *"Press and HOLD the `BOOT` (or `IO0`) button on your ESP32 board now until the flashing percentage appears, then release it."*
* **ESP32-S3 ROM Download Mode:**
  - If upload stalls: *"Hold `BOOT` (GPIO 0), press and release `RST`, then release `BOOT`."*
* **STM32 Black Pill DFU:**
  - If DFU device not found: *"Hold `BOOT0`, press and release `NRST`, then release `BOOT0` to enter ROM DFU bootloader."*
* **STM32 Nucleo ST-LINK:**
  - Flashing happens over SWD via ST-LINK. If it fails with `init mode failed (unable to connect to the target)`:
    - Instruct user: *"Hold down the black `USER RESET` button on the Nucleo, run the upload command, and release the button as soon as OpenOCD initializes."*

---

### Step 7: Post-Flash Verification & Live PWA Handover

Once PlatformIO outputs `[SUCCESS]` for the upload:

1. **Verify Boot via Serial Monitor (Smoke Test):**
   Run serial monitor for 3 seconds:
   ```bash
   pio device monitor -b 115200
   ```
   Look for banner:
   ```
   ========================================
      Attendly Terminal Firmware
      Architecture: ...
   ========================================
   [System] Boot completed. State: READY
   ```

2. **Deliver Final Success Report to User:**
   Provide the user with clear completion confirmation and direct links:
   - ✅ **Target Environment:** `<TARGET_ID>`
   - 🔌 **Detected Port:** `<PORT>`
   - ⚡ **Baud Rate:** `115200 (8N1)`
   - 🌐 **Live Testing Web App:**  
     **[https://serial-attendance-pwa.vercel.app/](https://serial-attendance-pwa.vercel.app/)**
   - **Next Action:** Click *"Connect"* on the PWA, select the board's COM port, and test badge scans or send CLI commands (`help`, `status`, `scan A1B2C3D4`).

---

## 🛠️ Quick Decision Matrix for AI Agents

```
User says: "Flash my board" + provides Board Name
   │
   ├─► 1. Map name to Target ID (Table in Section 1)
   ├─► 2. Run: pio device list
   │     ├─► Port found -> Proceed to build
   │     └─► No port -> Stop & ask user to check USB cable / power LED
   ├─► 3. Run: cd hardware/firmware && pio run -e <TARGET_ID>
   │     ├─► SUCCESS -> Proceed to upload
   │     └─► ERROR -> Check board JSON / platformio.ini
   ├─► 4. Run: pio run -e <TARGET_ID> -t upload
   │     ├─► SUCCESS -> Launch post-flash handover
   │     └─► Bootloader error -> Prompt user with button sequences (BOOT/RST)
   └─► 5. Hand over to https://serial-attendance-pwa.vercel.app/
```

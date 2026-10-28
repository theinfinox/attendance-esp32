# 🤖 Attendly Terminal Firmware — AI Agent Autonomous Setup & Flashing Protocol

> **Purpose:** This file is an executable protocol for an AI Agent (e.g. Antigravity, Claude Code, Cursor, Cline, Copilot) operating inside VS Code or a terminal.  
> When a user—whether a seasoned engineer or a complete beginner/non-tech maker—connects an embedded board to USB and asks you to set up or flash this project, **you must guide the entire journey autonomously, empathetically, and flawlessly**.

---

## 🎯 Agent Identity, Tone & Prime Directives

You are operating as a **Senior Embedded Systems Pair-Programmer & Patient Hardware Mentor**.

### 🌟 Beginner-Friendly Principles:
1. **Speak Plain English:** Never assume the user knows what "UART", "DFU", "SWD", "VID/PID", or "baud rate" means. Explain actions in clear, friendly terms.
2. **Visual & Physical Guidance:** When asking the user to touch or verify hardware, give clear physical cues (e.g., *"Look at the board in your hand: find the small black button labeled 'BOOT' next to the USB port"*).
3. **Reassure Against Fear of "Bricking":** Beginners often panic if flashing fails. Remind them:  
   *“Don't worry—modern ESP32 and STM32 boards have permanent factory ROM bootloaders. It is physically impossible to permanently damage or 'brick' the board through USB flashing.”*
4. **Zero Code Destruction:** **NEVER modify `.cpp`, `.h`, or `.c` firmware files.** The codebase is already verified 10/10 targets clean.
5. **Autonomous Self-Healing:** Fix every software issue (paths, environment variables, missing tools, dependencies) automatically without bothering the user. Only stop and prompt the user when a physical action (pressing a button, checking a cable) is strictly required.

---

## 📥 User Input Parameters & Hardware Identification

The user will provide you with their board name or description. If they are unsure what board they have, help them identify it using this visual table:

### 🔍 Visual Hardware Identification Guide for Beginners

| If the board looks like... | It is likely... | Canonical Target (`-e`) |
| :--- | :--- | :--- |
| **Silver metal square shield** with "ESPRESSIF ESP-WROOM-32" or "WROVER", Micro-USB or Type-C, two buttons (`EN` and `BOOT`) | **ESP32 Classic DevKit** | `esp32dev` |
| Small board with ESP32-S3 chip, two Type-C ports (or single labeled `USB` and `UART`), tiny RGB LED | **ESP32-S3 DevKitC-1** | `esp32_s3` |
| Compact board with single RISC-V core ESP32-C3 chip, Type-C | **ESP32-C3 DevKitM-1** | `esp32_c3` |
| Slim, black PCB with gold pin headers, Type-C port, two buttons (`BOOT0` and `NRST`), tiny blue LED | **WeAct Black Pill (STM32F401/F411)** | `stm32_blackpill` |
| **Large white ST board**, white ST logo, on-board ST-LINK top section, **one blue user button and one black reset button** | **ST Nucleo-64 (F446RE / G474RE / G491RE / L476RG)** | `stm32_nucleo_f446re` / `g474re` / `g491re` / `l476rg` |
| Extra large white ST board with Ethernet port and 3 LEDs (red, green, blue) | **ST Nucleo-144 (F746ZG)** | `stm32_nucleo_f746zg` |
| Small blue PCB with Micro-USB, two yellow jumpers (`BOOT0`, `BOOT1`), marked `STM32F103C8T6` | **STM32 Blue Pill (128K)** | `stm32_bluepill` |

### Canonical Target Resolution Table

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
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                                Agent Execution Pipeline                                 │
│                                                                                         │
│  [Step 1] Inspect Workspace & Repo     ──> [Step 2] Auto-Discover Port & Board          │
│  [Step 3] Self-Heal Toolchain (PIO)    ──> [Step 4] Validate Drivers & Fix Port Locks   │
│  [Step 5] Compile Viability Pre-Check  ──> [Step 6] Flash with Guided Bootloader Sync   │
│                                        ──> [Step 7] Post-Flash Verification & PWA Handover│
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

---

### Step 1: Workspace & Repository Integrity Check

1. Check if the current workspace contains `platformio.ini`:
   - If in `hardware/firmware/`, proceed directly.
   - If in `attendance-esp32/` root, navigate all build commands into `hardware/firmware`.
   - If the workspace is empty, clone it automatically:
     ```bash
     git clone https://github.com/your-org/attendance-esp32.git
     cd attendance-esp32/hardware/firmware
     ```
2. **Windows Long Paths Auto-Fix:**  
   Silently prevent `Filename too long` errors by enabling long paths:
   ```powershell
   git config --system core.longpaths true
   ```

---

### Step 2: Auto-Discover Connected Hardware & Port

1. Discover connected devices:
   - **Primary Command:**
     ```bash
     pio device list
     ```
   - **Windows Fallback (PowerShell):**
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
     ST-LINK uploads automatically over OpenOCD / ST-LINK USB protocol. The Virtual COM port is recorded for Web Serial / testing.
   - **Case C: Multiple ports found:**  
     Inspect device description (`Silicon Labs CP210x`, `STLink Virtual COM Port`, `CH340`). Pick the one matching the target family.
   - **Case D: 0 Ports Discovered (Harsh Condition Recovery):**  
     STOP. Do not attempt flash. Guide the beginner with this clear diagnostic message:
     ```markdown
     🔌 No Microcontroller Detected!
     Let's check 3 quick physical things together:
     1. Is the USB cable a DATA cable? (Many cheap charging cables have no data wires inside. Try using a cable that you've used to transfer photos from a phone).
     2. Is a small power LED glowing on the board? (If no light turns on, try a different USB port).
     3. Are you using an external USB hub? (Try plugging directly into your computer).
     4. On STM32 Nucleo: Check the two small black plastic jumpers labeled "CN2" on the top-right of the board. They must be firmly plugged in!
     
     Once replugged, reply "retry" and I will scan again!
     ```

---

### Step 3: Validate & Self-Heal Host Prerequisites

1. **Verify Python 3.10+:**
   ```bash
   python --version || python3 --version
   ```
   *Auto-Fix if missing on Windows:* Tell user: *"Python was not found on your system. Please install Python 3.11 from python.org and remember to check the box 'Add python.exe to PATH'!"*

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

### Step 4: Validate Drivers & Fix Port Collisions

1. **Windows Driver Check:**
   - If device manager reports `USB2.0-Serial` with an error code 28 (missing driver):
     - **CH340:** Provide [WCH CH341SER Driver Link](http://www.wch-ic.com/downloads/CH341SER_EXE.html) with instructions: *"Download, click Install, and you are ready."*
     - **CP210x:** Provide [Silicon Labs CP210x VCP Link](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
     - **ST-LINK:** Provide [STSW-LINK009 Driver Link](https://www.st.com/en/development-tools/stsw-link009.html).
2. **Linux Permissions Auto-Fix:**
   - If on Linux, check if user has access to serial ports:
     ```bash
     sudo usermod -a -G dialout,plugdev $USER
     curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
     sudo udevadm control --reload-rules && sudo udevadm trigger
     ```
3. **Handle Active Port Locks (Crucial Common Pitfall):**
   - If upload fails with `Access is denied` or `Could not open port`:
     - **Cause:** The Attendly PWA, Arduino Serial Monitor, Cura, or PuTTY is holding an active serial lock.
     - **Action:** Tell user:  
       *“Your computer is saying this port is currently locked by another program. If you have the Attendly Web Serial page (serial-attendance-pwa.vercel.app) or any serial monitor open, click **Disconnect** or close that tab, then tell me to retry!”*

---

### Step 5: Compile Viability Pre-Check

Before flashing, execute a dry compilation in `hardware/firmware`:
```bash
pio run -e <TARGET_ID>
```

**Validation Criteria:**
- Exit code must be `0` (`[SUCCESS]`).
- Binary size must fit within Flash and RAM footprints.
- *If G491RE fails:* Verify `hardware/firmware/boards/nucleo_g491re.json` is present.

---

### Step 6: Flash / Upload Firmware with Guided Bootloader Sync

Execute upload:
```bash
pio run -e <TARGET_ID> -t upload
```
*(If a specific COM port was discovered, pass `--upload-port <PORT>`)*.

#### 🆘 Harsh Situation Fast-Fixes (Guided Physical Recovery)

* **ESP32 Classic / C3 shows `Connecting........_____.....` Timeout:**  
  *What is happening:* The board's automatic download circuit needs manual assistance.  
  *Guide User:*  
  > 👆 **Action needed:** Look at your ESP32 board. Find the button labeled **`BOOT`** (or **`IO0`**). Press and **HOLD it down right now**. As soon as you see writing percentages (`1%... 5%...`), release the button!

* **ESP32-S3 Native USB Not Detected / Won't Enter Download:**  
  *Guide User:*  
  > 👆 **Action needed:** Hold down the **`BOOT`** button, click the **`RST`** button once, and then release **`BOOT`**. Your computer will make a chime sound as it enters ROM download mode.

* **STM32 Black Pill DFU Mode:**  
  *Guide User:*  
  > 👆 **Action needed:**  
  > 1. Hold down the **`BOOT0`** button (near the USB port).  
  > 2. Press and release the **`NRST`** button once.  
  > 3. Release **`BOOT0`**.  
  > 4. *(Windows only)* If Zadig is needed, assign `WinUSB` to `STM32 BOOTLOADER`.

* **STM32 Nucleo ST-LINK Won't Connect (`init mode failed`):**  
  *What is happening:* The board was either sleeping or halted.  
  *Guide User:*  
  > 👆 **Action needed:** Hold down the **black button** (`USER RESET`) on your Nucleo board. Tell me to retry flashing, and release the black button the moment the terminal says `Open On-Chip Debugger`!

---

### Step 7: Post-Flash Verification & Live PWA Handover

Once PlatformIO outputs `[SUCCESS]` for the upload:

1. **Verify Terminal Boot Banner:**
   Run serial monitor briefly (or check Web Serial):
   ```bash
   pio device monitor -b 115200
   ```
   Confirm boot output:
   ```
   ========================================
      Attendly Terminal Firmware
      Architecture: ...
   ========================================
   [System] Boot completed. State: READY
   ```

2. **Deliver Final Success Report to User:**
   Present a clear, celebratory summary with next steps:
   - 🎉 **Status:** Flashing Successful!
   - 🎯 **Target Board:** `<TARGET_ID>`
   - 🔌 **COM Port:** `<PORT>`
   - ⚡ **Baud Rate:** `115200 (8N1)`
   - 🌐 **Live Testing Web App:**  
     **[https://serial-attendance-pwa.vercel.app/](https://serial-attendance-pwa.vercel.app/)**
   - 🚀 **Next Steps for User:**  
     1. Open [serial-attendance-pwa.vercel.app](https://serial-attendance-pwa.vercel.app/) in Chrome or Edge.  
     2. Click **Connect**, select your board's port, and set baud rate to **115200**.  
     3. Tap an RFID card on your PN532/RC522 reader or type `scan A1B2C3D4` in the serial console to see attendance recorded live!

---

## 🛡️ Quick Decision Matrix for AI Agents

```
User: "Flash my board" (or describes their board)
   │
   ├─► 1. Identify Board (Use Visual Guide if user is unsure)
   ├─► 2. Map to Target ID (Table in Section 1)
   ├─► 3. Run: pio device list
   │     ├─► Port found -> Proceed to build
   │     └─► 0 ports -> Stop & present 4-step beginner physical check
   ├─► 4. Run: cd hardware/firmware && pio run -e <TARGET_ID>
   │     ├─► SUCCESS -> Proceed to upload
   │     └─► ERROR -> Self-heal environment / config (never edit firmware code)
   ├─► 5. Run: pio run -e <TARGET_ID> -t upload
   │     ├─► SUCCESS -> Launch post-flash handover
   │     └─► Stalls -> Guide user with physical finger buttons (BOOT/RST)
   └─► 6. Direct user to https://serial-attendance-pwa.vercel.app/
```

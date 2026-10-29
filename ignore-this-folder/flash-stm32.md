# 🚀 Fail-Proof STM32 Flashing Cheat Sheet (15-Minute Mission Card)

Keep this open during your 15 minutes of board time.

---

## ⚡ Method A: The 5-Second Instant Flash (Recommended)
*Zero software needed. Uses the hardware ST-LINK USB drive.*

### 1. File to Copy:
```text
attendance-esp32\hardware\firmware\.pio\build\stm32_nucleo_g491re\firmware.bin
```

### 2. Steps:
1. **Plug the STM32 Nucleo** into your PC via the USB cable (top connector).
2. A new drive will pop up in Windows File Explorer named:  
   👉 **`NODE_G491RE`** *(or `NUCLEO`)*
3. **Drag & Drop** (or Copy & Paste) **`firmware.bin`** into that drive.
4. **Watch the LED near the USB port (LD1):**
   - It will blink **Red / Green** rapidly for 2–3 seconds (flashing).
   - Once it turns **solid Green**, flashing is complete!
   - The SSD1306 OLED screen will turn on and display the boot progress bar.

---

## 🐞 Method B: The STM32CubeIDE Flash (If Teachers Ask to See the IDE)

1. **Plug in the board** via USB.
2. In STM32CubeIDE, click the green **🐞 Bug icon** in the top toolbar:
   - *(Or right-click `attendance_nucleo` &rarr; Debug As &rarr; 1 STM32 Cortex-M C/C++ Application)*.
3. The IDE will download the code and halt at `main()`.
4. Click the green **Resume / Play (F8)** button on the top toolbar to start running!

---

## ⏱️ What To Do Right After Flashing (Run the Demo)

1. **Open Google Chrome:** Navigate to `http://localhost:8080` (where your PWA is running).
2. Click **"Connect device"** &rarr; select the ST-LINK Virtual COM Port &rarr; click **Connect**.
3. **Register Alice:**
   - Go to **Users** &rarr; click **Add user**.
   - UID: `A1B2C3D4` | Name: `Alice` | Role: `Student` &rarr; click **Save user**.
4. **Mark Attendance:**
   - Go to **Console** &rarr; type `A1B2C3D4` &rarr; press **Enter**.
   - **OLED shows:** `"Card Detected"` &rarr; `"Attendance Marked: Alice"`.
   - **PWA shows:** Alice's check-in appears live in the **Logs** tab!
5. **Export Proof:** Click **Export CSV** on the Logs page.

---

## ⚠️ Troubleshooting in Seconds

| Symptom | Cause | 5-Second Fix |
| :--- | :--- | :--- |
| **`NODE_G491RE` drive doesn't appear** | USB cable is power-only (charge cable) | Swap with a known good **data cable** (the one that transfers files from your phone). |
| **A file named `FAIL.TXT` appears on the drive** | USB power dipped or copied before ready | Unplug board, plug back in, drag `firmware.bin` again. |
| **STM32CubeIDE says "ST-LINK not found"** | Port locked or loose connection | Unplug and replug the USB cable, or fall back to **Method A (drag & drop)**. |
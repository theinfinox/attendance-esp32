# RFID Attendance System — Project Report

## Scope and implementation reference

This report covers the current standalone STM32G491RE firmware and its companion Attendance PWA:

- Firmware: [standalone_g491re](../standalone_g491re/)
- Browser application: [hardware/attendance-pwa](attendance-pwa/)
- Firmware architecture reference: [NUCLEO G491RE presentation](../NUCLEO_G491RE_PRESENTATION.md)
- PWA technical reference: [Attendance PWA documentation](attendance-pwa/attendance-pwa.md)

The report describes the implementation represented by the source code. It does not claim that a physical hardware demonstration or end-to-end test was executed as part of preparing this document. Sample output shown below is illustrative unless explicitly identified as source-defined behaviour.

---

## 1. Introduction

The RFID Attendance System is a local attendance terminal built around the STM32G491RE microcontroller and a PN532 RFID reader. A student presents an RFID card to the reader, and the firmware uses the card UID to identify the enrolled user. For a recognised user, the system records an attendance event and alternates between check-in and check-out according to that user's current attendance state for the day.

The terminal uses the microcontroller's internal Flash memory to store enrolled users, current attendance state, and historical attendance events. An SSD1306 OLED provides on-device status and attendance feedback.

A browser-based Progressive Web Application (PWA) provides the host-side management interface. It connects to the board through USB serial using the Web Serial API. The PWA can retrieve device status, users, and attendance records; add or delete users; display live attendance events; and export data as JSON or CSV.

The current STM32 attendance path is designed to operate locally. It does not require Wi-Fi, a cloud backend, or a database server for card identification and attendance recording.

## 2. Problem Statement

Manual attendance recording is time-consuming and can introduce mistakes such as incorrect entries, duplicate entries, and difficulty maintaining a consistent record. A conventional paper-based process also makes it harder to retrieve historical information or generate digital reports.

The project addresses these issues by providing:

- RFID-based identification instead of manually writing attendance.
- Local storage of enrolled users and attendance events.
- Check-in/check-out tracking for multiple attendance sessions.
- A display for immediate feedback at the terminal.
- A browser interface for device management and attendance review.
- JSON and CSV export for further processing.

The system must also handle practical embedded-system constraints: limited Flash capacity, communication between the board and host, time synchronisation, repeated card reads, and persistence across device restarts.

## 3. Objectives

1. Identify enrolled users using RFID card UIDs read by the PN532 module.
2. Implement check-in and check-out transitions for each user.
3. Store user information and attendance history in the STM32G491RE internal Flash.
4. Maintain current attendance state separately from the historical event journal.
5. Display card-detection and attendance-result information on an OLED.
6. Provide a serial command interface for status queries, user management, time synchronisation, and attendance retrieval.
7. Build a browser-based PWA that communicates with the firmware over USB serial.
8. Provide dashboard, user-management, log-viewing, console, and export functions.
9. Keep the STM32 attendance workflow independent of a network backend.

## 4. Components Required

### 4.1 Hardware components

| Component | Purpose |
|---|---|
| ST NUCLEO-G491RE development board | Main controller; runs the attendance firmware |
| PN532 RFID reader module | Reads RFID/NFC card identifiers |
| Compatible RFID cards/tags | Carry the UID used to identify each enrolled user |
| SSD1306 OLED display | Displays boot, card-detection, and attendance-result information |
| USB cable | Board programming and serial communication with the host computer |
| Host computer | Runs the browser-based PWA and provides the USB serial connection |
| Jumper wires and suitable power source | Hardware interconnection and power |

The standalone firmware uses internal Flash storage. A MicroSD card is not required by this implementation.

The source defines LED and buzzer drivers, but the current configuration disables their outputs. They should not be listed as active feedback components in a description of the present build.

### 4.2 Firmware and software

| Software / library | Purpose |
|---|---|
| PlatformIO | Firmware build and upload workflow |
| STM32 Arduino framework | MCU programming framework |
| GNU C++17 | Firmware language standard |
| Adafruit PN532 | RFID reader interface |
| U8g2 | OLED display interface |
| ArduinoJson | JSON parsing and serial response generation |
| HTML, CSS, JavaScript ES modules | PWA user interface and application logic |
| Web Serial API | Browser-to-board serial communication |
| Service Worker and Web App Manifest | PWA shell caching and install metadata |

The PWA is a static browser application. It does not require a Node.js server, a frontend bundler, or an attendance-data backend for the current USB communication workflow.

## 5. System Design

### 5.1 High-level architecture

The system is divided into three functional layers:

**Hardware layer:** PN532 RFID reader, SSD1306 OLED, STM32G491RE, internal Flash, and USB serial interface.

**Firmware layer:** RFID manager, application state machine, display manager, attendance repository, Flash storage drivers, and serial CLI.

**Host application layer:** browser PWA, serial transport, protocol parser, dashboard, user management, attendance logs, and export utilities.

### 5.2 Architecture flow

    RFID Card
        |
        v
    PN532 RFID Reader
        |  SPI
        v
    STM32G491RE Firmware
        |
        +--> RFID Manager
        |
        +--> Application / State Machine
        |        |
        |        +--> User lookup
        |        +--> Check-in / Check-out decision
        |        +--> Attendance event creation
        |
        +--> Internal Flash Repository
        |        |
        |        +--> Zone A: User directory
        |        +--> Zone B: Active attendance state
        |        +--> Zone C: Attendance journal
        |
        +--> SSD1306 OLED
        |
        +--> USB Serial CLI <------> Browser PWA
                                      |
                                      +--> Dashboard
                                      +--> Users
                                      +--> Attendance Logs
                                      +--> Serial Console
                                      +--> JSON / CSV Export

The firmware owns the attendance operation and persistent data. The PWA acts as a host-side management and display interface; it does not directly access the RFID reader or MCU Flash.

### 5.3 Firmware architecture

The firmware uses an interface-oriented structure. The main entry point constructs the concrete peripheral managers and the Flash repository, then supplies them to the application. This separates application workflow from the individual hardware implementations.

Important firmware modules include:

| Module | Responsibility |
|---|---|
| main.cpp | Constructs the managers and runs setup/loop |
| app.cpp | Coordinates the attendance workflow |
| state_machine.cpp | Controls application state transitions |
| rfid_manager.cpp | Initialises and polls the PN532 reader |
| display_manager.cpp | Renders information on the OLED |
| cli_manager.cpp | Receives serial commands and sends responses |
| STM32FlashDriver.cpp | Performs internal Flash operations |
| STM32FlashUserStore.cpp | Stores enrolled users |
| STM32FlashStateStore.cpp | Stores active attendance state |
| STM32FlashJournal.cpp | Stores historical attendance events |
| STM32FlashRepository.cpp | Coordinates user, state, journal, and time operations |

### 5.4 Internal Flash storage design

The standalone firmware reserves a 64 KB region of internal Flash for attendance data.

| Storage zone | Address range | Size | Purpose |
|---|---|---:|---|
| Zone A | 0x08070000–0x08073FFF | 16 KB | Enrolled users; two 8 KB banks |
| Zone B | 0x08074000–0x08075FFF | 8 KB | Active attendance state; two 4 KB banks |
| Zone C | 0x08076000–0x0807FFFF | 40 KB | Append-only attendance event journal |
| **Total** | **0x08070000–0x0807FFFF** | **64 KB** | **Persistent application data** |

The journal uses a 16-byte header and 48-byte event records. Its source-defined slot calculation is:

    (40,960 - 16) / 48 = 853 records

Therefore, the journal has 853 event slots based on the current record layout. When the journal is full, further attendance writes are rejected rather than overwriting older records.

The three zones have different roles. Zone A stores enrolled identities, Zone B stores the current IN/OUT state used to decide the next punch, and Zone C stores the historical attendance events.

### 5.5 Attendance state flow

    Boot
      |
      v
    Initialise peripherals and Flash repository
      |
      v
    Ready for card
      |
      v
    Read card UID
      |
      v
    Validate card presence, debounce and cooldown
      |
      v
    Look up UID in user directory
      |
      +---- Unknown UID ----> Display failure / unknown card
      |
      v
    Determine current attendance state
      |
      +---- Currently OUT ----> Record Check-in
      |
      +---- Currently IN -----> Record Check-out
      |
      v
    Append event to Flash journal
      |
      v
    Update active attendance state
      |
      v
    Display result and emit serial event
      |
      v
    Return to Ready

The RFID manager polls at the configured 200 ms interval. The source also defines a 2,000 ms global scan debounce and a 5,000 ms same-UID cooldown. Card-presence handling requires removal to be detected before the same card can be accepted as a new presentation.

### 5.6 PWA architecture

The PWA is organised as a static web application:

| Module | Responsibility |
|---|---|
| index.html | Application shell and navigation |
| js/app.js | Page routing, connection orchestration, and UI actions |
| js/serial.js | Web Serial connection and line-based transport |
| js/protocol.js | JSON-line parsing and command lifecycle |
| js/state.js | Central in-memory application state |
| js/ui.js | Shared UI rendering and notifications |
| js/utils.js | Formatting, downloads, sorting, and deduplication |
| pages/dashboard.js | Dashboard |
| pages/users.js | User management |
| pages/logs.js | Attendance log display and export |
| pages/console.js | Serial command console |
| sw.js | Application-shell caching |

The PWA uses Web Serial at 115200 baud, 8 data bits, no parity, and 1 stop bit. It sends newline-terminated commands and processes newline-delimited device responses.

### 5.7 Connection and synchronisation sequence

After the user selects the board through the browser's serial device picker, the PWA performs a connection-time synchronisation sequence:

1. Open the serial port at 115200 baud.
2. Allow the board's startup/reset sequence to settle.
3. Send SET_TIME using the browser's Unix epoch and local timezone offset.
4. Request STATUS.
5. Request DUMP_USERS.
6. Request DUMP_LOGS.
7. Continue listening for live attendance log messages.

The firmware clock base is held in RAM. The PWA therefore synchronises time again when it connects after a device restart or power cycle.

## 6. Interfacing

### 6.1 PN532 to STM32G491RE

The PN532 communicates with the STM32 through the SPI1 interface in the current firmware configuration.

| PN532 signal | STM32 pin |
|---|---|
| SCK | PA5 |
| MISO | PA6 |
| MOSI | PA7 |
| CS / NSS | PB6 |
| Reset | PB1 |

The reader must be configured for the SPI interface. Confirm the exact module revision, power requirements, and logic-level compatibility before wiring. A module accepting 5 V power does not automatically imply that all of its logic pins are 5 V tolerant.

### 6.2 SSD1306 OLED to STM32G491RE

The OLED uses the I2C pins defined by the firmware:

| OLED signal | STM32 pin |
|---|---|
| SCL | PA9 |
| SDA | PA8 |

The display manager uses U8g2 to render the application screens.

### 6.3 STM32 to host computer

The board connects to the host through USB. The browser accesses the serial interface through the Web Serial API.

Serial configuration:

    Baud rate: 115200
    Data bits: 8
    Parity: None
    Stop bits: 1
    Flow control: None

The user must grant the browser permission to access the selected serial port. Web Serial requires a secure browser context, such as HTTPS or localhost, and is supported by compatible Chromium-based browsers.

The PWA also contains a CH340/CH341 WebUSB fallback. That fallback is specific to supported USB-to-UART hardware and includes ESP32-oriented RTS/DTR reset behaviour. The standard Web Serial route should be used for the current NUCLEO-G491RE connection unless the fallback has been independently validated for the actual adapter and wiring.

### 6.4 Firmware-to-PWA command protocol

The PWA and firmware use a line-oriented serial protocol. The firmware supports commands including:

| Command | Purpose |
|---|---|
| HELP | Display available CLI commands |
| STATUS | Return firmware, storage, RFID, user, log, and time status |
| DUMP_USERS | Return the enrolled UID-to-name mapping |
| DUMP_LOGS | Stream historical attendance records |
| ADD_USER with JSON payload | Add an enrolled user |
| DELETE_USER followed by UID | Delete an enrolled user |
| SET_TIME with epoch and timezone offset | Set the firmware's volatile time base |
| SYNC_TIME with epoch and timezone offset | Time-sync alias |
| CLEAR_LOGS | Clear the attendance event journal |

The CLI uses a JSON done marker to signal completion of structured operations. DUMP_LOGS streams individual event records and then sends the completion marker.

The current DUMP_USERS response contains a compact JSON mapping on one line between text markers. The current PWA can parse that compact JSON line; the marker lines are treated as non-JSON console output.

## 7. Code

The following extracts illustrate the key integration concepts and command shapes. They are representative examples, not a complete replacement for the source files.

### 7.1 Firmware application loop

The firmware entry point follows the Arduino setup/loop model. Initialisation opens serial communication and starts the application and CLI. The main loop polls the CLI and advances the application state machine.

    void setup() {
        Serial.begin(115200);
        app.init();
        cli.begin(&repository, &rfidManager);
    }

    void loop() {
        cli.update();
        app.update();
    }

The actual project also routes unhandled serial input to the RFID manager's simulated-input path. A simulated UID supplied over serial is a diagnostic path and is not proof of a physical PN532 card read.

### 7.2 Attendance decision

The repository uses the current attendance state to decide which event should be recorded. Conceptually:

    if (currentState == CHECKED_IN) {
        event = CHECK_OUT;
    } else {
        event = CHECK_IN;
        session++;
    }

The repository appends the event to the historical journal and then checkpoints the active state. During boot recovery, later journal events can be replayed to reconcile the active state store.

### 7.3 PWA serial connection configuration

The PWA opens the serial transport using the firmware's configured serial settings:

    {
        baudRate: 115200,
        dataBits: 8,
        stopBits: 1,
        parity: "none",
        flowControl: "none"
    }

Incoming data is decoded as text and split into complete lines. Outgoing commands are encoded as UTF-8 and terminated with a newline.

### 7.4 Example user creation command

    ADD_USER {"uid":"04A1B2C3","name":"Example Student","role":"Student"}

The current firmware validates the UID and name and stores the user. There is an implementation limitation around role persistence: the command can echo the selected role in its immediate response, but the current repository call stores the user through an API that does not pass the role through. DUMP_USERS also returns UID and name only. A refreshed PWA user list therefore cannot reliably restore the selected role.

### 7.5 Example time synchronisation command

    SET_TIME 1793385600 330

The first value is a Unix epoch in seconds. The second is the local timezone offset in minutes; 330 represents UTC+05:30. The PWA generates these values from the browser's current time and timezone.

### 7.6 Example attendance record

    {
      "uid": "04A1B2C3",
      "name": "Example Student",
      "timestamp": 1793385600,
      "event": "Check-in",
      "status": "IN",
      "session": 1,
      "event_id": 1,
      "date": 20261130
    }

This is an illustrative record showing the historical log fields. Actual values depend on the connected device and its stored attendance data.

## 8. Output

### 8.1 Device output

The firmware uses the OLED to show the boot process, readiness, card detection, and the result of an attendance operation. For an enrolled card, the result identifies the event as Check-in or Check-out. An unknown card or a storage failure follows the failure path.

The current configuration disables the external LED and buzzer outputs. The documented output should therefore focus on the OLED and serial interface.

### 8.2 Serial output

The CLI returns structured information for device status and attendance data. A representative status response has this general shape:

    {
      "type": "status",
      "data": {
        "board": "STM32",
        "firmware": "1.0.0",
        "storage": "FLASH",
        "rfid": "ONLINE",
        "users": 2,
        "logs": 14,
        "time_synced": true
      }
    }

The values above are illustrative. User count, log count, RFID state, and time-synchronisation state are supplied by the firmware at runtime.

### 8.3 PWA output

The browser application provides the following views:

- **Dashboard:** connection status, board and firmware information, user/log counts, current time, and recent attendance activity.
- **Users:** enrolled user list, search, sorting, pagination, add/delete actions, and JSON export.
- **Attendance Logs:** historical records, search, event filtering, pagination, JSON export, CSV export, and a clear-history action.
- **Serial Console:** raw command entry and device output.
- **Settings:** colour theme, Demo Time Mode, console auto-scroll, serial configuration information, and application cache refresh.
- **About:** application and connection information.

User data and attendance records are held in the PWA's runtime state and are retrieved from the device. The browser local storage is used for selected preferences, not as the system's primary attendance database.

Demo Time Mode changes the time presentation in the UI only. It does not change the firmware clock, raw attendance timestamps, Flash records, or exported data.

### 8.4 Output evidence note

The examples in this section describe expected response shapes based on the implementation. They are not screenshots or serial captures from a live hardware session. A project demonstration should capture actual OLED screens, serial responses, and PWA pages from the connected board.

## 9. Result

The source implementation provides the following system capabilities:

1. RFID UID acquisition through the PN532 interface.
2. User lookup against a locally stored directory.
3. Check-in/check-out event generation using the current attendance state.
4. Separate Flash storage for users, active attendance state, and historical events.
5. OLED feedback for the attendance workflow.
6. A serial CLI for status, user management, time synchronisation, and attendance retrieval.
7. A browser PWA for dashboard monitoring, user management, attendance-log review, and export.
8. Local attendance processing without requiring a network backend for the STM32 target.

The current implementation also has limitations that should be considered when interpreting the result:

- The firmware clock is volatile and must be synchronised again after a reset or power cycle.
- The event journal is limited to 853 records in the current Flash layout.
- The selected user role is not reliably persisted and returned by the current firmware repository/dump interface.
- The PWA's clear-log action does not validate the firmware's plain-text success/error response before clearing its local collection. A device-side erase failure could therefore be presented as a successful UI action.
- The CH340 WebUSB fallback is not the preferred generic connection route for the NUCLEO-G491RE.
- Source test scripts are present, but their presence alone does not establish a successful hardware integration test.

A complete project demonstration should validate card reads, unknown-card handling, repeated IN/OUT sessions, Flash persistence after reset, time synchronisation, serial command responses, and the PWA's user/log refresh and export operations on the actual hardware.

## 10. Conclusion

The project combines an STM32G491RE-based RFID attendance terminal with a browser-based PWA for device management and attendance review. The PN532 provides card identification, the firmware manages attendance state transitions, internal Flash stores users and attendance events, and the OLED provides on-device feedback. USB serial connects the terminal to the PWA, which offers a dashboard, user management, attendance logs, a serial console, and data export.

The current standalone implementation keeps the attendance workflow local to the microcontroller and does not depend on a cloud service to record a card tap. Its storage architecture separates user records, active attendance state, and historical attendance events, while the host application provides a convenient interface for managing and reviewing the information.

The implementation forms a functional base for an offline-capable attendance terminal. Before describing it as production-ready, the remaining operational gaps—particularly time resynchronisation, role persistence, clear-log result handling, Flash capacity management, and hardware-level validation—should be addressed and tested.

---

## Source references

### Firmware

- [PlatformIO configuration](../standalone_g491re/platformio.ini)
- [Firmware entry point](../standalone_g491re/src/main.cpp)
- [Application workflow](../standalone_g491re/src/app.cpp)
- [RFID manager](../standalone_g491re/src/rfid_manager.cpp)
- [Pin definitions](../standalone_g491re/include/pins.h)
- [Timing and feature constants](../standalone_g491re/include/constants.h)
- [Flash layout and record definitions](../standalone_g491re/include/flash_storage.h)
- [Serial CLI](../standalone_g491re/src/cli_manager.cpp)
- [Flash repository](../standalone_g491re/src/STM32FlashRepository.cpp)
- [Attendance event output](../standalone_g491re/src/api_client.cpp)

### PWA

- [PWA application shell](attendance-pwa/index.html)
- [Application controller](attendance-pwa/js/app.js)
- [Serial transport](attendance-pwa/js/serial.js)
- [Protocol adapter](attendance-pwa/js/protocol.js)
- [Application state](attendance-pwa/js/state.js)
- [Dashboard](attendance-pwa/pages/dashboard.js)
- [Users page](attendance-pwa/pages/users.js)
- [Attendance logs page](attendance-pwa/pages/logs.js)
- [PWA technical documentation](attendance-pwa/attendance-pwa.md)

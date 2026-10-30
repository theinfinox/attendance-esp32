# Attendly — USB Serial Attendance PWA
## Frontend Architecture, Device Integration & Technical Reference

> **Scope:** This document describes files inside \`hardware/attendance-pwa/\` only.  
> **Firmware integration reference:** the current \`standalone_g491re/\` implementation on the repository's \`master\` branch.  
> **Application type:** browser-side Progressive Web App (PWA), served as static files.  
> **Device transport:** USB serial through the browser Web Serial API.  
> **Serial settings:** 115200 baud, 8 data bits, no parity, 1 stop bit, no flow control.  
> **Current firmware target:** NUCLEO-G491RE / STM32G491RE.

This document is based on the current frontend source and its serial protocol integration with the standalone G491 firmware. It distinguishes implemented frontend features from known compatibility gaps. It does not describe the legacy firmware, backend services, or unrelated folders in the repository.

---

## 1. Project overview

Attendly is a local-first attendance management interface for a supported attendance terminal. The browser communicates directly with the device over a USB serial connection. It does not require a Node.js application server, database server, REST API, WebSocket server, or cloud service to manage the connected STM32 terminal.

The frontend provides:

- A dashboard with connection state, user and attendance counts, device details, and recent activity.
- User management: list, search, sort, paginate, add, delete, and export enrolled users.
- Attendance log viewing: search, event filtering, pagination, JSON export, CSV export, and clearing device logs.
- A serial console for direct CLI commands and device output.
- Settings for theme, console auto-scrolling, Demo Time Mode, and application cache refresh.
- PWA installation metadata and a service worker for caching the application shell.

The application uses native browser APIs and ES modules. There is no frontend framework or bundler configured in this directory.

### System boundary

\`\`\`mermaid
flowchart LR
    BROWSER[Browser / Attendly PWA]
    WEBUSB[Web Serial API]
    CH340[Optional CH340 WebUSB fallback]
    USB[USB cable / serial bridge]
    MCU[NUCLEO-G491RE]
    CLI[STM32 serial CLI]
    FLASH[STM32 internal Flash]
    PN[PN532 RFID reader]

    BROWSER --> WEBUSB
    BROWSER -. optional fallback .-> CH340
    WEBUSB --> USB
    CH340 --> USB
    USB --> CLI
    CLI <--> MCU
    MCU <--> FLASH
    PN --> MCU
\`\`\`

The frontend is the host-side management interface. The STM32 firmware owns card acquisition, user persistence, attendance state transitions, timestamps, and event-journal storage. The PWA requests or displays that information; it does not directly read the PN532 or access MCU Flash.

### No network API in the device data path

The PWA's device operations are serial commands. It does not send attendance data to an HTTP endpoint. The configured server URL and Wi-Fi settings found in firmware configuration files are not part of the standalone STM32/PWA communication path.

A static web host may be used to serve the frontend, but that host is not an application backend and does not process attendance data.

---

## 2. Runtime requirements

### Browser and security context

The application uses the Web Serial API, which requires a secure browser context. Use:

- HTTPS when hosted remotely.
- \`http://localhost\` for local development.

Do not open \`index.html\` directly with \`file://\`.

The source README recommends a current Chromium-based browser such as Google Chrome or Microsoft Edge. The user must initiate the device picker from the page's Connect device action and grant access to the serial port.

The host operating system must expose the board's USB serial interface as an available port. The frontend cannot install USB drivers or make an absent device enumerate.

### Current NUCLEO-G491RE connection path

For the standalone STM32 target, use the serial interface exposed by the NUCLEO board's USB/debug interface and select the corresponding serial port in the browser picker. The PWA opens the port using the firmware's 115200-baud configuration.

The default transport is Web Serial. The optional CH340/CH341 WebUSB implementation is a fallback for supported USB-to-UART adapters; it is not required for the normal NUCLEO-G491RE serial connection.

### Local development

From the repository root:

\`\`\`powershell
cd hardware/attendance-pwa
python -m http.server 8080
\`\`\`

Open:

\`\`\`text
http://localhost:8080
\`\`\`

Then select **Connect device** and choose the serial port exposed by the board.

No \`npm install\`, bundler build, or backend process is defined for this frontend directory. The app is composed of static HTML, CSS, JavaScript modules, and a service worker.

### Deployment

Any static hosting environment that serves the directory over HTTPS can host the PWA. Preserve the relative directory structure because the HTML, CSS imports, JavaScript modules, manifest, icon, and service worker refer to one another using relative paths.

The existing \`deploy.bat\` is a machine-specific rsync script with fixed local and remote paths. It is not a portable deployment configuration; update its paths and target before using it in another environment.

---

## 3. Frontend directory structure

The frontend is intentionally self-contained in this folder.

| Path | Responsibility |
|---|---|
| \`index.html\` | Application shell, navigation, top bar, connect button, page content root, dialogs, and module entry point |
| \`manifest.json\` | PWA name, display mode, theme colours, start URL, and icon |
| \`sw.js\` | Application-shell caching, cache versioning, offline fallback, and cache cleanup |
| \`assets/icon.svg\` | Application icon |
| \`css/main.css\` | CSS entry point, design tokens, theme variables, base styles, and imports |
| \`css/layout.css\` | Sidebar, top bar, page layout, and responsive navigation |
| \`css/components.css\` | Cards, metrics, buttons, tables, forms, pagination, modals, terminal, badges, and toasts |
| \`css/animations.css\` | Page entrance, toast, and connection-status animations |
| \`js/app.js\` | App controller, page routing, device orchestration, command actions, auto-sync, and live event handling |
| \`js/state.js\` | Central in-memory application state and subscription API |
| \`js/serial.js\` | Web Serial transport, serial port lifecycle, line framing, and writes |
| \`js/ch340.js\` | CH340/CH341 WebUSB transport fallback |
| \`js/protocol.js\` | JSON-line CLI parsing, request lifecycle, stream collection, timeout handling, and message events |
| \`js/ui.js\` | Shared navigation/status rendering, toast notifications, and console output |
| \`js/utils.js\` | HTML escaping, date formatting, Demo Time Mode, downloads, CSV, log ordering, and deduplication |
| \`pages/dashboard.js\` | Dashboard rendering |
| \`pages/users.js\` | User table, form, search, sort, pagination, and actions |
| \`pages/logs.js\` | Attendance table, search, filtering, pagination, and actions |
| \`pages/console.js\` | Terminal view and command input/history |
| \`audit.html\` | Standalone browser/Android USB compatibility audit utility; not part of the normal app navigation |
| \`test/\` | JavaScript validation and browser-serial workflow test scripts |

### Application modules and data flow

\`\`\`mermaid
flowchart TD
    ENTRY[index.html] --> APP[js/app.js]
    APP --> STATE[js/state.js]
    APP --> UI[js/ui.js]
    APP --> PROTOCOL[js/protocol.js]
    PROTOCOL --> SERIAL[js/serial.js]
    SERIAL --> CH340[js/ch340.js]
    APP --> UTILS[js/utils.js]
    APP --> DASH[pages/dashboard.js]
    APP --> USERS[pages/users.js]
    APP --> LOGS[pages/logs.js]
    APP --> CONSOLE[pages/console.js]
    APP --> DEVICE[USB serial device]
\`\`\`

The app controller composes the transport and protocol objects once, then uses the page modules as renderers. Page modules render HTML and bind their own UI event handlers through callbacks supplied by \`app.js\`.

---

## 4. Application shell and navigation

\`index.html\` defines the persistent shell around the page content.

### Main shell elements

- **Sidebar:** Dashboard, Users, Attendance Logs, Serial Console, Settings, and About.
- **Sidebar footer:** connection status indicator.
- **Top bar:** current page title, Demo Time badge, theme toggle, and Connect device / Disconnect button.
- **Page content:** \`#app-content\`, where the active page renderer inserts its content.
- **Toast region:** transient success/error information.
- **Reconnect dialog:** prompts the user when the connected serial device is removed or disconnected while the page is focused.

The initial page is Dashboard. Navigation is client-side: selecting a sidebar item changes the active page and renders the corresponding module without a full document navigation.

### Responsive layout and themes

The CSS uses a fixed desktop sidebar and a slide-in sidebar/backdrop on narrower screens. The dashboard metrics use a multi-column grid that collapses to fewer columns on smaller viewports. Tables are horizontally scrollable when their content exceeds the available width.

Light and dark themes are implemented through CSS custom properties on the document root. The selected theme is saved in local storage.

---

## 5. Page-level functionality

### 5.1 Dashboard

The dashboard is rendered by \`pages/dashboard.js\` and updated by the controller when serial state changes.

It displays:

- USB connection state.
- Registered user count.
- Attendance record count.
- Current/device time.
- Board type.
- Firmware version.
- Storage backend/status.
- RFID status.
- The five latest attendance records.
- Quick actions to refresh users, refresh logs, and request CLI help.

Dashboard values are initially derived from central state and the last status response. They are refreshed when serial status or data is received. The current time display is refreshed periodically.

### 5.2 Users

The Users page is implemented in \`pages/users.js\`.

Available operations:

- Load the enrolled users from the device using \`DUMP_USERS\`.
- Search users by visible fields.
- Sort by name, ID, or role.
- Paginate the table (10 entries per page).
- Add a new user using UID, name, and role.
- Populate the UID field using Fetch Last when a recent scanned UID was detected in device output.
- Delete a user from the device after confirmation.
- Export the current in-memory user collection as JSON.

The form validates a hexadecimal UID of 4–32 characters and a non-empty name. The current firmware applies its own validation as well.

**Import JSON is deliberately disabled.** The current serial CLI does not define a safe bulk-import command, so the frontend does not pretend to support importing an arbitrary JSON file into MCU storage.

The page does not provide a user-edit workflow; an existing user can be deleted and re-added.

### 5.3 Attendance Logs

The Logs page is implemented in \`pages/logs.js\`.

Available operations:

- Retrieve historical attendance events using \`DUMP_LOGS\`.
- Search across record fields.
- Filter by all events, Check-in, or Check-out.
- Paginate the table (12 entries per page).
- Export the currently loaded collection as JSON.
- Export the currently loaded collection as CSV.
- Clear device attendance logs after a confirmation prompt.

Displayed columns are time, user, RFID card, and event.

The frontend normalises common field variants such as \`timestamp\` / \`time\`, \`uid\` / \`card\`, and \`name\` / \`user_name\`. The standalone G491 firmware's historical records include a numeric \`event_id\`, timestamp, UID, name, event type, status, session, and date.

**Clear logs affects the device.** In the current STM32 Flash implementation, \`CLEAR_LOGS\` erases the historical event journal (Zone C), but preserves the enrolled user directory (Zone A) and active attendance state (Zone B). Clearing the history therefore does not necessarily reset a student's current IN/OUT state.

### 5.4 Serial Console

The Serial Console is implemented in \`pages/console.js\`.

It provides:

- A terminal-like view of incoming device output and outgoing commands.
- A command input field.
- Enter-to-send behaviour.
- Up/down arrow navigation through the console's in-memory command history.
- Automatic scrolling when enabled in Settings.

The shared console state retains at most 600 entries. The console sends raw command text through the serial transport; it does not validate arbitrary CLI syntax. It is useful for diagnostics and for commands that are not exposed as dedicated UI actions.

The console is not a separate shell or operating-system terminal. It only exchanges text with the connected firmware's serial CLI.

### 5.5 Settings

The Settings page is rendered inside \`js/app.js\`.

It currently provides:

| Setting | Behaviour |
|---|---|
| Colour theme | Switches light/dark theme; stored in local storage |
| Demo Time Mode | Enables a display-only time shift |
| Presentation Time Offset | Sets positive or negative hour/minute offset while Demo Time Mode is active |
| Reset to actual time | Turns Demo Time Mode off and clears its offset |
| Automatic console scrolling | Controls whether terminal output scrolls to the latest entry; runtime state only |
| Serial configuration | Displays the fixed 115200 / 8-N-1 configuration |
| Update App | Unregisters service workers, clears Cache Storage, and reloads the page |

### 5.6 About

The About page identifies the product as a local-first PWA for attendance hardware. It documents USB serial at 115200 baud, a newline-delimited JSON CLI, offline-capable application-shell caching, and Chromium-based browser support.

---

## 6. USB serial transport

The transport layer is implemented in \`js/serial.js\`. It is intentionally independent of the attendance command schema and DOM.

### Primary transport: Web Serial

The connection process is:

1. User activates Connect device.
2. Browser displays the available serial-port picker (unless a previously granted port is reused).
3. The selected port is opened with the fixed serial configuration.
4. A read loop continuously consumes bytes from the readable stream.
5. A streaming text decoder converts byte chunks to text.
6. Text is buffered until a newline is received.
7. Each completed line is dispatched to the protocol adapter.
8. Writes are UTF-8 encoded and terminated with a newline.

The transport opens the port using:

\`\`\`javascript
{
  baudRate: 115200,
  dataBits: 8,
  stopBits: 1,
  parity: "none",
  flowControl: "none"
}
\`\`\`

The reader releases its lock and closes the port when the connection ends. Device disconnection is surfaced as an event to the app controller.

### Optional WebUSB fallback: CH340 / CH341

\`js/ch340.js\` implements a direct WebUSB transport for CH340/CH341 USB-to-serial chips. It requests devices using vendor ID \`0x1A86\`, claims interface 0, configures the serial chip for 115200 baud, and transfers serial data through bulk endpoint 2.

The fallback is selected when Web Serial is unavailable or the Web Serial connection path throws a \`NotFoundError\`. It is not a generic WebUSB serial driver for every USB bridge.

**Hardware caution:** the CH340 initialisation code also toggles RTS/DTR with a reset pulse explicitly commented for an ESP32 NodeMCU circuit. Do not assume that reset signalling is suitable for the NUCLEO-G491RE or another STM32 board. The standard Web Serial path should be preferred for the current NUCLEO target; use the CH340 fallback only with a compatible adapter and after validating its control-line behaviour.

### Connection and reconnection behaviour

The app stores the selected Web Serial port object in runtime state and uses it for reconnect attempts where available. This selection is not persisted across page reloads.

If the serial transport reports a disconnect while the page is focused, the app opens a reconnect dialog. A WebUSB fallback connection does not populate the Web Serial port object, so reconnecting through that path requires another device selection.

---

## 7. Serial protocol adapter

\`js/protocol.js\` converts newline-delimited serial text into JSON messages and matches messages to the active command request.

### Message handling

- A line that parses as JSON is dispatched as a \`message\` event.
- A line that does not parse as JSON is dispatched as a \`malformed\` event.
- During a multiline JSON request, non-diagnostic lines are appended to a buffer and retried as a combined JSON document.
- A command request can have a timeout, streaming mode, idle timeout, and multiline JSON mode.
- Only one structured command request may be pending at a time.
- A non-streaming request normally completes on the first JSON message other than \`log\` or \`user\`, or on a \`{"type":"done"}\` message.
- A streaming request completes when the \`done\` marker is received, unless an explicit idle timeout is configured.

Malformed lines are still useful output: the app routes them to the console and attempts to interpret selected firmware diagnostic strings.

### Why the done marker matters

The current G491 CLI emits \`{"type":"done"}\` after \`STATUS\`, \`DUMP_USERS\`, \`DUMP_LOGS\`, \`CLEAR_LOGS\`, \`HELP\`, \`ADD_USER\`, and \`DELETE_USER\`.

The PWA's \`DUMP_LOGS\` request is streaming and waits for this marker. The current firmware emits it, so the stream has an explicit completion condition rather than relying on an arbitrary serial-idle delay.

The PWA README describes an earlier CLI behaviour in which \`DUMP_LOGS\` completed after 800 ms of serial inactivity because the firmware did not send a final marker. That description is stale for the current standalone G491 firmware.

---

## 8. Current standalone G491 firmware integration

This section documents only the interface between this frontend and \`standalone_g491re/\`.

### Connection-time synchronisation

After a serial connection succeeds, \`app.js\` starts an automatic sequence:

1. Wait 1.5 seconds to allow a board reset/boot to complete.
2. Send \`SET_TIME <unix_epoch_seconds> <timezone_offset_minutes>\` using the browser's current time and local timezone offset.
3. Wait briefly.
4. Send \`STATUS\`.
5. Wait briefly.
6. Request users using \`DUMP_USERS\`.
7. Wait briefly.
8. Request attendance history using \`DUMP_LOGS\`.

The initial time-sync request is important for the STM32 firmware because its epoch base is RAM-held and must be supplied again after a reset or power cycle. The frontend calculates the timezone offset in minutes using the browser's local timezone.

Auto-sync is sequential. If one awaited operation fails, the surrounding auto-sync block logs a warning and stops the remaining sequence. In particular, a user-list protocol failure can prevent the initial log dump from being requested.

### Commands exposed by the current G491 CLI

| PWA action | Serial command | Current firmware behaviour |
|---|---|---|
| Connection time sync | \`SET_TIME <epoch> <tz-minutes>\` | Sets the firmware's volatile clock base and timezone |
| Dashboard status | \`STATUS\` | Returns board, firmware, storage, RFID, user/log counts, and clock fields |
| Refresh users | \`DUMP_USERS\` | Prints UID/name mapping surrounded by text markers |
| Add user | \`ADD_USER {"uid":"...","name":"...","role":"..."}\` | Validates and stores a user; emits a \`type=user\` JSON response on success |
| Delete user | \`DELETE_USER <uid>\` | Deletes the enrolled UID; emits a \`type=deleted\` response on success |
| Refresh attendance history | \`DUMP_LOGS\` | Streams one JSON attendance record per line, then emits \`type=done\` |
| Clear attendance history | \`CLEAR_LOGS\` | Erases the journal; prints a plain-text success/error line, then emits \`type=done\` |
| Console help | \`HELP\` | Prints CLI help, then emits \`type=done\` |

### STATUS response shape

The current STM32 firmware emits a status message with a structure equivalent to:

\`\`\`json
{
  "type": "status",
  "data": {
    "board": "STM32",
    "firmware": "1.0.0",
    "storage": "FLASH",
    "rfid": "ONLINE",
    "users": 2,
    "logs": 14,
    "time_synced": true,
    "time": "2026-10-30T12:30:00Z"
  }
}
\`\`\`

Values above are illustrative. Actual counts, time, and RFID state come from the connected firmware.

### Historical attendance record shape

The standalone repository streams records containing fields such as:

\`\`\`json
{
  "uid": "04A1B2C3",
  "name": "Example Student",
  "timestamp": 1793363400,
  "event": "Check-in",
  "status": "IN",
  "session": 1,
  "event_id": 1,
  "date": 20261030
}
\`\`\`

The PWA's Logs page normalises these fields and displays the event in local date/time formatting.

### Live attendance event shape

After a successful local attendance write, the standalone firmware emits a JSON message equivalent to:

\`\`\`json
{
  "type": "log",
  "data": {
    "uid": "04A1B2C3",
    "name": "Example Student",
    "timestamp": 1793363400,
    "event": "Check-in",
    "status": "IN",
    "session": 1
  }
}
\`\`\`

The live message currently does not include \`event_id\` or \`date\`, whereas the historical Flash dump includes them. The PWA uses event ID when available for sorting/deduplication and falls back to UID/card plus timestamp when reconciling a live event with the subsequent historical dump.

### Important protocol compatibility gaps

These are source-level mismatches found when comparing the current frontend with the current standalone G491 CLI. They are recorded here for developers; this document does not change either side.

#### A. DUMP_USERS framing is incompatible with the current multiline parser

The G491 CLI currently prints:

\`\`\`text
--- BEGIN USERS ---
{"04A1B2C3":"Example Student"}
--- END USERS ---
{"type":"done"}
\`\`\`

The PWA requests this command with \`allowMultilineJson: true\`. Its multiline parser appends non-JSON lines to one raw buffer. The \`--- BEGIN USERS ---\` and \`--- END USERS ---\` markers are not treated as diagnostics or discarded framing; they are included in the attempted JSON document. As a result, the combined buffer is not valid JSON and the request eventually times out.

This affects manual user refresh and the connection-time auto-sync sequence. Because auto-sync awaits the user refresh before requesting logs, this failure can also prevent the initial \`DUMP_LOGS\` fetch after connection.

**Integration requirement:** the frontend parser and firmware output format must agree. Either the firmware should provide a clean parseable JSON response with framing the frontend expects, or the frontend should explicitly ignore the BEGIN/END marker lines and parse only the enclosed JSON. Do not assume user refresh currently works end-to-end with this framing.

#### B. CLEAR_LOGS failure is not distinguished by the UI

The firmware reports clear success/failure as plain text (for example, \`OK: Logs cleared.\` or \`ERR: Failed to clear logs.\`) and then emits the normal \`done\` marker. The PWA's protocol adapter does not include malformed text lines in the structured command result. The Logs page clears its local collection and displays “Logs cleared” after the command promise resolves, without checking the plain-text device result.

Consequently, a Flash erase failure can be presented by the UI as a successful clear. The frontend should only clear its local state after receiving an explicit machine-readable success response from the device.

#### C. User role is not round-tripped by the current firmware repository interface

The Users form sends the selected role and the firmware CLI echoes it in the immediate \`type=user\` success response. However, the current CLI calls the repository's two-argument \`addStudent(uid, name)\` API. The Flash user store's role parameter therefore receives its default value rather than the selected role. Further, \`DUMP_USERS\` returns a UID-to-name mapping without role metadata.

The PWA can temporarily display the role echoed by the add response, but a later refresh cannot reconstruct that selected role from the current dump and falls back to the generic role. Treat role selection as presentation metadata until the firmware repository and dump protocol persist and return it.

#### D. Import JSON is intentionally unavailable

The frontend disables Import JSON because the current G491 CLI has no bulk-import command. A file selected in the browser cannot be written safely to MCU storage without a defined firmware-side import protocol.

---

## 9. Live event processing, sorting, and duplicate handling

The application listens to protocol-level JSON messages independently of the currently visible page.

When a message has \`type: "log"\`, the controller normalises the event fields and adds the record to the central log collection. The app ignores an event already present according to its duplicate check, sorts the resulting list, and updates the Logs table when that page is visible.

### Ordering rules

The utility functions in \`js/utils.js\` use these rules:

1. If both records contain valid positive event IDs and those IDs differ, the higher event ID sorts first.
2. Otherwise, records are ordered by descending timestamp.
3. A positive event ID sorts ahead of a record without one when their timestamps tie.

This gives the historical STM32 journal's monotonically increasing event IDs precedence over timestamp ordering when both records have IDs.

### Deduplication rules

- When both records have valid event IDs, matching event IDs identify the same event.
- If both do not have IDs, matching positive timestamps plus matching UID/card identify duplicates.
- The merge function preserves richer event fields such as event ID, timestamp, session, and date when merging duplicate live and historical entries.

The live event from the current G491 firmware has no event ID. When the historical dump later returns the same event with an event ID, the fallback UID/timestamp comparison is what allows the PWA to collapse them into one entry.

### Demo Time Mode does not modify device data

Demo Time Mode shifts date/time values only when the UI formats them for display. It does not rewrite the raw record timestamp, the firmware clock, Flash contents, JSON export, or CSV export.

This is a presentation feature. It must not be used to correct an unsynchronised firmware clock or to change real attendance data.

---

## 10. State and browser-side persistence

The central state module is \`js/state.js\`. It provides \`get()\`, \`set()\`, \`reset()\`, and \`subscribe()\` methods and notifies registered listeners when a state patch is applied.

### In-memory application state

The current state includes:

- Connection and connecting flags.
- Board and firmware labels.
- User and attendance arrays.
- Latest device status.
- Console history.
- Theme and console auto-scroll settings.
- Last error and selected Web Serial port.
- Demo Time Mode and its offset.

### Local storage

The frontend persists only selected preferences:

| Key | Purpose |
|---|---|
| \`attendance-theme\` | Light/dark theme |
| \`attendance-demo-mode\` | Demo Time Mode enabled/disabled |
| \`attendance-demo-offset-ms\` | Presentation time offset in milliseconds |

User records, attendance logs, console lines, and the selected port are not persisted as application data in local storage. They are held in runtime state and reloaded from the device after connection. Exported files are downloaded to the user's browser/device; they are not uploaded to a backend.

### Service worker and offline shell

The service worker caches the PWA shell and its static dependencies. The cache version is currently \`14.3\` in \`sw.js\`, matching the version query on the app entry script. During activation, the service worker removes older cache entries and claims clients.

The fetch handler serves a cached response when available while attempting a network refresh. If the network fetch fails, it falls back to the cached app index.

**Offline limitation:** cached UI availability does not mean the attendance terminal is available offline from the browser. A USB device must still be connected for status, user management, log refresh, or device-side commands.

**Cache-scope caution:** the service worker activation logic deletes cache entries whose names do not equal this app's current cache name. The Update App action also unregisters service workers and deletes Cache Storage entries for the origin. If multiple unrelated applications share the same origin, this broad cleanup may affect their caches or service-worker registrations.

---

## 11. Data export

### Users JSON

The Users page exports the user collection currently held in frontend state as a JSON file named:

\`\`\`text
attendance-users.json
\`\`\`

The export is a client-side download. It does not write data back to the device.

### Attendance JSON

The Logs page exports the currently loaded attendance collection as:

\`\`\`text
attendance-logs.json
\`\`\`

### Attendance CSV

The Logs page exports the currently loaded collection as:

\`\`\`text
attendance-logs.csv
\`\`\`

The CSV utility builds a column set from the union of keys in the supplied records and quotes values, escaping embedded double quotes. Exported values remain the original record data; Demo Time Mode affects only formatted UI output.

Exports include the records currently loaded in browser state. Refresh from the device before exporting when a complete current journal is required.

---

## 12. Error handling and operational behaviour

| Situation | Frontend behaviour |
|---|---|
| Web Serial unavailable | Attempts the CH340 WebUSB fallback where the browser exposes WebUSB |
| User cancels or port picker reports NotFoundError | Current catch path can attempt the WebUSB fallback |
| Serial port open fails | Connection state is reset and an error toast is shown |
| Device disconnects | Connection state is updated; reconnect dialog is shown when the page is focused |
| Structured command times out | Error is written to console and shown as a toast |
| Device prints non-JSON diagnostics | Line is shown in console; selected diagnostic patterns update device/last-scan UI state |
| Attendance log arrives live | Normalised, deduplicated, sorted, and placed in central state |
| No device connected | Device actions show an error and do not submit their command |
| User import requested | Import JSON remains disabled |
| Clear-log command completes | Current UI assumes success after the structured request resolves; see the compatibility caveat above |

The protocol adapter permits only one structured request at a time. Avoid sending a second command through the structured UI while another request is pending. Raw console writes bypass the request queue and should be used carefully while a structured operation is in progress.

---

## 13. Testing and verification assets

The \`test/\` directory contains the following scripts:

| File | Intended coverage |
|---|---|
| \`test_app_init.mjs\` | App startup in a mocked DOM, top-level event binding, navigation, Settings render, and Demo Time badge visibility |
| \`test_demo_mode.mjs\` | Positive/negative offset calculations, display formatting, reset behaviour, invalid timestamps, and raw export integrity |
| \`test_log_ordering.mjs\` | Event ID extraction, timestamp parsing, descending ordering, duplicate detection, merge behaviour, and reconnect ordering |
| \`test_settings_view.mjs\` | Settings-related helper/render preparation checks |
| \`verify_all_syntax.mjs\` | Syntax parsing for the app modules and service worker |

The \`audit.html\` file is a separate manual Android USB/Web Serial compatibility audit utility. It is not part of the normal Attendly navigation and is not required for desktop operation.

These scripts are test assets in the repository. This documentation review did not execute them in a browser with a connected NUCLEO-G491RE. Their existence does not establish that the complete hardware-to-browser workflow has passed.

### Recommended end-to-end validation

1. Serve the folder on localhost or HTTPS.
2. Open the app in a supported Chromium browser.
3. Connect the NUCLEO-G491RE serial interface at 115200 baud.
4. Confirm the connection status changes to Connected.
5. Confirm that the frontend sends \`SET_TIME\` and receives a \`time_sync\` response.
6. Run \`STATUS\` from the Console and verify board \`STM32\`, firmware \`1.0.0\`, and storage \`FLASH\`.
7. Verify \`DUMP_USERS\` framing before relying on the Users page; the current marker/parser mismatch must be resolved.
8. Verify \`DUMP_LOGS\` returns individual JSON records followed by \`{"type":"done"}\`.
9. Add and delete a test user; reconnect and verify persistence.
10. Present a test RFID card and verify that a live \`type: "log"\` message is received.
11. Refresh historical logs and verify no duplicate live/history rows remain.
12. Test \`CLEAR_LOGS\` only with disposable data and verify the device response, not just the frontend toast.
13. Reload the page and confirm cached shell availability; reconnect to the device because runtime user/log state is not stored in local storage.

---

## 14. Known frontend implementation caveats

The following observations are about the current source in this folder and its integration with the current G491 firmware:

1. **DUMP_USERS parser mismatch:** current firmware surrounds the JSON user map with non-JSON BEGIN/END marker lines. The frontend multiline parser includes these lines in its JSON buffer, so the request times out. This also stops the sequential first-connect auto-sync before the log dump.
2. **CLEAR_LOGS false-success presentation:** the current firmware's clear result is plain text; the PWA resolves on the done marker and clears local state without validating the device's textual success/error line.
3. **Role does not round-trip:** the current firmware accepts and echoes a role in the immediate add response, but the repository call omits the role and the user dump does not return role metadata. A refreshed list cannot preserve the selected role.
4. **Demo Time is UI-only:** it must not be represented as a firmware clock or as a modification to saved attendance timestamps.
5. **Runtime collections are not browser-persisted:** users/logs/console output must be retrieved again after a full page reload and device reconnection.
6. **WebUSB fallback is CH340-specific:** it is not a generic fallback for arbitrary USB serial devices. Its RTS/DTR reset pulse is ESP32-oriented and should not be presumed safe for the STM32 board.
7. **Cache cleanup is origin-wide:** service-worker activation and Update App can remove cache/service-worker entries broader than this single app when multiple apps share an origin.
8. **No bulk import endpoint:** Import JSON is correctly disabled until a matching firmware command and validation contract exist.
9. **No framework build pipeline:** the directory uses browser ES modules directly. Introducing a bundler or framework would require updating service-worker precache paths and deployment instructions as well.
10. **Test evidence is not hardware evidence:** mocked DOM and utility tests do not replace a serial integration test with the actual STM32 board.

These items are not claims that every user session fails. They identify concrete behaviour or compatibility risks visible in the current source and should be resolved before the frontend is described as a fully verified production integration.

---

## 15. Source-of-truth map

Frontend implementation:

- [Application shell](index.html)
- [PWA manifest](manifest.json)
- [Service worker](sw.js)
- [Application controller](js/app.js)
- [Central state](js/state.js)
- [USB serial transport](js/serial.js)
- [CH340 WebUSB fallback](js/ch340.js)
- [CLI protocol adapter](js/protocol.js)
- [Shared UI helpers](js/ui.js)
- [Formatting and log utilities](js/utils.js)
- [Dashboard page](pages/dashboard.js)
- [Users page](pages/users.js)
- [Logs page](pages/logs.js)
- [Serial console page](pages/console.js)
- [Stylesheets](css/)
- [Frontend tests](test/)

Firmware integration reference only:

- [Standalone G491 CLI](../../standalone_g491re/src/cli_manager.cpp)
- [Standalone G491 attendance event output](../../standalone_g491re/src/api_client.cpp)
- [Standalone G491 Flash repository](../../standalone_g491re/src/STM32FlashRepository.cpp)

For future frontend changes, update this document from files inside \`hardware/attendance-pwa/\`. For USB protocol compatibility, compare against the current \`standalone_g491re/\` CLI and event output. Do not use an older PWA README or legacy firmware folder as the protocol authority.

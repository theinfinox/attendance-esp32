# Serial Attendance Manager

A client-side PWA for managing ESP32 or STM32 attendance hardware through USB Serial. It uses the browser Web Serial API only; it has no backend and does not use Wi-Fi, HTTP, or WebSockets for device communication.

## Run locally

Web Serial needs a secure context, so do not open `index.html` with `file://`. Serve this directory on localhost instead:

```powershell
cd attendance-pwa
python -m http.server 8080
```

Open `http://localhost:8080` in a current Chromium-based browser such as Chrome or Edge, then choose **Connect device**. The app opens the selected port at 115200 baud and sends newline-terminated CLI commands.

## Expected protocol

The app is configured for the supplied CLI: `DUMP_USERS`, `DUMP_LOGS`, `CLEAR_LOGS`, and `HELP`. `DUMP_USERS` may return a complete JSON file (including a pretty-formatted multi-line document); `DUMP_LOGS` may stream JSONL records. A log dump completes after 800 ms of serial inactivity because this firmware does not provide a final `{ "type": "done" }` marker.

`Import JSON` remains a deliberately disabled future enhancement because the supplied serial CLI specification does not define a safe import command.

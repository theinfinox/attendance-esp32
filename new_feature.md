Implement the following additions to the existing Serial CLI. Do not modify or remove any existing ESP32 Wi-Fi/Web UI functionality. The Serial CLI is an additional interface only.

Requirements:

1. Implement the command:

STATUS

Response format:

{"type":"status","data":{...}}
{"type":"done"}

Populate fields only from information that already exists in the firmware. Do not invent values. Include available fields such as board, firmware version, users count, logs count, storage status, RFID status, and current time only if they are already obtainable.

2. Implement the command:

ADD_USER {"uid":"A1B2C3D4","name":"Anu","role":"Student"}

Requirements:

- Recognize ADD_USER before any RFID fallback logic.
- Split at the first space and parse the remaining text as JSON.
- Validate:
  - uid: hexadecimal, 4–32 characters
  - name: non-empty, max 80 characters
  - role: optional (default "User")
- Reject malformed JSON.
- Reject duplicate UIDs.
- Reject storage/write failures.
- Save the user using the existing users repository (same storage used by DUMP_USERS).
- Use safe persistence (temporary file then replace/rename the original).

Success response:

{"type":"user","data":{"uid":"A1B2C3D4","name":"Anu","role":"Student"}}
{"type":"done"}

Failure response:

{"type":"error","message":"<reason>"}
{"type":"done"}

3. Preserve existing CLI commands and behavior:

- HELP
- DUMP_USERS
- DUMP_LOGS
- CLEAR_LOGS

4. Do not change:

- Existing Web UI
- Wi-Fi functionality
- PN532/RFID behavior
- Existing application architecture

5. Verify manually:

STATUS
DUMP_USERS
ADD_USER {"uid":"A1B2C3D4","name":"Anu","role":"Student"}
DUMP_USERS

The newly added user must appear in DUMP_USERS and ADD_USER must not be interpreted as an RFID scan.

Before uploading firmware, ask me to disconnect the PWA from the COM port so PlatformIO can access it.

~~~

For STATUS, use these exact field names and types when available:

{
  "board": "string",
  "firmware": "string",
  "users": 0,
  "logs": 0,
  "storage": "string",
  "rfid": "string",
  "time": "ISO-8601 string"
}

Normalize ADD_USER uid to uppercase before duplicate checking and persistence.

The CLI must emit valid JSON only for command responses—no INFO/DEBUG text between the response JSON lines.

Add manual verification for:
- duplicate UID rejection
- malformed JSON rejection
- persistence after an ESP32 reboot
- DUMP_LOGS still works after ADD_USER

All commands are newline-terminated.

~~~

One frontend note: the PWA already parses the specified STATUS response, but it is not currently auto-sending STATUS on connect because the firmware did not support it before. Once the firmware agent adds it, I can switch that automatic status load back on.

~~

be additive, dont be destrcutive at any cost.

current architecture never changes until explicitly said
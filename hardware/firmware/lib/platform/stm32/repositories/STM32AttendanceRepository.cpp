#if defined(ARDUINO_ARCH_STM32)
// ============================================================================
// STM32AttendanceRepository.cpp — STM32 Storage Implementation
// ============================================================================

#include "STM32AttendanceRepository.h"
#include "logger.h"
#include "pins.h"

#define USERS_FILE "/users.json"
#define LOGS_FILE  "/queue.jsonl"

void STM32AttendanceRepository::init() {
    // 1. Explicitly isolate MicroSD CS pin to prevent shared SPI1 bus contention
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);

    _hasSD = SD.begin(PIN_SD_CS);
    if (_hasSD) {
        LOG_INFO("STM32: SD Card initialized successfully.");
        if (!SD.exists(USERS_FILE)) {
            writeFile(USERS_FILE, "{}");
        }
        if (!SD.exists(LOGS_FILE)) {
            writeFile(LOGS_FILE, "");
        }
    } else {
        // Guarantee CS remains driven HIGH so it never clamps SPI1 MOSI/MISO/SCK
        digitalWrite(PIN_SD_CS, HIGH);
        LOG_WARN("STM32: SD Card not found or failed to mount on CS pin %d.", PIN_SD_CS);
        LOG_WARN("STM32: Operating in volatile in-RAM repository mode (SPI1 isolated).");
        _ramUsersJson = "{}";
        _ramLogsJsonl = "";
    }
}

String STM32AttendanceRepository::readFile(const char* path) {
    if (!_hasSD) return "{}";
    File file = SD.open(path, FILE_READ);
    if (!file) return "{}";
    String content = file.readString();
    file.close();
    return content;
}

bool STM32AttendanceRepository::writeFile(const char* path, const String& content) {
    if (!_hasSD) return false;
    File file = SD.open(path, FILE_WRITE);
    if (!file) return false;
    size_t written = file.print(content);
    file.close();
    return written == content.length();
}

bool STM32AttendanceRepository::addStudent(const String& uid, const String& name) {
    if (!_hasSD) {
        JsonDocument doc;
        deserializeJson(doc, _ramUsersJson);
        doc[uid] = name;
        _ramUsersJson = "";
        serializeJson(doc, _ramUsersJson);
        return true;
    }

    String current = readFile(USERS_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, current);
    if (err) {
        doc.clear();
    }
    doc[uid] = name;

    String tempFile = "/users.tmp";
    File file = SD.open(tempFile.c_str(), FILE_WRITE);
    if (!file) return false;
    if (serializeJson(doc, file) == 0) {
        file.close();
        SD.remove(tempFile.c_str());
        return false;
    }
    file.close();

    SD.remove(USERS_FILE);
    // SD.h might not have rename, fallback to rewrite if rename unsupported
    File src = SD.open(tempFile.c_str(), FILE_READ);
    File dst = SD.open(USERS_FILE, FILE_WRITE);
    if (src && dst) {
        while (src.available()) {
            dst.write(src.read());
        }
        src.close();
        dst.close();
        SD.remove(tempFile.c_str());
        return true;
    }
    if (src) src.close();
    if (dst) dst.close();
    return false;
}

bool STM32AttendanceRepository::deleteStudent(const String& uid) {
    if (!_hasSD) {
        JsonDocument doc;
        deserializeJson(doc, _ramUsersJson);
        if (!doc.containsKey(uid)) return false;
        doc.remove(uid);
        _ramUsersJson = "";
        serializeJson(doc, _ramUsersJson);
        return true;
    }

    String current = readFile(USERS_FILE);
    JsonDocument doc;
    deserializeJson(doc, current);
    if (!doc.containsKey(uid)) return false;
    doc.remove(uid);

    File dst = SD.open(USERS_FILE, FILE_WRITE);
    if (!dst) return false;
    serializeJson(doc, dst);
    dst.close();
    return true;
}

String STM32AttendanceRepository::getStudentName(const String& uid) {
    JsonDocument doc;
    if (!_hasSD) {
        deserializeJson(doc, _ramUsersJson);
    } else {
        String current = readFile(USERS_FILE);
        deserializeJson(doc, current);
    }
    if (doc.containsKey(uid)) {
        return doc[uid].as<String>();
    }
    return "";
}

String STM32AttendanceRepository::getStudents() {
    if (!_hasSD) {
        return _ramUsersJson;
    }
    return readFile(USERS_FILE);
}

bool STM32AttendanceRepository::markAttendance(const String& uid, const String& name, unsigned long timestamp) {
    JsonDocument doc;
    doc["uid"] = uid;
    doc["name"] = name;
    doc["timestamp"] = timestamp;

    String jsonLine;
    serializeJson(doc, jsonLine);

    if (!_hasSD) {
        _ramLogsJsonl += jsonLine + "\n";
        return true;
    }

    File file = SD.open(LOGS_FILE, FILE_WRITE);
    if (!file) return false;
    file.seek(file.size()); // Append to end
    file.println(jsonLine);
    file.close();
    return true;
}

void STM32AttendanceRepository::forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) {
    if (!callback) return;

    if (!_hasSD) {
        int start = 0;
        while (start < (int)_ramLogsJsonl.length()) {
            int end = _ramLogsJsonl.indexOf('\n', start);
            if (end == -1) end = _ramLogsJsonl.length();
            String line = _ramLogsJsonl.substring(start, end);
            line.trim();
            if (line.length() > 0) {
                callback(line.c_str(), context);
            }
            start = end + 1;
        }
        return;
    }

    File file = SD.open(LOGS_FILE, FILE_READ);
    if (!file) return;

    String line = "";
    while (file.available()) {
        char c = file.read();
        if (c == '\n') {
            line.trim();
            if (line.length() > 0) {
                callback(line.c_str(), context);
            }
            line = "";
        } else if (c != '\r') {
            line += c;
        }
    }
    line.trim();
    if (line.length() > 0) {
        callback(line.c_str(), context);
    }
    file.close();
}

bool STM32AttendanceRepository::clearAttendanceLogs() {
    if (!_hasSD) {
        _ramLogsJsonl = "";
        return true;
    }
    if (SD.exists(LOGS_FILE)) {
        SD.remove(LOGS_FILE);
    }
    return writeFile(LOGS_FILE, "");
}

#endif // ARDUINO_ARCH_STM32


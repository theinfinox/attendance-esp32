// ============================================================================
// cli_manager.cpp — Serial Command Line Interface
// ============================================================================

#include "cli_manager.h"
#include "logger.h"

void CliManager::init(IAttendanceRepository* repo) {
    _repo = repo;
    _inputBuffer.reserve(64);
    LOG_INFO("CLI Manager initialized. Type HELP for commands.");
}

String CliManager::update() {
    String unhandled = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (_inputBuffer.length() > 0) {
                if (!processCommand(_inputBuffer)) {
                    if (unhandled.length() == 0) unhandled = _inputBuffer;
                }
                _inputBuffer = "";
            }
        } else {
            _inputBuffer += c;
            // Prevent buffer overflow from garbage data
            if (_inputBuffer.length() > 100) {
                _inputBuffer = "";
            }
        }
    }
    return unhandled;
}

bool CliManager::processCommand(const String& cmd) {
    String upperCmd = cmd;
    upperCmd.trim();
    upperCmd.toUpperCase();

    if (upperCmd == "HELP") {
        printHelp();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (upperCmd == "DUMP_USERS") {
        dumpUsers();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (upperCmd == "DUMP_LOGS") {
        dumpLogs();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (upperCmd == "CLEAR_LOGS") {
        clearLogs();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } 
    return false;
}

void CliManager::printHelp() {
    Serial.println("=== CLI Commands ===");
    Serial.println("HELP       - Show this message");
    Serial.println("DUMP_USERS - Print the raw users.json data");
    Serial.println("DUMP_LOGS  - Stream attendance logs (queue.jsonl)");
    Serial.println("CLEAR_LOGS - Clear all attendance logs");
    Serial.println("====================");
}

void CliManager::dumpUsers() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    Serial.println("--- BEGIN USERS ---");
    Serial.println(_repo->getStudents());
    Serial.println("--- END USERS ---");
}

void CliManager::printLogLine(const char* jsonLine, void* context) {
    (void)context;
    if (jsonLine) {
        Serial.println(jsonLine);
    }
}

void CliManager::dumpLogs() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    Serial.println("--- BEGIN LOGS ---");
    _repo->forEachAttendance(printLogLine, nullptr);
    Serial.println("--- END LOGS ---");
}

void CliManager::clearLogs() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    if (_repo->clearAttendanceLogs()) {
        Serial.println("OK: Logs cleared.");
        LOG_INFO("Attendance logs cleared via CLI");
    } else {
        Serial.println("ERR: Failed to clear logs.");
    }
}

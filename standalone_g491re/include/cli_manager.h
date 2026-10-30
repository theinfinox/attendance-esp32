#pragma once
// ============================================================================
// cli_manager.h — Serial Command Line Interface
// Hardware-agnostic CLI for data extraction over standard Serial.
// ============================================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include "IAttendanceRepository.h"
#include "IRFID.h"

class CliManager {
public:
    void init(IAttendanceRepository* repo, IRFID* rfid = nullptr);
    String update(); // Called from loop() to process incoming serial data, returns unhandled input

private:
    IAttendanceRepository* _repo = nullptr;
    IRFID*                 _rfid = nullptr;
    String _inputBuffer = "";

    bool processCommand(const String& cmd);
    void printHelp();
    void dumpUsers();
    void dumpLogs();
    void clearLogs();
    void printStatus();
    void addUser(const String& jsonPayload);
    void deleteUser(const String& uid);

    // Callback used for streaming logs without high RAM overhead
    static void printLogLine(const char* jsonLine, void* context);
};

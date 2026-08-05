#pragma once
// ============================================================================
// web_manager.h — Embedded Dashboard Server
// ============================================================================

#include <Arduino.h>
#include <WebServer.h>
#include "IAttendanceRepository.h"
#include "IRFID.h"

class RfidManager;

class WebManager {
public:
    WebManager() = default;
    void init(IAttendanceRepository* repo, RfidManager* rfid);
    void update(); // call in main loop to handle clients

private:
    WebServer    _server{80};
    IAttendanceRepository* _repo = nullptr;
    RfidManager* _rfid = nullptr;

    void setupRoutes();
    void handleRoot();
    void handleGetUsers();
    void handlePostUsers();
    void handleDeleteUsers();
    void handleGetLogs();
    void handleDeleteLogs();
    void handleGetLastScan();
};

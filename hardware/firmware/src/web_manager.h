#pragma once
// ============================================================================
// web_manager.h — Embedded Dashboard Server
// ============================================================================

#include <Arduino.h>
#include <WebServer.h>
#include "storage.h"
#include "rfid_manager.h"

class WebManager {
public:
    void init(Storage* storage, RfidManager* rfid);
    void update(); // call in main loop to handle clients

private:
    WebServer    _server{80};
    Storage*     _storage = nullptr;
    RfidManager* _rfid    = nullptr;

    void setupRoutes();
    void handleRoot();
    void handleGetUsers();
    void handlePostUsers();
    void handleDeleteUsers();
    void handleGetLogs();
    void handleDeleteLogs();
    void handleGetLastScan();
};

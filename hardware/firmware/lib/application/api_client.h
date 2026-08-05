#pragma once
// ============================================================================
// api_client.h — Backend API Client (Mocked)
// Abstracts all backend communication. Currently returns mock data.
// Swap internals to real HTTPS later — no other module changes needed.
// ============================================================================

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "constants.h"
#include "IAttendanceRepository.h"

class ApiClient {
public:
    void init(IAttendanceRepository* storage);

    // Submit attendance for a given UID
    AttendanceResponse submitAttendance(const String& uid);

    // Heartbeat check — is the backend alive?
    bool heartbeat();

    // Quick ping — can we reach the server?
    bool ping();

    // Is the client currently using mocked responses?
    bool isMocked() const { return _mocked; }

private:
    bool     _mocked      = true;
    uint8_t  _mockCounter = 0;  // Cycles through result types for demo
    IAttendanceRepository* _repo     = nullptr;

    // Mock implementations
    AttendanceResponse mockSubmitAttendance(const String& uid);
    bool               mockHeartbeat();
    bool               mockPing();
};

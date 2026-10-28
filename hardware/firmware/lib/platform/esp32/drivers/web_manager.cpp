#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
// ============================================================================
// web_manager.cpp — Embedded Dashboard Server
// ============================================================================

#include "web_manager.h"
#include "rfid_manager.h"
#include "logger.h"
#include "pins.h"
#include <ArduinoJson.h>

const char* INDEX_HTML PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Attendance</title>
    <style>
        :root {
            --bg: #f3f4f6;
            --surface: #ffffff;
            --primary: #3b82f6;
            --primary-hover: #2563eb;
            --danger: #ef4444;
            --danger-hover: #dc2626;
            --text-main: #111827;
            --text-muted: #6b7280;
            --border: #e5e7eb;
            --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -1px rgba(0, 0, 0, 0.06);
            --radius: 12px;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; 
            background: var(--bg); color: var(--text-main); line-height: 1.5; padding: 20px; 
        }
        .container { max-width: 800px; margin: 0 auto; }
        header { text-align: center; margin-bottom: 30px; }
        h1 { font-size: 1.8rem; font-weight: 700; color: var(--text-main); }
        h2 { font-size: 1.2rem; font-weight: 600; margin-bottom: 16px; color: var(--text-main); border-bottom: 2px solid var(--bg); padding-bottom: 8px;}
        .card { 
            background: var(--surface); border-radius: var(--radius); padding: 24px; 
            box-shadow: var(--shadow); margin-bottom: 24px; 
        }
        .form-group { margin-bottom: 16px; }
        label { display: block; font-size: 0.9rem; font-weight: 500; margin-bottom: 6px; color: var(--text-muted); }
        input { 
            width: 100%; padding: 12px; border: 1px solid var(--border); border-radius: 8px; 
            font-size: 1rem; transition: border-color 0.2s; outline: none;
        }
        input:focus { border-color: var(--primary); box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1); }
        .input-group { display: flex; gap: 8px; }
        .input-group input { flex: 1; }
        button { 
            padding: 12px 16px; border: none; border-radius: 8px; font-size: 1rem; font-weight: 500; 
            cursor: pointer; transition: background-color 0.2s; 
        }
        .btn-primary { background: var(--primary); color: white; width: 100%; }
        .btn-primary:hover { background: var(--primary-hover); }
        .btn-secondary { background: var(--border); color: var(--text-main); white-space: nowrap;}
        .btn-secondary:hover { background: #d1d5db; }
        .btn-danger { background: var(--danger); color: white; padding: 6px 12px; font-size: 0.85rem;}
        .btn-danger:hover { background: var(--danger-hover); }
        .table-responsive { overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; white-space: nowrap; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid var(--border); font-size: 0.95rem;}
        th { color: var(--text-muted); font-weight: 600; }
        tbody tr:last-child td { border-bottom: none; }
        .empty-state { text-align: center; color: var(--text-muted); padding: 20px; font-style: italic; }
        @media (max-width: 600px) {
            body { padding: 12px; }
            .card { padding: 16px; }
            .input-group { flex-direction: column; }
            .btn-secondary { width: 100%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Attendance Terminal</h1>
        </header>
        
        <div class="card">
            <h2>Register New Card</h2>
            <div class="form-group">
                <label for="uid">Card UID</label>
                <div class="input-group">
                    <input type="text" id="uid" placeholder="e.g. E3BBC914">
                    <button class="btn-secondary" onclick="fetchLastScan()">Fetch Last Scanned</button>
                </div>
            </div>
            <div class="form-group">
                <label for="name">Employee Name</label>
                <input type="text" id="name" placeholder="e.g. John Doe">
            </div>
            <button class="btn-primary" onclick="addStudent()">Assign Card</button>
        </div>

        <div class="card">
            <h2>Registered Users</h2>
            <div class="table-responsive">
                <table>
                    <thead><tr><th>UID</th><th>Name</th><th>Action</th></tr></thead>
                    <tbody id="usersTable"><tr><td colspan="3" class="empty-state">Loading...</td></tr></tbody>
                </table>
            </div>
        </div>

        <div class="card">
            <h2>Recent Attendance Logs</h2>
            <div style="margin-bottom: 12px; text-align: right;">
                <button class="btn-danger" onclick="clearAttendanceLogs()">Clear All Logs</button>
            </div>
            <div class="table-responsive">
                <table>
                    <thead><tr><th>Time</th><th>UID</th><th>Name</th></tr></thead>
                    <tbody id="logsTable"><tr><td colspan="3" class="empty-state">Loading...</td></tr></tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        function loadUsers() {
            fetch('/api/users').then(r => r.json()).then(data => {
                let html = '';
                const entries = Object.entries(data);
                if(entries.length === 0) {
                    html = '<tr><td colspan="3" class="empty-state">No users registered yet.</td></tr>';
                } else {
                    for (const [uid, name] of entries) {
                        html += `<tr><td>${uid}</td><td>${name}</td>
                                 <td><button class="btn-danger" onclick="deleteCard('${uid}')">Delete</button></td></tr>`;
                    }
                }
                document.getElementById('usersTable').innerHTML = html;
            });
        }
        
        function loadLogs() {
            fetch('/api/logs').then(r => r.json()).then(data => {
                let html = '';
                if(data.length === 0) {
                    html = '<tr><td colspan="3" class="empty-state">No logs recorded yet.</td></tr>';
                } else {
                    data.reverse().forEach(log => {
                        let d = log.time || log.ts || "";
                        if (typeof d !== 'string') d = String(d);
                        if (!isNaN(d) && d.trim() !== '') {
                            d = new Date(parseInt(d) * (d.length < 13 ? 1000 : 1)).toLocaleString();
                        } else {
                            let parsed = new Date(d);
                            if (!isNaN(parsed)) d = parsed.toLocaleString();
                        }
                        html += `<tr><td>${d}</td><td>${log.uid || ''}</td><td>${log.name || ''}</td></tr>`;
                    });
                }
                document.getElementById('logsTable').innerHTML = html;
            });
        }

        function addStudent() {
            const uid = document.getElementById('uid').value.trim();
            const name = document.getElementById('name').value.trim();
            if (!uid || !name) return alert('Please fill in both fields.');
            fetch('/api/users', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({uid, name})
            }).then(() => {
                document.getElementById('uid').value = '';
                document.getElementById('name').value = '';
                loadUsers();
            });
        }

        function deleteCard(uid) {
            if (!confirm('Are you sure you want to delete this card?')) return;
            fetch('/api/users', {
                method: 'DELETE',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({uid})
            }).then(() => loadUsers());
        }

        function fetchLastScan() {
            const btn = document.querySelector('.btn-secondary');
            const originalText = btn.innerText;
            btn.innerText = 'Fetching...';
            
            fetch('/api/last_scan').then(r => r.json()).then(data => {
                if(data.uid && data.uid !== "") {
                    document.getElementById('uid').value = data.uid;
                } else {
                    alert('No card scanned recently! Please tap a card on the terminal first.');
                }
            }).catch(e => alert('Failed to fetch from device.'))
              .finally(() => btn.innerText = originalText);
        }

        function clearAttendanceLogs() {
            if (!confirm('Are you sure you want to clear all attendance logs? This cannot be undone.')) return;
            fetch('/api/logs', { method: 'DELETE' }).then(() => loadLogs());
        }

        loadUsers();
        loadLogs();
        setInterval(loadLogs, 5000);
    </script>
</body>
</html>
)=====";

void WebManager::init(IAttendanceRepository* repo, RfidManager* rfid) {
    _repo = repo;
    _rfid = rfid;
    setupRoutes();
    _server.begin();
    LOG_INFO("Web Manager initialized on Port 80");
}

void WebManager::update() {
    _server.handleClient();
}

void WebManager::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/api/users", HTTP_GET, [this]() { handleGetUsers(); });
    _server.on("/api/users", HTTP_POST, [this]() { handlePostUsers(); });
    _server.on("/api/users", HTTP_DELETE, [this]() { handleDeleteUsers(); });
    _server.on("/api/logs", HTTP_GET, [this]() { handleGetLogs(); });
    _server.on("/api/logs", HTTP_DELETE, [this]() { handleDeleteLogs(); });
    _server.on("/api/last_scan", HTTP_GET, [this]() { handleGetLastScan(); });
}

void WebManager::handleRoot() {
    _server.send(200, "text/html", INDEX_HTML);
}

void WebManager::handleGetUsers() {
    _server.send(200, "application/json", _repo->getStudents());
}

void WebManager::handlePostUsers() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "text/plain", "Body required");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, _server.arg("plain"))) {
        _server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    String uid = doc["uid"];
    String name = doc["name"];
    if (_repo->addStudent(uid, name)) {
        _server.send(200, "text/plain", "OK");
    } else {
        _server.send(500, "text/plain", "Error writing DB");
    }
}

void WebManager::handleDeleteUsers() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "text/plain", "Body required");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, _server.arg("plain"))) {
        _server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    String uid = doc["uid"];
    if (_repo->deleteStudent(uid)) {
        _server.send(200, "text/plain", "OK");
    } else {
        _server.send(500, "text/plain", "Error writing DB");
    }
}

void WebManager::handleGetLogs() {
    _server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    _server.send(200, "application/json", "[");
    
    struct LogContext {
        WebServer* server;
        bool first;
    };
    LogContext ctx = {&_server, true};
    
    _repo->forEachAttendance([](const char* jsonLine, void* context) {
        LogContext* ctx = static_cast<LogContext*>(context);
        if (!ctx->first) {
            ctx->server->sendContent(",");
        }
        ctx->server->sendContent(jsonLine);
        ctx->first = false;
    }, &ctx);
    
    _server.sendContent("]");
    _server.sendContent(""); // Signals end of chunked response
}

void WebManager::handleDeleteLogs() {
    if (_repo->clearAttendanceLogs()) {
        _server.send(200, "text/plain", "OK");
    } else {
        _server.send(500, "text/plain", "Error clearing logs");
    }
}

void WebManager::handleGetLastScan() {
    String uid = _rfid->lastUID();
    _server.send(200, "application/json", "{\"uid\":\"" + uid + "\"}");
}

#endif // ARDUINO_ARCH_ESP32

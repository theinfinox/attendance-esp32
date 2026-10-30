import { escapeHtml, formatDate, getPillClass, sortAttendanceLogs } from "../js/utils.js";

const metric = (label, valueHtml, detail = "", key) => `<article class="card metric-card"><span class="metric-label">${label}</span><strong class="metric-value" data-metric="${key}">${valueHtml}</strong><span class="metric-detail">${escapeHtml(detail)}</span></article>`;

/** Render device summary cards and recent activity. */
export function renderDashboard(app) {
  const s = app.status || {};
  const storage = s.storage ?? s.storage_status ?? "Unknown";
  const rfid = s.rfid ?? s.rfid_status ?? "Unknown";

  const recentLogs = sortAttendanceLogs(app.logs || []).slice(0, 5);
  const recentRows = recentLogs.length ? recentLogs.map(log => {
    const statusVal = String(log.status ?? "").toUpperCase();
    const eventName = log.event ?? ({ IN:"Check-in", OUT:"Check-out" }[statusVal] ?? log.status ?? "Attendance");
    const pillClass = getPillClass(eventName || log.status);
    return `<tr><td>${escapeHtml(formatDate(log.timestamp ?? log.time ?? log.date))}</td><td><strong>${escapeHtml(log.name ?? "Unknown")}</strong></td><td>${escapeHtml(log.card ?? log.uid ?? "—")}</td><td><span class="${pillClass}">${escapeHtml(eventName)}</span></td></tr>`;
  }).join("") : `<tr><td colspan="4"><div class="empty-state" style="min-height:90px;"><span>No recent attendance activity recorded yet.</span></div></td></tr>`;

  const connBadge = `<span class="${getPillClass(app.connected ? 'connected' : 'offline')}">${app.connected ? "Connected" : "Offline"}</span>`;

  return `<div class="dashboard-grid">
    ${metric("Connection", connBadge, app.connected ? "USB serial at 115200 baud" : "Connect a device to begin", "connection")}
    ${metric("Registered users", escapeHtml(String(app.users.length || s.users || 0)), "Synced from device", "users")}
    ${metric("Attendance records", escapeHtml(String(app.logs.length || s.logs || 0)), "Stored attendance events", "logs")}
    ${metric("Current time", escapeHtml(formatDate(s.time || new Date())), app.connected ? "Device or local time" : "Local time", "time")}
    <article class="card wide-card"><div><h2>Device details</h2><p class="muted">Live information reported by the serial CLI.</p></div><div class="device-details"><div><span>Board type</span><strong data-device="board">${escapeHtml(app.board || s.board || "Waiting for device")}</strong></div><div><span>Firmware version</span><strong data-device="firmware">${escapeHtml(app.firmware || s.firmware || "—")}</strong></div><div><span>Storage status</span><strong data-device="storage">${escapeHtml(typeof storage === "object" ? JSON.stringify(storage) : storage)}</strong></div><div><span>RFID status</span><strong data-device="rfid">${escapeHtml(typeof rfid === "object" ? JSON.stringify(rfid) : rfid)}</strong></div></div></article>
    <article class="card wide-card"><div><h2>Quick actions</h2><p class="muted">Retrieve data from the device's serial CLI.</p></div><div class="action-row"><button class="button primary" data-action="users">Refresh users</button><button class="button" data-action="logs">Refresh logs</button><button class="button" data-command="HELP">View CLI help</button></div></article>
    <article class="card wide-card" style="grid-column: 1 / -1;">
      <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
        <div><h2>Recent activity</h2><p class="muted">Latest recorded attendance events.</p></div>
        <button class="button" data-nav="logs">View all logs</button>
      </div>
      <div class="table-wrap">
        <table class="data-table">
          <thead><tr><th>Time</th><th>User</th><th>RFID card</th><th>Event</th></tr></thead>
          <tbody>${recentRows}</tbody>
        </table>
      </div>
    </article>
  </div>`;
}

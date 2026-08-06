import { escapeHtml, formatDate } from "../js/utils.js";

const metric = (label, value, detail = "", key) => `<article class="card metric-card"><span class="metric-label">${label}</span><strong class="metric-value" data-metric="${key}">${escapeHtml(value)}</strong><span class="metric-detail">${escapeHtml(detail)}</span></article>`;
/** Render device summary cards. */
export function renderDashboard(app) {
  const s = app.status || {}; const storage = s.storage ?? s.storage_status ?? "Unknown"; const rfid = s.rfid ?? s.rfid_status ?? "Unknown";
  return `<div class="dashboard-grid">
    ${metric("Connection", app.connected ? "Connected" : "Offline", app.connected ? "USB serial at 115200 baud" : "Connect a device to begin", "connection")}
    ${metric("Registered users", app.users.length || s.users || 0, "Synced from device", "users")}
    ${metric("Attendance records", app.logs.length || s.logs || 0, "Stored attendance events", "logs")}
    ${metric("Current time", formatDate(s.time || new Date()), app.connected ? "Device or local time" : "Local time", "time")}
    <article class="card wide-card"><div><h2>Device details</h2><p class="muted">Live information reported by the serial CLI.</p></div><div class="device-details"><div><span>Board type</span><strong data-device="board">${escapeHtml(app.board || s.board || "Waiting for device")}</strong></div><div><span>Firmware version</span><strong data-device="firmware">${escapeHtml(app.firmware || s.firmware || "—")}</strong></div><div><span>Storage status</span><strong data-device="storage">${escapeHtml(typeof storage === "object" ? JSON.stringify(storage) : storage)}</strong></div><div><span>RFID status</span><strong data-device="rfid">${escapeHtml(typeof rfid === "object" ? JSON.stringify(rfid) : rfid)}</strong></div></div></article>
    <article class="card wide-card"><div><h2>Quick actions</h2><p class="muted">Retrieve data from the device's serial CLI.</p></div><div class="action-row"><button class="button primary" data-action="users">Refresh users</button><button class="button" data-action="logs">Refresh logs</button><button class="button" data-command="HELP">View CLI help</button></div></article>
  </div>`;
}

import { state } from "./state.js";
import { SerialTransport } from "./serial.js";
import { AttendanceProtocol } from "./protocol.js";
import { addConsoleLine, setPage, syncChrome, toast } from "./ui.js";
import { csv, download, formatDate, getPillClass } from "./utils.js";
import { renderDashboard } from "../pages/dashboard.js";
import { bindUsers, renderUsers } from "../pages/users.js";
import { bindLogs, renderLogs } from "../pages/logs.js";
import { bindConsole, renderConsole } from "../pages/console.js";

const serial = new SerialTransport();
const protocol = new AttendanceProtocol(serial);
const content = document.querySelector("#app-content");
let page = "dashboard";

function settings(app) { return `<div class="settings-grid"><section class="card setting-row"><div><h2>Colour theme</h2><p>Use a dark interface for low-light environments.</p></div><button class="button" id="theme-setting">${app.theme === "dark" ? "Use light mode" : "Use dark mode"}</button></section><section class="card setting-row"><div><h2>Automatic console scrolling</h2><p>Keep terminal output positioned at the latest message.</p></div><input class="toggle" id="autoscroll" type="checkbox" ${app.autoScroll ? "checked" : ""}></section><section class="card setting-row"><div><h2>Serial configuration</h2><p>Fixed to 115200 baud, 8 data bits, no parity, and one stop bit.</p></div><span class="pill">Web Serial</span></section><section class="card setting-row"><div><h2>App Updates</h2><p>Force the app to bypass the cache and fetch the latest version.</p></div><button class="button primary" id="force-update">Update App</button></section></div>`; }
function about() { return `<section class="card wide-card"><h2>Serial Attendance Manager</h2><p class="muted">A local-first Progressive Web App for ESP32 and STM32 attendance hardware. It communicates directly through the browser Web Serial API; no Wi-Fi, server, HTTP API, or WebSocket connection is used.</p><div class="device-details"><div><span>Transport</span><strong>USB Serial, 115200 baud</strong></div><div><span>Protocol</span><strong>Newline-delimited JSON CLI</strong></div><div><span>Installation</span><strong>Offline-capable PWA</strong></div><div><span>Browser support</span><strong>Chromium-based browsers</strong></div></div></section>`; }
function render() { const app = state.get(); syncChrome(); const views = { dashboard:renderDashboard, users:renderUsers, logs:renderLogs, console:renderConsole, settings, about }; content.innerHTML = views[page](app); bind(); }
function syncDashboard(app) {
  if (page !== "dashboard") return;
  const s = app.status || {};
  const set = (selector, value) => { const node = content.querySelector(selector); if (node) node.textContent = value; };
  const connNode = content.querySelector("[data-metric='connection']");
  if (connNode) {
    connNode.innerHTML = `<span class="${getPillClass(app.connected ? 'connected' : 'offline')}">${app.connected ? "Connected" : "Offline"}</span>`;
  }
  set("[data-metric='users']", String(app.users.length || s.users || 0));
  set("[data-metric='logs']", String(app.logs.length || s.logs || 0));
  set("[data-metric='time']", formatDate(s.time || new Date()));
  set("[data-device='board']", app.board || s.board || "Waiting for device");
  set("[data-device='firmware']", app.firmware || s.firmware || "—");
  set("[data-device='storage']", typeof (s.storage ?? s.storage_status) === "object" ? JSON.stringify(s.storage ?? s.storage_status) : (s.storage ?? s.storage_status ?? "Unknown"));
  set("[data-device='rfid']", typeof (s.rfid ?? s.rfid_status) === "object" ? JSON.stringify(s.rfid ?? s.rfid_status) : (s.rfid ?? s.rfid_status ?? "Unknown"));
}
function appendLiveLog(entry) {
  if (page !== "logs") return;
  const body = content.querySelector(".data-table tbody");
  const search = content.querySelector("#log-search")?.value.trim().toLowerCase() || "";
  const filter = content.querySelector("#log-filter")?.value || "all";
  const onFirstPage = content.querySelector(".page-indicator")?.textContent.trim().startsWith("1 /");
  if (!body || !onFirstPage || body.querySelector(".empty-state")) {
    if (body?.querySelector(".empty-state")) render();
    return;
  }
  const statusVal = String(entry.status ?? "").toUpperCase();
  const event = entry.event ?? ({ IN:"Check-in", OUT:"Check-out" }[statusVal] ?? "Attendance");
  const haystack = Object.values(entry).join(" ").toLowerCase();
  if ((search && !haystack.includes(search)) || (filter !== "all" && event.toLowerCase() !== filter)) return;
  const pillClass = getPillClass(event || entry.status);
  const row = document.createElement("tr");
  [formatDate(entry.timestamp ?? entry.time), entry.name ?? "Unknown", entry.card ?? entry.uid ?? "—", event].forEach((value, index) => {
    const cell = document.createElement("td");
    if (index === 1) {
      const strong = document.createElement("strong");
      strong.textContent = value;
      cell.append(strong);
    } else if (index === 3) {
      const pill = document.createElement("span");
      pill.className = pillClass;
      pill.textContent = value;
      cell.append(pill);
    } else cell.textContent = value;
    row.append(cell);
  });
  body.prepend(row);
  const rows = body.querySelectorAll("tr");
  if (rows.length > 12) rows[rows.length - 1].remove();
  const count = content.querySelector(".pagination > span");
  if (count) count.textContent = `${state.get().logs.length} record${state.get().logs.length === 1 ? "" : "s"}`;
}
function bind() {
  const app = state.get();
  if (page === "dashboard") {
    content.querySelectorAll("[data-command]").forEach(button => button.addEventListener("click", () => command(button.dataset.command)));
    content.querySelector("[data-action='users']")?.addEventListener("click", users);
    content.querySelector("[data-action='logs']")?.addEventListener("click", logs);
    content.querySelectorAll("[data-nav='logs']").forEach(button => button.addEventListener("click", () => {
      page = "logs";
      setPage("logs");
      render();
    }));
  } if (page === "users") bindUsers(content, { render, refresh:users, add:addUser, remove:removeUser, fetchLast: () => state.get().lastScannedUid, export:() => download("attendance-users.json", JSON.stringify(app.users, null, 2)) }); if (page === "logs") bindLogs(content, { render, refresh:logs, clear, exportJson:() => download("attendance-logs.json", JSON.stringify(app.logs, null, 2)), exportCsv:() => download("attendance-logs.csv", csv(app.logs), "text/csv;charset=utf-8") }); if (page === "console") bindConsole(content, raw); if (page === "settings") { content.querySelector("#theme-setting").addEventListener("click", theme); content.querySelector("#autoscroll").addEventListener("change", event => state.set({ autoScroll:event.target.checked })); content.querySelector("#force-update")?.addEventListener("click", async () => { if ('serviceWorker' in navigator) { const regs = await navigator.serviceWorker.getRegistrations(); for (let reg of regs) await reg.unregister(); } if ('caches' in window) { const keys = await caches.keys(); for (let key of keys) await caches.delete(key); } window.location.reload(); }); } }
function theme() { const next = state.get().theme === "dark" ? "light" : "dark"; localStorage.setItem("attendance-theme", next); state.set({ theme:next }); if (page === "settings") render(); }
function status(message) { const data = message.data && typeof message.data === "object" ? { ...message, ...message.data } : message; state.set({ status:data, board:data.board ?? data.board_type ?? state.get().board, firmware:data.firmware ?? data.version ?? state.get().firmware }); }
function records(items, plural) {
  const singular = plural.slice(0, -1);
  const values = items.flatMap(item => Array.isArray(item) ? item : [item]);
  const wrapper = values.find(item => item?.type === plural || item?.type === singular || Array.isArray(item?.[plural]));
  if (Array.isArray(wrapper?.data)) return wrapper.data;
  if (Array.isArray(wrapper?.[plural])) return wrapper[plural];
  const rows = values.filter(item => item && item.type !== "done" && item.type !== "status").map(item => item.data ?? item);
  if (plural !== "users" || rows.length !== 1 || Array.isArray(rows[0]) || typeof rows[0] !== "object") return rows;
  const root = rows[0];
  const nested = root[plural] ?? root.data;
  if (nested && typeof nested === "object" && !Array.isArray(nested)) return Object.entries(nested).map(([uid, value]) => typeof value === "object" ? { uid, ...value } : { uid, name:value });
  const knownFields = ["id", "user_id", "uid", "name", "display_name", "card", "rfid", "card_id"];
  if (knownFields.some(field => field in root)) return rows;
  const entries = Object.entries(root);
  if (!entries.length || !entries.every(([, value]) => value && (typeof value === "object" || typeof value === "string"))) return rows;
  return entries.map(([uid, value]) => typeof value === "object" ? { uid, ...value } : { uid, name:value });
}
async function command(text, options) { if (!state.get().connected) return toast("Connect a serial device first.", "error"); addConsoleLine(text, "out"); try { return await protocol.request(text, options); } catch (error) { addConsoleLine(error.message, "error"); toast(error.message, "error"); throw error; } }
async function users() { const reply = await command("DUMP_USERS", { timeout:15000, allowMultilineJson:true }); state.set({ users:records(reply, "users") }); if (page === "users") render(); toast("Users refreshed."); }
async function addUser(user) { try { const reply = await command(`ADD_USER ${JSON.stringify(user)}`, { timeout:12000 }); const failure = reply.find(item => item?.type === "error"); if (failure) throw new Error(failure.message || "The device rejected this user."); const confirmed = reply.find(item => item?.type === "user")?.data ?? user; const uid = confirmed.uid ?? confirmed.id ?? user.uid; state.set({ users:[...state.get().users.filter(item => (item.uid ?? item.id ?? item.card_id) !== uid), confirmed] }); render(); toast(`${confirmed.name ?? user.name} was added to the device.`); } catch (error) { toast(error.message, "error"); } }
async function removeUser(uid) { try { const reply = await command(`DELETE_USER ${uid}`, { timeout: 12000 }); const failure = reply.find(item => item?.type === "error"); if (failure) throw new Error(failure.message || "The device rejected the deletion."); state.set({ users: state.get().users.filter(item => (item.uid ?? item.id ?? item.card_id) !== uid) }); render(); toast("User was deleted from the device."); } catch (error) { toast(error.message, "error"); } }
async function logs() { const reply = await command("DUMP_LOGS", { streaming:true, timeout:20000 }); state.set({ logs:records(reply, "logs") }); if (page === "logs") render(); toast("Attendance logs refreshed."); }
async function clear() { if (!state.get().connected) return toast("Connect a serial device first.", "error"); if (!confirm("Clear all attendance logs from the device? This cannot be undone.")) return; await command("CLEAR_LOGS", { timeout:12000 }); state.set({ logs:[] }); if (page === "logs") render(); toast("Logs cleared."); }
async function raw(text) { if (!state.get().connected) return; addConsoleLine(text, "out"); try { await serial.write(text); } catch (error) { addConsoleLine(error.message, "error"); toast(error.message, "error"); } }
async function connect(port) { 
  if (state.get().connected) return serial.disconnect(); 
  state.set({ connecting:true }); 
  try { 
    await serial.connect(port); 
    state.set({ connected:true, connecting:false, selectedPort:serial.port }); 
    toast("Serial device connected. Fetching data..."); 
    
    (async () => {
      try { 
        await new Promise(r => setTimeout(r, 1500)); // Wait for MCU to boot if DTR triggered a reset
        const unixSec = Math.floor(Date.now() / 1000);
        const tzOffset = -new Date().getTimezoneOffset();
        try {
          await command(`SET_TIME ${unixSec} ${tzOffset}`, { timeout: 3000 });
        } catch (e) {
          console.warn("Firmware SET_TIME rejected or unsupported:", e);
        }
        await new Promise(r => setTimeout(r, 300));
        await command("STATUS", { timeout: 3000 }); 
        await new Promise(r => setTimeout(r, 500));
        await users(); 
        await new Promise(r => setTimeout(r, 500));
        await logs();
      } catch (e) {
        console.warn("Auto-sync failed or was interrupted:", e);
      }
    })();
    
  } catch (error) { 
    state.set({ connecting:false, connected:false, lastError:error.message }); 
    toast(error.message, "error"); 
  } 
}
async function reconnect() { const port = state.get().selectedPort; if (port) await connect(port); else await connect(); }

serial.addEventListener("disconnected", () => { const live = state.get().connected; state.set({ connected:false, connecting:false }); if (live && document.hasFocus()) document.querySelector("#reconnect-dialog").showModal(); });
serial.addEventListener("error", event => { const message = event.detail?.message || "Serial reader failed."; addConsoleLine(message, "error"); toast(message, "error"); });
function adaptFirmwareLine(line) {
  const patch = {};
  const firmware = line.match(/Firmware v([^\s]+)/);
  const device = line.match(/Device:\s*(.+)$/);
  if (firmware) patch.firmware = firmware[1];
  if (device) patch.board = device[1].trim();
  if (/PN532 RFID Manager initialized|State: .*→ READY/.test(line)) patch.status = { ...state.get().status, rfid:"READY" };
  if (Object.keys(patch).length) state.set(patch);
  const unknown = line.match(/Unknown Card Scanned:\s*([A-F0-9]+)/i) || line.match(/Processing card UID:\s*([A-F0-9]+)/i);
  if (unknown) state.set({ lastScannedUid: unknown[1].toUpperCase() });
}
protocol.addEventListener("malformed", event => {
  const line = event.detail.line;
  addConsoleLine(line, "in");
  adaptFirmwareLine(line);
});
protocol.addEventListener("message", event => {
  const message = event.detail;
  if (message?.type === "status") status(message);
  if (message?.type === "log") {
    const raw = message.data ?? message;
    const statusVal = String(raw.status ?? "").toUpperCase();
    const eventName = raw.event ?? ({ IN:"Check-in", OUT:"Check-out" }[statusVal] ?? raw.status ?? "Attendance");
    const entry = {
      ...raw,
      time: raw.timestamp ?? raw.time ?? raw.date ?? "",
      timestamp: raw.timestamp ?? raw.time ?? raw.date ?? "",
      name: raw.name ?? raw.user_name ?? raw.user ?? "Unknown",
      card: raw.card ?? raw.rfid ?? raw.rfid_uid ?? raw.card_id ?? raw.uid ?? "—",
      uid: raw.uid ?? raw.card ?? "—",
      event: eventName,
      status: raw.status ?? (eventName === "Check-out" ? "OUT" : "IN")
    };
    const currentLogs = state.get().logs;
    const isDup = currentLogs.some(item =>
      (entry.event_id && item.event_id === entry.event_id) ||
      (entry.timestamp && item.timestamp === entry.timestamp && (item.card === entry.card || item.uid === entry.uid))
    );
    if (!isDup) {
      state.set({ logs:[entry, ...currentLogs] });
      appendLiveLog(entry);
    }
  }
  addConsoleLine(JSON.stringify(message), "in");
});
state.subscribe(app => { syncChrome(); syncDashboard(app); });
document.querySelectorAll(".nav-item").forEach(button => button.addEventListener("click", () => { page = button.dataset.page; setPage(page); render(); document.querySelector("#sidebar").classList.remove("open"); document.querySelector("#backdrop").classList.remove("visible"); }));
document.querySelector("#connect-button").addEventListener("click", () => connect()); document.querySelector("#theme-button").addEventListener("click", theme);
document.querySelector("#menu-button").addEventListener("click", () => { document.querySelector("#sidebar").classList.add("open"); document.querySelector("#backdrop").classList.add("visible"); }); document.querySelector("#backdrop").addEventListener("click", () => { document.querySelector("#sidebar").classList.remove("open"); document.querySelector("#backdrop").classList.remove("visible"); });
document.querySelector("#reconnect-dialog").addEventListener("close", () => { if (document.querySelector("#reconnect-dialog").returnValue === "reconnect") reconnect(); });
if ("serviceWorker" in navigator) window.addEventListener("load", () => navigator.serviceWorker.register("./sw.js").catch(error => console.warn("Service worker registration failed", error)));
render();
setInterval(() => syncDashboard(state.get()), 1000);

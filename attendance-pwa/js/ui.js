import { state } from "./state.js";

const pages = { dashboard:["Dashboard", "Monitor your attendance device"], users:["Users", "Browse registered RFID users"], logs:["Attendance Logs", "Review recorded attendance events"], console:["Serial Console", "Send direct commands to the device"], settings:["Settings", "Configure your local application preferences"], about:["About", "Serial Attendance Manager"] };
/** Display a transient feedback message. */
export function toast(message, kind = "info") { const el = document.createElement("div"); el.className = `toast ${kind === "error" ? "error" : ""}`; el.textContent = message; document.querySelector("#toast-region").append(el); setTimeout(() => el.remove(), 4200); }
/** Add an incoming or outgoing terminal record. */
export function addConsoleLine(content, direction = "in") {
  const entry = { content, direction, time:new Date().toISOString() };
  state.set({ consoleLines:[...state.get().consoleLines, entry].slice(-600) });
  const terminal = document.querySelector("#terminal-output");
  if (!terminal) return;
  const row = document.createElement("div"); row.className = `terminal-line ${direction === "out" ? "out" : direction === "error" ? "error" : ""}`;
  const time = document.createElement("span"); time.className = "terminal-time"; time.textContent = new Date(entry.time).toLocaleTimeString([], { hour:"2-digit", minute:"2-digit" });
  const text = document.createElement("span"); text.className = "terminal-content"; text.textContent = `${direction === "out" ? "› " : ""}${content}`;
  row.append(time, text); terminal.append(row);
  if (state.get().autoScroll) terminal.scrollTop = terminal.scrollHeight;
}
/** Set nav UI elements and button state from central state. */
export function syncChrome() { const app = state.get(); document.documentElement.dataset.theme = app.theme; const live = app.connected; document.querySelectorAll(".status-dot").forEach(dot => dot.classList.toggle("connected", live)); document.querySelector("#sidebar-status").textContent = app.connecting ? "Connecting…" : live ? "Connected" : "Disconnected"; const button = document.querySelector("#connect-button"); button.disabled = app.connecting; button.querySelector("span:last-child").textContent = app.connecting ? "Connecting…" : live ? "Disconnect" : "Connect device"; document.querySelector("#theme-button").textContent = app.theme === "dark" ? "☀" : "☾"; }
/** Change the active navigation page. */
export function setPage(page) { const [title, subtitle] = pages[page] || pages.dashboard; document.querySelector("#page-title").textContent = title; document.querySelector("#page-subtitle").textContent = subtitle; document.querySelectorAll(".nav-item").forEach(item => item.classList.toggle("active", item.dataset.page === page)); document.querySelector("#sidebar").classList.remove("open"); document.querySelector("#backdrop").classList.remove("visible"); }

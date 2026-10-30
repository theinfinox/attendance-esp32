import { state } from "./state.js";

/** Escape a string before placing it in HTML. */
export function escapeHtml(value = "") { return String(value).replace(/[&<>'"]/g, char => ({ "&":"&amp;", "<":"&lt;", ">":"&gt;", "'":"&#039;", '"':"&quot;" })[char]); }
/** Format an ISO date or timestamp for the user's locale with optional Demo Time Mode display offset. */
export function formatDate(value) {
  if (!value) return "—";
  let date;
  if (value instanceof Date) {
    date = new Date(value.getTime());
  } else if (typeof value === "number" || (!Number.isNaN(Number(value)) && !String(value).includes("-") && !String(value).includes(":"))) {
    let num = Number(value);
    if (!Number.isFinite(num) || num <= 0) return "—";
    if (num < 10000000000) num *= 1000;
    date = new Date(num);
  } else {
    date = new Date(value);
  }

  // Invalid, zero, or uninitialized timestamps must remain '—' regardless of Demo Mode
  if (Number.isNaN(date.getTime()) || date.getFullYear() <= 1970) {
    return "—";
  }

  // Apply normalized Demo Time Mode offset in milliseconds strictly for UI rendering
  const appState = state.get ? state.get() : null;
  if (appState?.demoMode && typeof appState.demoOffsetMs === "number" && appState.demoOffsetMs !== 0) {
    date = new Date(date.getTime() + appState.demoOffsetMs);
  }

  return date.toLocaleString([], { dateStyle:"medium", timeStyle:"short" });
}
/** Normalize signed direction, hours, and minutes into a single total offset in milliseconds. */
export function calculateDemoOffsetMs(direction = "+", hours = 0, minutes = 0) {
  const sign = direction === "-" ? -1 : 1;
  const h = Math.max(0, parseInt(hours, 10) || 0);
  const m = Math.max(0, Math.min(59, parseInt(minutes, 10) || 0));
  return sign * ((h * 60 + m) * 60 * 1000);
}
/** Format compact badge string for topbar (e.g. 'DEMO TIME (+2h 00m)'). */
export function formatDemoBadge(offsetMs = 0) {
  const sign = offsetMs < 0 ? "-" : "+";
  const totalMin = Math.round(Math.abs(offsetMs) / 60000);
  const h = Math.floor(totalMin / 60);
  const m = totalMin % 60;
  return `DEMO TIME (${sign}${h}h ${m < 10 ? "0" + m : m}m)`;
}
/** Format human-readable offset summary for settings view. */
export function formatDemoOffsetSummary(offsetMs = 0) {
  if (!offsetMs) return "0 hours, 0 minutes (Actual time)";
  const sign = offsetMs < 0 ? "-" : "+";
  const totalMin = Math.round(Math.abs(offsetMs) / 60000);
  const h = Math.floor(totalMin / 60);
  const m = totalMin % 60;
  return `${sign}${h}h ${m}m (${sign === "+" ? "Forward into future" : "Backward into past"})`;
}
/** Determine CSS pill badge class from event or status string. */
export function getPillClass(eventOrStatus = "") {
  const str = String(eventOrStatus ?? "").trim().toLowerCase();
  if (!str) return "pill unknown";
  if (/check-in|\bin\b|entered|present|success|connected|online/i.test(str)) {
    return "pill check-in";
  }
  if (/check-out|\bout\b|departed|exit|error|failed|rejected|danger/i.test(str)) {
    return "pill check-out";
  }
  if (/duplicate|dup|cooldown|suppressed|already|warning/i.test(str)) {
    return "pill duplicate";
  }
  if (/unknown|offline|disconnected|unavailable|waiting|none|unregistered/i.test(str)) {
    return "pill unknown";
  }
  return "pill";
}
/** Trigger a browser download of serialised data. */
export function download(filename, content, type = "application/json") { const url = URL.createObjectURL(new Blob([content], { type })); const link = Object.assign(document.createElement("a"), { href:url, download:filename }); link.click(); setTimeout(() => URL.revokeObjectURL(url), 0); }
export function csv(rows) { if (!rows.length) return ""; const keys = [...new Set(rows.flatMap(row => Object.keys(row)))]; const quote = value => `"${String(value ?? "").replaceAll('"', '""')}"`; return [keys.join(","), ...rows.map(row => keys.map(key => quote(typeof row[key] === "object" ? JSON.stringify(row[key]) : row[key])).join(","))].join("\n"); }
export function debounce(callback, wait = 180) { let timer; return (...args) => { clearTimeout(timer); timer = setTimeout(() => callback(...args), wait); }; }

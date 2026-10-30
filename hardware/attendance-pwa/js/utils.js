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

/** Extract normalized numeric timestamp in milliseconds from an attendance record. */
export function getLogTimestampMs(record) {
  const val = record?.timestamp ?? record?.time ?? record?.date;
  if (!val) return 0;
  if (val instanceof Date) return val.getTime();
  let num = Number(val);
  if (!Number.isNaN(num) && Number.isFinite(num) && num > 0) {
    if (num < 10000000000) num *= 1000;
    return num;
  }
  const d = new Date(val);
  return Number.isNaN(d.getTime()) ? 0 : d.getTime();
}

/** Extract positive numeric event_id if present and valid. */
export function getLogEventId(record) {
  const id = record?.event_id ?? record?.id;
  if (typeof id === "number" && Number.isFinite(id) && id > 0) return id;
  if (typeof id === "string" && /^\d+$/.test(id.trim())) {
    const parsed = parseInt(id.trim(), 10);
    if (Number.isFinite(parsed) && parsed > 0) return parsed;
  }
  return null;
}

/**
 * Compare two attendance log records for descending order (newest first).
 * Prefers event_id descending when available; falls back to timestamp descending.
 */
export function compareAttendanceLogs(a, b) {
  const idA = getLogEventId(a);
  const idB = getLogEventId(b);

  if (idA !== null && idB !== null && idA !== idB) {
    return idB - idA;
  }

  const timeA = getLogTimestampMs(a);
  const timeB = getLogTimestampMs(b);

  if (timeA !== timeB) {
    return timeB - timeA;
  }

  if (idA !== null && idB === null) return -1;
  if (idA === null && idB !== null) return 1;

  return 0;
}

/**
 * Check whether two log records represent the exact same attendance event.
 * Matches by event_id if available on both, or by UID/card and timestamp.
 */
export function isDuplicateLog(a, b) {
  if (!a || !b) return false;

  const idA = getLogEventId(a);
  const idB = getLogEventId(b);
  if (idA !== null && idB !== null) {
    return idA === idB;
  }

  const timeA = getLogTimestampMs(a);
  const timeB = getLogTimestampMs(b);
  if (timeA > 0 && timeB > 0 && timeA === timeB) {
    const cardA = String(a.uid ?? a.card ?? a.rfid ?? "").trim().toUpperCase();
    const cardB = String(b.uid ?? b.card ?? b.rfid ?? "").trim().toUpperCase();
    if (cardA && cardB && cardA === cardB) {
      return true;
    }
  }

  return false;
}

/**
 * Merge and deduplicate an array of logs, preserving richer fields on match.
 */
export function deduplicateLogs(logs = []) {
  const result = [];
  for (const log of logs) {
    if (!log) continue;
    const existingIdx = result.findIndex(item => isDuplicateLog(item, log));
    if (existingIdx === -1) {
      result.push(log);
    } else {
      result[existingIdx] = {
        ...log,
        ...result[existingIdx],
        event_id: result[existingIdx].event_id ?? log.event_id,
        timestamp: result[existingIdx].timestamp ?? log.timestamp,
        time: result[existingIdx].time ?? log.time,
        date: result[existingIdx].date ?? log.date,
        session: result[existingIdx].session ?? log.session
      };
    }
  }
  return result;
}

/**
 * Sort attendance logs in consistent descending order (newest first).
 */
export function sortAttendanceLogs(logs = []) {
  return [...logs].sort(compareAttendanceLogs);
}


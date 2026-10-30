/** Escape a string before placing it in HTML. */
export function escapeHtml(value = "") { return String(value).replace(/[&<>'"]/g, char => ({ "&":"&amp;", "<":"&lt;", ">":"&gt;", "'":"&#039;", '"':"&quot;" })[char]); }
/** Format an ISO date or timestamp for the user's locale. */
export function formatDate(value) {
  if (!value) return "—";
  let date;
  if (value instanceof Date) {
    date = value;
  } else if (typeof value === "number" || (!Number.isNaN(Number(value)) && !String(value).includes("-") && !String(value).includes(":"))) {
    let num = Number(value);
    if (!Number.isFinite(num) || num <= 0) return "—";
    if (num < 10000000000) num *= 1000;
    date = new Date(num);
  } else {
    date = new Date(value);
  }
  if (Number.isNaN(date.getTime()) || date.getFullYear() <= 1970) {
    return "—";
  }
  return date.toLocaleString([], { dateStyle:"medium", timeStyle:"short" });
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

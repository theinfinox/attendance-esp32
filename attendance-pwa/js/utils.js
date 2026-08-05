/** Escape a string before placing it in HTML. */
export function escapeHtml(value = "") { return String(value).replace(/[&<>'"]/g, char => ({ "&":"&amp;", "<":"&lt;", ">":"&gt;", "'":"&#039;", '"':"&quot;" })[char]); }
/** Format an ISO date or timestamp for the user's locale. */
export function formatDate(value) { if (!value) return "—"; const date = new Date(value); return Number.isNaN(date.getTime()) ? String(value) : date.toLocaleString([], { dateStyle:"medium", timeStyle:"short" }); }
/** Trigger a browser download of serialised data. */
export function download(filename, content, type = "application/json") { const url = URL.createObjectURL(new Blob([content], { type })); const link = Object.assign(document.createElement("a"), { href:url, download:filename }); link.click(); setTimeout(() => URL.revokeObjectURL(url), 0); }
export function csv(rows) { if (!rows.length) return ""; const keys = [...new Set(rows.flatMap(row => Object.keys(row)))]; const quote = value => `"${String(value ?? "").replaceAll('"', '""')}"`; return [keys.join(","), ...rows.map(row => keys.map(key => quote(typeof row[key] === "object" ? JSON.stringify(row[key]) : row[key])).join(","))].join("\n"); }
export function debounce(callback, wait = 180) { let timer; return (...args) => { clearTimeout(timer); timer = setTimeout(() => callback(...args), wait); }; }

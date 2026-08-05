import { escapeHtml } from "../js/utils.js";

let view = { query:"", sort:"name", page:1, size:10 };
const field = user => ({ id:user.id ?? user.uid ?? user.card_id ?? "—", name:user.name ?? user.display_name ?? "Unnamed", card:user.card ?? user.rfid ?? user.card_id ?? "—", role:user.role ?? "User" });
/** Render the local user collection, including client-side search, sort and pagination. */
export function renderUsers(app) {
  const items = app.users.map(field).filter(user => Object.values(user).join(" ").toLowerCase().includes(view.query.toLowerCase())).sort((a,b) => String(a[view.sort]).localeCompare(String(b[view.sort])));
  const pages = Math.max(1, Math.ceil(items.length / view.size)); view.page = Math.min(view.page, pages); const shown = items.slice((view.page - 1) * view.size, view.page * view.size);
  const rows = shown.length ? shown.map(user => `<tr><td>${escapeHtml(user.id)}</td><td><strong>${escapeHtml(user.name)}</strong></td><td>${escapeHtml(user.card)}</td><td><span class="pill">${escapeHtml(user.role)}</span></td></tr>`).join("") : `<tr><td colspan="4"><div class="empty-state"><strong>No users to display</strong><span>Connect a device and press Refresh to load users.</span></div></td></tr>`;
  return `<div class="toolbar"><div class="toolbar-left"><input id="user-search" class="input search-input" value="${escapeHtml(view.query)}" placeholder="Search users"><select id="user-sort" class="select"><option value="name">Sort by name</option><option value="id">Sort by ID</option><option value="role">Sort by role</option></select></div><div class="toolbar-right"><button class="button" id="export-users">Export JSON</button><button class="button" id="import-users" title="Reserved for device firmware with import support" disabled>Import JSON</button><button class="button primary" id="refresh-users">Refresh</button></div></div><section class="card table-card"><div class="table-wrap"><table class="data-table"><thead><tr><th>ID</th><th>Name</th><th>RFID card</th><th>Role</th></tr></thead><tbody>${rows}</tbody></table></div><div class="pagination"><span>${items.length} user${items.length === 1 ? "" : "s"}</span><div class="pagination-controls"><button class="button" data-users-page="prev" ${view.page === 1 ? "disabled" : ""}>Previous</button><span class="page-indicator">${view.page} / ${pages}</span><button class="button" data-users-page="next" ${view.page === pages ? "disabled" : ""}>Next</button></div></div></section>`;
}
export function bindUsers(root, handlers) {
  root.querySelector("#user-sort").value = view.sort;
  root.querySelector("#user-search")?.addEventListener("input", event => { view.query = event.target.value; view.page = 1; handlers.render(); });
  root.querySelector("#user-sort")?.addEventListener("change", event => { view.sort = event.target.value; handlers.render(); });
  root.querySelectorAll("[data-users-page]").forEach(button => button.addEventListener("click", () => { view.page += button.dataset.usersPage === "next" ? 1 : -1; handlers.render(); }));
  root.querySelector("#refresh-users")?.addEventListener("click", handlers.refresh);
  root.querySelector("#export-users")?.addEventListener("click", handlers.export);
}

import { escapeHtml } from "../js/utils.js";

let view = { query:"", sort:"name", page:1, size:10 };
const field = user => {
  const value = user && typeof user === "object" ? user : { name:String(user ?? "") };
  const id = value.id ?? value.user_id ?? value.userId ?? value.uid ?? value.rfid_uid ?? value.rfidUid ?? value.rfidId ?? value.card_id ?? value.cardId ?? "—";
  return {
    id,
    name:value.name ?? value.display_name ?? value.displayName ?? value.full_name ?? value.fullName ?? value.username ?? value.student_name ?? "Unnamed",
    card:value.card ?? value.rfid ?? value.rfid_uid ?? value.rfidUid ?? value.rfidId ?? value.card_id ?? value.cardId ?? value.uid ?? id,
    role:value.role ?? value.user_type ?? value.type ?? value.department ?? "User"
  };
};
/** Render the local user collection, including client-side search, sort and pagination. */
export function renderUsers(app) {
  const items = app.users.map(field).filter(user => Object.values(user).join(" ").toLowerCase().includes(view.query.toLowerCase())).sort((a,b) => String(a[view.sort]).localeCompare(String(b[view.sort])));
  const pages = Math.max(1, Math.ceil(items.length / view.size)); view.page = Math.min(view.page, pages); const shown = items.slice((view.page - 1) * view.size, view.page * view.size);
  const rows = shown.length ? shown.map(user => `<tr><td>${escapeHtml(user.id)}</td><td><strong>${escapeHtml(user.name)}</strong></td><td>${escapeHtml(user.card)}</td><td><span class="pill">${escapeHtml(user.role)}</span></td><td style="text-align:right;"><button class="icon-button delete-user-btn" data-uid="${escapeHtml(user.id)}" title="Delete user" style="color:var(--danger);">🗑</button></td></tr>`).join("") : `<tr><td colspan="5"><div class="empty-state"><strong>No users to display</strong><span>Connect a device and press Refresh to load users.</span></div></td></tr>`;
  return `<div class="toolbar"><div class="toolbar-left"><input id="user-search" class="input search-input" value="${escapeHtml(view.query)}" placeholder="Search users"><select id="user-sort" class="select"><option value="name">Sort by name</option><option value="id">Sort by ID</option><option value="role">Sort by role</option></select></div><div class="toolbar-right"><button class="button" id="export-users">Export JSON</button><button class="button" id="import-users" title="Reserved for device firmware with import support" disabled>Import JSON</button><button class="button primary" id="add-user">Add user</button><button class="button" id="refresh-users">Refresh</button></div></div><section class="card table-card"><div class="table-wrap"><table class="data-table"><thead><tr><th>ID</th><th>Name</th><th>RFID card</th><th>Role</th><th style="width:40px;"></th></tr></thead><tbody>${rows}</tbody></table></div><div class="pagination"><span>${items.length} user${items.length === 1 ? "" : "s"}</span><div class="pagination-controls"><button class="button" data-users-page="prev" ${view.page === 1 ? "disabled" : ""}>Previous</button><span class="page-indicator">${view.page} / ${pages}</span><button class="button" data-users-page="next" ${view.page === pages ? "disabled" : ""}>Next</button></div></div></section><dialog class="modal" id="add-user-dialog"><form id="add-user-form"><h2>Add RFID user</h2><p>Enter the UID printed by the card reader, then save it to the device.</p><div class="form-grid"><label>RFID UID<div style="display:flex;gap:8px;"><input class="input" name="uid" id="add-user-uid" style="flex:1;" inputmode="text" autocomplete="off" placeholder="e.g. E3BBC914" required maxlength="32" pattern="[A-Fa-f0-9]+"><button type="button" class="button" id="fetch-last-scan">Fetch Last</button></div></label><label>Name<input class="input" name="name" autocomplete="name" placeholder="e.g. Krishna" required maxlength="80"></label><label>Role<select class="select" name="role"><option>User</option><option>Student</option><option>Employee</option><option>Admin</option></select></label></div><p class="form-error" id="add-user-error" role="alert"></p><div class="modal-actions"><button type="button" class="button secondary" id="cancel-add-user">Cancel</button><button class="button primary" id="save-user">Save user</button></div></form></dialog>`;
}
export function bindUsers(root, handlers) {
  root.querySelector("#user-sort").value = view.sort;
  root.querySelector("#user-search")?.addEventListener("input", event => { view.query = event.target.value; view.page = 1; handlers.render(); });
  root.querySelector("#user-sort")?.addEventListener("change", event => { view.sort = event.target.value; handlers.render(); });
  root.querySelectorAll("[data-users-page]").forEach(button => button.addEventListener("click", () => { view.page += button.dataset.usersPage === "next" ? 1 : -1; handlers.render(); }));
  root.querySelector("#refresh-users")?.addEventListener("click", handlers.refresh);
  root.querySelector("#export-users")?.addEventListener("click", handlers.export);
  const dialog = root.querySelector("#add-user-dialog");
  root.querySelector("#add-user")?.addEventListener("click", () => dialog.showModal());
  root.querySelector("#cancel-add-user")?.addEventListener("click", () => dialog.close());
  root.querySelector("#fetch-last-scan")?.addEventListener("click", () => {
    const uid = handlers.fetchLast?.();
    if (uid) root.querySelector("#add-user-uid").value = uid;
    else root.querySelector("#add-user-error").textContent = "No recent card scan found. Please tap a card on the reader first.";
  });
  root.querySelector("#add-user-form")?.addEventListener("submit", async event => {
    event.preventDefault(); const form = new FormData(event.currentTarget); const uid = String(form.get("uid") ?? "").trim().toUpperCase(); const name = String(form.get("name") ?? "").trim(); const role = String(form.get("role") ?? "User"); const error = root.querySelector("#add-user-error");
    if (!/^[A-F0-9]{4,32}$/.test(uid)) { error.textContent = "Enter a hexadecimal RFID UID (4–32 characters)."; return; }
    if (!name) { error.textContent = "Enter a name."; return; }
    error.textContent = ""; const save = root.querySelector("#save-user"); save.disabled = true;
    try { await handlers.add({ uid, name, role }); } finally { save.disabled = false; }
  });
  root.querySelectorAll(".delete-user-btn").forEach(button => button.addEventListener("click", () => {
    const uid = button.dataset.uid;
    if (confirm(`Are you sure you want to delete user ${uid} from the device? This cannot be undone.`)) {
      handlers.remove(uid);
    }
  }));
}

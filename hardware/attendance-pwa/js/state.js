/** Central application state with a minimal subscription API. */
const initialState = {
  connected: false, connecting: false, board: "—", firmware: "—", users: [], logs: [],
  status: {}, consoleLines: [], theme: localStorage.getItem("attendance-theme") || "light",
  autoScroll: true, lastError: "", selectedPort: null
};
let data = structuredClone(initialState);
const listeners = new Set();

export const state = {
  get: () => data,
  set(patch) { data = { ...data, ...patch }; listeners.forEach(listener => listener(data)); },
  reset() { data = structuredClone(initialState); listeners.forEach(listener => listener(data)); },
  subscribe(listener) { listeners.add(listener); return () => listeners.delete(listener); }
};

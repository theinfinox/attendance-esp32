/** Central application state with a minimal subscription API. */
const savedDemoMode = typeof localStorage !== "undefined" && localStorage.getItem("attendance-demo-mode") === "true";
const savedDemoOffsetMs = typeof localStorage !== "undefined" ? (parseInt(localStorage.getItem("attendance-demo-offset-ms") || "0", 10) || 0) : 0;

const initialState = {
  connected: false, connecting: false, board: "—", firmware: "—", users: [], logs: [],
  status: {}, consoleLines: [], theme: (typeof localStorage !== "undefined" && localStorage.getItem("attendance-theme")) || "light",
  autoScroll: true, lastError: "", selectedPort: null,
  demoMode: savedDemoMode,
  demoOffsetMs: savedDemoOffsetMs
};
let data = structuredClone(initialState);
const listeners = new Set();

export const state = {
  get: () => data,
  set(patch) { data = { ...data, ...patch }; listeners.forEach(listener => listener(data)); },
  reset() { data = structuredClone(initialState); listeners.forEach(listener => listener(data)); },
  subscribe(listener) { listeners.add(listener); return () => listeners.delete(listener); }
};

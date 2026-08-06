/** CLI protocol adapter. Commands and stream parsing are serial-transport independent. */
export class AttendanceProtocol extends EventTarget {
  constructor(serial) { super(); this.serial = serial; this.pending = null; serial.addEventListener("line", event => this.receive(event.detail)); }
  receive(line) {
    let message;
    try { message = JSON.parse(line); }
    catch {
      const pending = this.pending;
      const diagnostic = /^\[(INFO|DEBUG|WARN|ERROR)\]/.test(line.trim());
      if (pending?.allowMultilineJson && !diagnostic) {
        pending.rawBuffer += `${pending.rawBuffer ? "\n" : ""}${line}`;
        try { message = JSON.parse(pending.rawBuffer); pending.rawBuffer = ""; }
        catch { return; }
      } else { this.dispatchEvent(new CustomEvent("malformed", { detail:{ line } })); return; }
    }
    this.handleMessage(message);
  }
  handleMessage(message) {
    this.dispatchEvent(new CustomEvent("message", { detail:message }));
    if (this.pending) {
      this.pending.items.push(message);
      if (message?.type === "done" || (!this.pending.streaming && message?.type !== "log" && message?.type !== "user")) this.finish();
      else if (this.pending.idleMs) this.resetIdleTimer();
    }
  }
  resetIdleTimer() { const pending = this.pending; if (!pending?.idleMs) return; clearTimeout(pending.idleTimer); pending.idleTimer = setTimeout(() => this.finish(), pending.idleMs); }
  finish(error) { const pending = this.pending; this.pending = null; clearTimeout(pending?.timer); clearTimeout(pending?.idleTimer); error ? pending?.reject(error) : pending?.resolve(pending.items); }
  /** Send one CLI request and collect its JSON response stream. The protocol is intentionally sequential. */
  request(command, { timeout = 8000, streaming = false, idleMs = 0, allowMultilineJson = false } = {}) {
    if (this.pending) return Promise.reject(new Error("A serial command is already in progress."));
    return new Promise(async (resolve, reject) => {
      const timer = setTimeout(() => this.finish(new Error(`Timed out waiting for ${command}.`)), timeout);
      this.pending = { resolve, reject, items:[], timer, idleMs, idleTimer:null, streaming, allowMultilineJson, rawBuffer:"" };
      try { await this.serial.write(command); if (idleMs) this.resetIdleTimer(); } catch (error) { this.finish(error); }
    });
  }
}

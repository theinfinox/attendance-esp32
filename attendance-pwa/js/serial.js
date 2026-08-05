/** Browser Web Serial transport. It deliberately knows nothing about the DOM or CLI response schema. */
export class SerialTransport extends EventTarget {
  constructor() { super(); this.port = null; this.reader = null; this.reading = false; }
  get connected() { return Boolean(this.port?.readable && this.port?.writable); }
  async connect(port) {
    if (!("serial" in navigator)) throw new Error("Web Serial is unavailable. Use a current Chromium-based browser over HTTPS or localhost.");
    this.port = port || await navigator.serial.requestPort();
    await this.port.open({ baudRate:115200, dataBits:8, stopBits:1, parity:"none", flowControl:"none" });
    this.reading = true; this.dispatchEvent(new Event("connected")); this.readLoop();
  }
  async readLoop() {
    const decoder = new TextDecoder(); let buffered = "";
    try {
      this.reader = this.port.readable.getReader();
      while (this.reading) {
        const { value, done } = await this.reader.read(); if (done) break;
        buffered += decoder.decode(value, { stream:true });
        const chunks = buffered.split(/\r?\n/); buffered = chunks.pop() ?? "";
        chunks.filter(Boolean).forEach(line => this.dispatchEvent(new CustomEvent("line", { detail:line })));
      }
      if (buffered.trim()) this.dispatchEvent(new CustomEvent("line", { detail:buffered.trim() }));
    } catch (error) { if (this.reading) this.dispatchEvent(new CustomEvent("error", { detail:error })); }
    finally { this.reader?.releaseLock(); this.reader = null; if (this.reading) this.handleDisconnect(); }
  }
  async write(text) { if (!this.connected) throw new Error("No serial device is connected."); const writer = this.port.writable.getWriter(); try { await writer.write(new TextEncoder().encode(`${text.replace(/[\r\n]+$/g, "")}\n`)); } finally { writer.releaseLock(); } }
  async disconnect() { const hadPort = this.port; this.reading = false; try { await this.reader?.cancel(); } catch { /* reader may already be closed */ } await this.closePort(); if (hadPort) this.dispatchEvent(new Event("disconnected")); }
  async handleDisconnect() { const hadPort = this.port; await this.closePort(); if (hadPort) this.dispatchEvent(new Event("disconnected")); }
  async closePort() { try { await this.port?.close(); } catch { /* device may have been removed */ } this.port = null; }
}

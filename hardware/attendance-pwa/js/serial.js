import { Ch340WebUsbTransport } from "./ch340.js";

/** Browser Web Serial transport with WebUSB fallback for Android. It deliberately knows nothing about the DOM or CLI response schema. */
export class SerialTransport extends EventTarget {
  constructor() { super(); this.port = null; this.reader = null; this.reading = false; this.fallback = null; }
  get connected() { return this.fallback ? this.fallback.connected : Boolean(this.port?.readable && this.port?.writable); }
  async connect(port) {
    if (!("serial" in navigator) && !("usb" in navigator)) throw new Error("Web Serial and WebUSB are unavailable.");
    
    try {
      if (!("serial" in navigator)) throw new DOMException("Web Serial API not available", "NotFoundError");
      this.port = port || await navigator.serial.requestPort();
      await this.port.open({ baudRate:115200, dataBits:8, stopBits:1, parity:"none", flowControl:"none" });
      this.reading = true; this.dispatchEvent(new Event("connected")); this.readLoop();
    } catch (err) {
      if (err.name === "NotFoundError" && "usb" in navigator) {
        this.fallback = new Ch340WebUsbTransport();
        this.fallback.addEventListener("connected", () => this.dispatchEvent(new Event("connected")));
        this.fallback.addEventListener("disconnected", () => this.dispatchEvent(new Event("disconnected")));
        this.fallback.addEventListener("line", e => this.dispatchEvent(new CustomEvent("line", { detail: e.detail })));
        this.fallback.addEventListener("error", e => this.dispatchEvent(new CustomEvent("error", { detail: e.detail })));
        await this.fallback.connect();
        return;
      }
      throw err;
    }
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
  async write(text) { 
    if (this.fallback) return this.fallback.write(text);
    if (!this.connected) throw new Error("No serial device is connected."); 
    const writer = this.port.writable.getWriter(); 
    try { await writer.write(new TextEncoder().encode(`${text.replace(/[\r\n]+$/g, "")}\n`)); } finally { writer.releaseLock(); } 
  }
  async disconnect() { 
    if (this.fallback) return this.fallback.disconnect();
    const hadPort = this.port; this.reading = false; try { await this.reader?.cancel(); } catch { /* reader may already be closed */ } await this.closePort(); if (hadPort) this.dispatchEvent(new Event("disconnected")); 
  }
  async handleDisconnect() { 
    if (this.fallback) return this.fallback.handleDisconnect();
    const hadPort = this.port; await this.closePort(); if (hadPort) this.dispatchEvent(new Event("disconnected")); 
  }
  async closePort() { 
    if (this.fallback) return this.fallback.closeDevice();
    try { await this.port?.close(); } catch { /* device may have been removed */ } this.port = null; 
  }
}

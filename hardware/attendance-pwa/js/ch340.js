/** WebUSB driver polyfill for CH340/CH341 USB-to-Serial chips (Vendor 0x1A86, Product 0x7523). */
export class Ch340WebUsbTransport extends EventTarget {
  constructor() {
    super();
    this.device = null;
    this.reading = false;
    this.decoder = new TextDecoder();
    this.encoder = new TextEncoder();
  }

  get connected() {
    return this.device !== null && this.device.opened;
  }

  async connect(device) {
    if (!("usb" in navigator)) throw new Error("WebUSB is unavailable.");
    
    // Request device if not provided
    this.device = device || await navigator.usb.requestDevice({ filters: [{ vendorId: 0x1a86 }] });
    
    await this.device.open();
    if (this.device.configuration === null) await this.device.selectConfiguration(1);
    await this.device.claimInterface(0);

    // Initialize CH340 settings (LCR, Handshake, and Baud Rate)
    await this.initSerial(115200);

    this.reading = true;
    this.dispatchEvent(new Event("connected"));
    this.readLoop();
  }

  async initSerial(baudRate) {
    // Basic LCR (Line Control Register): 8 bits, 1 stop bit, no parity (Register 0x2518 -> 0xc3)
    await this.device.controlTransferOut({ requestType: 'vendor', recipient: 'device', request: 0x9a, value: 0x2518, index: 0x00c3 });

    // Set Baud Rate to 115200 (using ch341 linux driver exact constants)
    await this.device.controlTransferOut({ requestType: 'vendor', recipient: 'device', request: 0x9a, value: 0x1312, index: 0xcc03 });
    await this.device.controlTransferOut({ requestType: 'vendor', recipient: 'device', request: 0x9a, value: 0x0f2c, index: 0x0008 });

    // Automatic Hardware Reset Pulse (ESP32 NodeMCU circuit)
    // To pull EN low (reset): RTS=1 (active), DTR=0 (inactive). 
    // Bit 5 is DTR (0x20), Bit 6 is RTS (0x40). Value is ~((DTR?0x20:0) | (RTS?0x40:0)).
    // RTS=1, DTR=0 -> ~0x40 = 0xFFBF
    await this.device.controlTransferOut({ requestType: 'vendor', recipient: 'device', request: 0xa4, value: 0xFFBF, index: 0 });
    await new Promise(r => setTimeout(r, 100)); // Hold reset for 100ms
    
    // Release Reset (RTS=0, DTR=0 -> 0xFFFF)
    await this.device.controlTransferOut({ requestType: 'vendor', recipient: 'device', request: 0xa4, value: 0xFFFF, index: 0 });
    await new Promise(r => setTimeout(r, 100)); // Wait for ESP32 to boot
  }

  async readLoop() {
    let buffered = "";
    try {
      while (this.reading && this.connected) {
        // CH340 uses Endpoint 2 IN for bulk data (packetSize 32)
        const result = await this.device.transferIn(2, 64);
        if (result.status !== "ok") continue;
        
        buffered += this.decoder.decode(result.data, { stream: true });
        const chunks = buffered.split(/\r?\n/);
        buffered = chunks.pop() ?? "";
        
        chunks.filter(Boolean).forEach(line => this.dispatchEvent(new CustomEvent("line", { detail: line })));
      }
    } catch (error) {
      if (this.reading) this.dispatchEvent(new CustomEvent("error", { detail: error }));
    } finally {
      if (this.reading) this.handleDisconnect();
    }
  }

  async write(text) {
    if (!this.connected) throw new Error("No WebUSB device is connected.");
    // CH340 uses Endpoint 2 OUT for bulk data
    await this.device.transferOut(2, this.encoder.encode(`${text.replace(/[\r\n]+$/g, "")}\n`));
  }

  async disconnect() {
    const hadDevice = this.device;
    this.reading = false;
    await this.closeDevice();
    if (hadDevice) this.dispatchEvent(new Event("disconnected"));
  }

  async handleDisconnect() {
    const hadDevice = this.device;
    await this.closeDevice();
    if (hadDevice) this.dispatchEvent(new Event("disconnected"));
  }

  async closeDevice() {
    try {
      if (this.device?.opened) {
        await this.device.releaseInterface(0);
        await this.device.close();
      }
    } catch { /* Ignore cleanup errors */ }
    this.device = null;
  }
}

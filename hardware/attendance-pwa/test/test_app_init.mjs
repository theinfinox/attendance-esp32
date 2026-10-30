// ============================================================================
// test_app_init.mjs — Verify DOM event registration and initialization
// ============================================================================

class MockElement {
  constructor(tag, id = '', className = '') {
    this.tagName = tag.toUpperCase();
    this.id = id;
    this.className = className;
    this.classList = {
      classes: new Set(className.split(' ').filter(Boolean)),
      add(c) { this.classes.add(c); },
      remove(c) { this.classes.delete(c); },
      contains(c) { return this.classes.has(c); }
    };
    this.children = [];
    this.listeners = {};
    this.innerHTML = '';
    this.textContent = '';
    this.style = {};
    this.dataset = {};
  }
  addEventListener(event, handler) {
    if (!this.listeners[event]) this.listeners[event] = [];
    this.listeners[event].push(handler);
  }
  dispatchEvent(event) {
    const list = this.listeners[event.type] || [];
    list.forEach(h => h(event));
  }
  querySelector(sel) {
    if (this.id === 'connect-button' && sel.includes('span')) return new MockElement('span');
    if (sel.startsWith('#')) {
      const id = sel.slice(1);
      if (this.id === id) return this;
      for (const ch of this.children) {
        const found = ch.querySelector(sel);
        if (found) return found;
      }
    }
    return null;
  }
  querySelectorAll(sel) {
    const res = [];
    if (sel.startsWith('.')) {
      const cls = sel.slice(1);
      if (this.classList.contains(cls)) res.push(this);
    }
    for (const ch of this.children) {
      res.push(...ch.querySelectorAll(sel));
    }
    return res;
  }
  appendChild(ch) { this.children.push(ch); return ch; }
}

const docElements = {
  '#app-content': new MockElement('section', 'app-content'),
  '#sidebar': new MockElement('aside', 'sidebar'),
  '#backdrop': new MockElement('div', 'backdrop'),
  '#page-title': new MockElement('h1', 'page-title'),
  '#page-subtitle': new MockElement('p', 'page-subtitle'),
  '#demo-badge': new MockElement('span', 'demo-badge'),
  '#theme-button': new MockElement('button', 'theme-button'),
  '#connect-button': new MockElement('button', 'connect-button'),
  '#menu-button': new MockElement('button', 'menu-button'),
  '#reconnect-dialog': new MockElement('dialog', 'reconnect-dialog'),
  '#sidebar-status': new MockElement('span', 'sidebar-status'),
  '#sidebar-status-dot': new MockElement('span', 'sidebar-status-dot')
};

const navItems = [
  Object.assign(new MockElement('button', '', 'nav-item active'), { dataset: { page: 'dashboard' } }),
  Object.assign(new MockElement('button', '', 'nav-item'), { dataset: { page: 'users' } }),
  Object.assign(new MockElement('button', '', 'nav-item'), { dataset: { page: 'logs' } }),
  Object.assign(new MockElement('button', '', 'nav-item'), { dataset: { page: 'console' } }),
  Object.assign(new MockElement('button', '', 'nav-item'), { dataset: { page: 'settings' } }),
  Object.assign(new MockElement('button', '', 'nav-item'), { dataset: { page: 'about' } })
];

global.document = {
  documentElement: new MockElement('html'),
  querySelector(sel) {
    if (sel === "#connect-button span:last-child") return new MockElement('span');
    return docElements[sel] || null;
  },
  querySelectorAll(sel) {
    if (sel === '.nav-item') return navItems;
    return [];
  },
  createElement(tag) {
    return new MockElement(tag);
  },
  hasFocus() { return true; }
};

global.window = {
  addEventListener() {},
  location: { reload() {} }
};

global.navigator = {
  serviceWorker: { register: () => Promise.resolve() }
};

global.localStorage = {
  store: {},
  getItem(k) { return this.store[k] || null; },
  setItem(k, v) { this.store[k] = String(v); }
};

console.log('--- Loading app.js in Mock DOM Environment ---');

import('../js/app.js').then(() => {
  console.log('PASS: app.js loaded and executed top-level code successfully.');

  // Verify button event listeners
  console.log('\n--- Verifying Button Listeners ---');
  const connectBtn = docElements['#connect-button'];
  console.log('Connect button listeners:', Object.keys(connectBtn.listeners));
  if (!connectBtn.listeners['click'] || connectBtn.listeners['click'].length === 0) {
    throw new Error('Connect button click listener is missing!');
  }
  console.log('PASS: #connect-button click listener registered.');

  const themeBtn = docElements['#theme-button'];
  if (!themeBtn.listeners['click'] || themeBtn.listeners['click'].length === 0) {
    throw new Error('Theme button click listener is missing!');
  }
  console.log('PASS: #theme-button click listener registered.');

  const demoBadge = docElements['#demo-badge'];
  if (!demoBadge.listeners['click'] || demoBadge.listeners['click'].length === 0) {
    throw new Error('Demo badge click listener is missing!');
  }
  console.log('PASS: #demo-badge click listener registered.');

  const menuBtn = docElements['#menu-button'];
  if (!menuBtn.listeners['click'] || menuBtn.listeners['click'].length === 0) {
    throw new Error('Menu button click listener is missing!');
  }
  console.log('PASS: #menu-button click listener registered.');

  let navClicks = 0;
  navItems.forEach(item => {
    if (item.listeners['click'] && item.listeners['click'].length > 0) navClicks++;
  });
  if (navClicks !== navItems.length) {
    throw new Error(`Expected all ${navItems.length} nav buttons to have listeners, found ${navClicks}`);
  }
  console.log(`PASS: All ${navItems.length} navigation button listeners registered.`);

  // Verify initial render populated content
  const appContent = docElements['#app-content'];
  if (!appContent.innerHTML || appContent.innerHTML.length === 0) {
    throw new Error('Initial render() did not populate #app-content!');
  }
  console.log('PASS: Initial render() populated #app-content successfully.');
  console.log('\n>>> APP INITIALIZATION AND BUTTON HANDLERS VERIFIED 100% <<<');
  process.exit(0);
}).catch(err => {
  console.error('FAIL: app.js threw error during initialization:', err);
  process.exit(1);
});

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
      contains(c) { return this.classes.has(c); },
      toggle(c, force) {
        if (force !== undefined) {
          if (force) this.classes.add(c);
          else this.classes.delete(c);
        } else {
          if (this.classes.has(c)) this.classes.delete(c);
          else this.classes.add(c);
        }
      }
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
      return new MockElement('div', id);
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

import('../js/app.js').then(async () => {
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
  // Test Navigating to Users then Settings
  console.log('\n--- Testing Navigation to Users then Settings ---');
  const usersNav = navItems.find(item => item.dataset.page === 'users');
  usersNav.dispatchEvent({ type: 'click' });
  if (!appContent.innerHTML.includes('Search users')) {
    throw new Error('Navigating to Users did not render user management table!');
  }
  console.log('PASS: Users page renders users table.');

  const settingsNav = navItems.find(item => item.dataset.page === 'settings');
  settingsNav.dispatchEvent({ type: 'click' });
  if (!appContent.innerHTML.includes('settings-grid')) {
    throw new Error('Navigating to Settings did not render settings grid!');
  }
  if (appContent.innerHTML.includes('Search users')) {
    throw new Error('Navigating to Settings still displays users management table!');
  }
  console.log('PASS: Settings page renders settings grid and does not display users table.');

  // Test Navigating back to Users
  usersNav.dispatchEvent({ type: 'click' });
  if (!appContent.innerHTML.includes('Search users') || appContent.innerHTML.includes('settings-grid')) {
    throw new Error('Navigating back to Users failed!');
  }
  console.log('PASS: Re-navigating to Users correctly renders users table.');

  // Test Demo Badge page-specific visibility
  console.log('\n--- Testing Demo Badge Visibility per Page ---');
  const { state } = await import('../js/state.js');
  state.set({ demoMode: true, demoOffsetMs: 54000000 }); // +15h

  const dashboardNav = navItems.find(item => item.dataset.page === 'dashboard');
  const logsNav = navItems.find(item => item.dataset.page === 'logs');
  const consoleNav = navItems.find(item => item.dataset.page === 'console');

  dashboardNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'none') throw new Error('Demo badge should be hidden on dashboard!');
  console.log('PASS: Demo badge hidden on Dashboard.');

  usersNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'none') throw new Error('Demo badge should be hidden on Users!');
  console.log('PASS: Demo badge hidden on Users.');

  logsNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'none') throw new Error('Demo badge should be hidden on Logs!');
  console.log('PASS: Demo badge hidden on Logs.');

  consoleNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'none') throw new Error('Demo badge should be hidden on Console!');
  console.log('PASS: Demo badge hidden on Console.');

  settingsNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'inline-flex') throw new Error('Demo badge should be visible on Settings when demoMode is active!');
  console.log('PASS: Demo badge visible on Settings when demoMode is active.');

  state.set({ demoMode: false });
  settingsNav.dispatchEvent({ type: 'click' });
  if (demoBadge.style.display !== 'none') throw new Error('Demo badge should be hidden when demoMode is false!');
  console.log('PASS: Demo badge hidden on Settings when demoMode is inactive.');

  console.log('\n>>> APP INITIALIZATION AND BUTTON HANDLERS VERIFIED 100% <<<');
  process.exit(0);
}).catch(err => {
  console.error('FAIL: app.js threw error during initialization:', err);
  process.exit(1);
});

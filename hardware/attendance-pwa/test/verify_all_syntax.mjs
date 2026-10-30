import fs from 'fs';
import path from 'vm';
import vm from 'vm';

const files = [
  'hardware/attendance-pwa/js/app.js',
  'hardware/attendance-pwa/js/utils.js',
  'hardware/attendance-pwa/js/state.js',
  'hardware/attendance-pwa/js/protocol.js',
  'hardware/attendance-pwa/js/serial.js',
  'hardware/attendance-pwa/js/ch340.js',
  'hardware/attendance-pwa/js/ui.js',
  'hardware/attendance-pwa/pages/dashboard.js',
  'hardware/attendance-pwa/pages/logs.js',
  'hardware/attendance-pwa/pages/users.js',
  'hardware/attendance-pwa/pages/console.js',
  'hardware/attendance-pwa/sw.js'
];

console.log('--- Checking syntax for all PWA JavaScript files ---');
let allValid = true;

for (const file of files) {
  try {
    const code = fs.readFileSync(file, 'utf8');
    new vm.SourceTextModule(code);
    console.log(`PASS: ${file} syntax valid.`);
  } catch (err) {
    console.error(`FAIL: ${file} syntax error:`, err);
    allValid = false;
  }
}

if (!allValid) {
  process.exit(1);
}
console.log('\nAll PWA JavaScript files passed syntax verification!');

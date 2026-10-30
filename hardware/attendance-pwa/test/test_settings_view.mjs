import fs from 'fs';

// Let's test the settings view function directly
const appCode = fs.readFileSync('hardware/attendance-pwa/js/app.js', 'utf8');

// Check what settings returns
import { state } from '../js/state.js';
import { calculateDemoOffsetMs, formatDemoOffsetSummary, escapeHtml } from '../js/utils.js';

const app = state.get();
console.log('Current state:', app);

// Let's extract the settings function and run it
console.log('Testing settings(app)...');
try {
  const offsetMs = app.demoOffsetMs || 0;
  const isNegative = offsetMs < 0;
  const absMin = Math.round(Math.abs(offsetMs) / 60000);
  const hours = Math.floor(absMin / 60);
  const minutes = absMin % 60;
  const summary = escapeHtml(formatDemoOffsetSummary(offsetMs));
  console.log('summary:', summary);
  console.log('settings HTML generation succeeded.');
} catch (e) {
  console.error('settings failed:', e);
}

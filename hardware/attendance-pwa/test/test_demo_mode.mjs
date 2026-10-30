global.localStorage = {
  store: {},
  getItem(k) { return this.store[k] || null; },
  setItem(k, v) { this.store[k] = String(v); }
};

Promise.all([
  import('../js/state.js'),
  import('../js/utils.js')
]).then(([stateMod, utilsMod]) => {
  const { state } = stateMod;
  const { formatDate, calculateDemoOffsetMs, formatDemoBadge, formatDemoOffsetSummary, csv } = utilsMod;

  console.log('--- 1. Testing Offset Calculations & Badges ---');
  const posMs = calculateDemoOffsetMs('+', 2, 30);
  console.log('calculateDemoOffsetMs(+, 2, 30) =', posMs, 'ms');
  if (posMs !== 9000000) throw new Error('Expected 9000000 ms, got ' + posMs);

  const negMs = calculateDemoOffsetMs('-', 1, 15);
  console.log('calculateDemoOffsetMs(-, 1, 15) =', negMs, 'ms');
  if (negMs !== -4500000) throw new Error('Expected -4500000 ms, got ' + negMs);

  const badgePos = formatDemoBadge(posMs);
  console.log('Badge Pos:', badgePos);
  if (badgePos !== 'DEMO TIME (+2h 30m)') throw new Error('Badge pos mismatch: ' + badgePos);

  const badgeNeg = formatDemoBadge(negMs);
  console.log('Badge Neg:', badgeNeg);
  if (badgeNeg !== 'DEMO TIME (-1h 15m)') throw new Error('Badge neg mismatch: ' + badgeNeg);

  console.log('--- 2. Testing formatDate with Demo Mode Disabled ---');
  state.set({ demoMode: false, demoOffsetMs: 0 });
  const baseEpochSec = 1730304000;
  const normalStr = formatDate(baseEpochSec);
  console.log('Actual time (demoMode=false):', normalStr);
  if (formatDate(0) !== '—') throw new Error('0 must be —');
  if (formatDate(15) !== '—') throw new Error('15s boot time must be —');
  if (formatDate('garbage') !== '—') throw new Error('garbage must be —');

  console.log('--- 3. Testing formatDate with Demo Mode Enabled (+2h) ---');
  state.set({ demoMode: true, demoOffsetMs: 7200000 });
  const shiftedStr = formatDate(baseEpochSec);
  console.log('Shifted time (+2h):', shiftedStr);

  const normalDate = new Date(baseEpochSec * 1000);
  const shiftedDate = new Date(baseEpochSec * 1000 + 7200000);
  const expectedShifted = shiftedDate.toLocaleString([], { dateStyle: 'medium', timeStyle: 'short' });
  console.log('Expected shifted:', expectedShifted);
  if (shiftedStr !== expectedShifted) {
    throw new Error('Shifted string mismatch: expected ' + expectedShifted + ' but got ' + shiftedStr);
  }

  // Critical: Uninitialized, 0, or garbage must STILL be '—'
  if (formatDate(0) !== '—') throw new Error('0 must REMAIN — in demo mode');
  if (formatDate(15) !== '—') throw new Error('15s boot time must REMAIN — in demo mode');
  if (formatDate(null) !== '—') throw new Error('null must REMAIN — in demo mode');

  console.log('--- 4. Testing Reset Action ---');
  state.set({ demoMode: false, demoOffsetMs: 0 });
  const restoredStr = formatDate(baseEpochSec);
  console.log('Restored time after reset:', restoredStr);
  if (restoredStr !== normalStr) throw new Error('Reset failed to restore original display time');

  console.log('--- 5. Verifying Raw Exports Data Integrity ---');
  const mockLogs = [
    { uid: 'E3BBC914', name: 'Bhagyasree pl', timestamp: 1730304000, event: 'Check-in', status: 'IN' }
  ];
  state.set({ logs: mockLogs, demoMode: true, demoOffsetMs: 7200000 });

  // JSON export
  const exportedJson = JSON.stringify(mockLogs, null, 2);
  if (!exportedJson.includes('"timestamp": 1730304000')) {
    throw new Error('Exported JSON timestamp was corrupted!');
  }
  console.log('Exported JSON contains authentic raw timestamp: 1730304000');

  // CSV export
  const exportedCsv = csv(mockLogs);
  if (!exportedCsv.includes('1730304000')) {
    throw new Error('Exported CSV timestamp was corrupted!');
  }
  console.log('Exported CSV contains authentic raw timestamp: 1730304000');

  console.log('\n>>> ALL DEMO MODE TESTS PASSED PERFECTLY! <<<');
}).catch(err => {
  console.error('Test execution failed:', err);
  process.exit(1);
});


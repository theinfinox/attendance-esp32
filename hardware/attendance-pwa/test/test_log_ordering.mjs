// ============================================================================
// test_log_ordering.mjs — Comprehensive Validation of PWA Log Ordering & Deduplication
// ============================================================================

global.localStorage = {
  store: {},
  getItem(k) { return this.store[k] || null; },
  setItem(k, v) { this.store[k] = String(v); }
};

Promise.all([
  import('../js/utils.js')
]).then(([utilsMod]) => {
  const {
    getLogTimestampMs,
    getLogEventId,
    compareAttendanceLogs,
    isDuplicateLog,
    deduplicateLogs,
    sortAttendanceLogs
  } = utilsMod;

  let passed = 0;
  function assert(condition, message) {
    if (!condition) {
      console.error(`FAIL: ${message}`);
      process.exit(1);
    }
    console.log(`PASS: ${message}`);
    passed++;
  }

  console.log('\n--- 1. Testing Event ID & Timestamp Extraction ---');
  assert(getLogEventId({ event_id: 5 }) === 5, 'Extract numeric event_id');
  assert(getLogEventId({ id: '12' }) === 12, 'Extract string event_id');
  assert(getLogEventId({ event_id: 0 }) === null, 'Ignore zero event_id');
  assert(getLogEventId({ event_id: -3 }) === null, 'Ignore negative event_id');
  assert(getLogEventId({ name: 'No ID' }) === null, 'Handle missing event_id safely');

  assert(getLogTimestampMs({ timestamp: 1730300940 }) === 1730300940000, 'Convert unix seconds to ms');
  assert(getLogTimestampMs({ timestamp: 1730300940000 }) === 1730300940000, 'Keep existing ms unchanged');
  assert(getLogTimestampMs({ time: 0 }) === 0, 'Handle 0 timestamp safely');
  assert(getLogTimestampMs({ date: 'invalid' }) === 0, 'Handle invalid date string safely');

  console.log('\n--- 2. Testing Monotonic event_id Precedence (Requirement 2 & 3) ---');
  const e1 = { event_id: 1, timestamp: 1730300000, name: 'Student A' };
  const e2 = { event_id: 2, timestamp: 1730200000, name: 'Student B' }; // Earlier timestamp, but higher event_id!
  const sortedE = sortAttendanceLogs([e1, e2]);
  assert(sortedE[0].event_id === 2 && sortedE[1].event_id === 1, 'Higher event_id precedes even if timestamp is earlier');

  console.log('\n--- 3. Testing Timestamp Fallback when event_id Unavailable (Requirement 3) ---');
  const tOld = { timestamp: 1730300000, name: 'Older' };
  const tNew = { timestamp: 1730301000, name: 'Newer' };
  const sortedT = sortAttendanceLogs([tOld, tNew]);
  assert(sortedT[0].name === 'Newer' && sortedT[1].name === 'Older', 'Newer timestamp precedes when event_id is absent');

  console.log('\n--- 4. Mixed Comparison (Historical with event_id vs Live without event_id) ---');
  const hist = { event_id: 4, timestamp: 1730300940, name: 'Historical' };
  const live = { timestamp: 1730302000, name: 'Live' }; // Happened later, no event_id yet
  const sortedMixed = sortAttendanceLogs([hist, live]);
  assert(sortedMixed[0].name === 'Live' && sortedMixed[1].name === 'Historical', 'Live event with later timestamp appears at top');

  console.log('\n--- 5. Handling Missing / Uninitialized Timestamps Safely ---');
  const valid = { event_id: 1, timestamp: 1730300940, name: 'Valid' };
  const uninit1 = { timestamp: 0, name: 'Uninit 1' };
  const uninit2 = { name: 'Uninit 2' };
  const sortedUninit = sortAttendanceLogs([uninit1, valid, uninit2]);
  assert(sortedUninit[0].name === 'Valid', 'Valid events take precedence over uninitialized');
  assert(sortedUninit.length === 3, 'All uninitialized events preserved at bottom');

  console.log('\n--- 6. Reconstructing the User Scenario: Student State & Ordering ---');
  // At 8:39 PM:
  // Event 1: Avani Check-in
  // Event 2: Avani Check-out
  // Event 3: Bhagyasree Check-in
  // Event 4: Bhagyasree Check-out
  // At 8:57 PM:
  // Event 5: Bhagyasree Check-in
  // Event 6: Avani Check-in
  const rawDumpOldestFirst = [
    { event_id: 1, timestamp: 1730300940, uid: '2333A791', name: 'Avani krishna', event: 'Check-in', status: 'IN', session: 1 },
    { event_id: 2, timestamp: 1730300950, uid: '2333A791', name: 'Avani krishna', event: 'Check-out', status: 'OUT', session: 1 },
    { event_id: 3, timestamp: 1730300960, uid: 'E3BBC914', name: 'Bhagyasree pl', event: 'Check-in', status: 'IN', session: 1 },
    { event_id: 4, timestamp: 1730300970, uid: 'E3BBC914', name: 'Bhagyasree pl', event: 'Check-out', status: 'OUT', session: 1 },
    { event_id: 5, timestamp: 1730302030, uid: 'E3BBC914', name: 'Bhagyasree pl', event: 'Check-in', status: 'IN', session: 2 },
    { event_id: 6, timestamp: 1730302040, uid: '2333A791', name: 'Avani krishna', event: 'Check-in', status: 'IN', session: 2 }
  ];

  const sortedScenario = sortAttendanceLogs(rawDumpOldestFirst);
  assert(sortedScenario[0].event_id === 6, 'Event #6 (Avani Check-in 8:57 PM) is at the very top');
  assert(sortedScenario[1].event_id === 5, 'Event #5 (Bhagyasree Check-in 8:57 PM) is second');
  assert(sortedScenario[2].event_id === 4, 'Event #4 (Bhagyasree Check-out 8:39 PM) is third');
  assert(sortedScenario[3].event_id === 3, 'Event #3 (Bhagyasree Check-in 8:39 PM) is fourth');
  assert(sortedScenario[4].event_id === 2, 'Event #2 (Avani Check-out 8:39 PM) is fifth');
  assert(sortedScenario[5].event_id === 1, 'Event #1 (Avani Check-in 8:39 PM) is at the bottom');

  // Verify chronological consistency for each student
  const avaniLogs = sortedScenario.filter(l => l.uid === '2333A791');
  assert(avaniLogs[0].event === 'Check-in' && avaniLogs[0].session === 2, 'Avani latest is Session 2 Check-in');
  assert(avaniLogs[1].event === 'Check-out' && avaniLogs[1].session === 1, 'Avani second is Session 1 Check-out');
  assert(avaniLogs[2].event === 'Check-in' && avaniLogs[2].session === 1, 'Avani third is Session 1 Check-in');

  const bhagyaLogs = sortedScenario.filter(l => l.uid === 'E3BBC914');
  assert(bhagyaLogs[0].event === 'Check-in' && bhagyaLogs[0].session === 2, 'Bhagyasree latest is Session 2 Check-in');
  assert(bhagyaLogs[1].event === 'Check-out' && bhagyaLogs[1].session === 1, 'Bhagyasree second is Session 1 Check-out');
  assert(bhagyaLogs[2].event === 'Check-in' && bhagyaLogs[2].session === 1, 'Bhagyasree third is Session 1 Check-in');

  console.log('\n--- 7. Reconnection & Deduplication (Requirement 5 & 6) ---');
  // State had live punches before reconnect (live punches didn't have event_id yet)
  const currentBeforeReconnect = [
    { timestamp: 1730302040, uid: '2333A791', name: 'Avani krishna', event: 'Check-in', status: 'IN' },
    { timestamp: 1730302030, uid: 'E3BBC914', name: 'Bhagyasree pl', event: 'Check-in', status: 'IN' }
  ];

  // Reconnection loads DUMP_LOGS from Flash (which has event_id 1..6)
  const mergedOnReconnect = deduplicateLogs([...rawDumpOldestFirst, ...currentBeforeReconnect]);
  assert(mergedOnReconnect.length === 6, `Expected exactly 6 deduplicated logs, got ${mergedOnReconnect.length}`);

  const sortedAfterReconnect = sortAttendanceLogs(mergedOnReconnect);
  assert(sortedAfterReconnect.length === 6, 'Length remains 6 after sorting');
  assert(sortedAfterReconnect[0].event_id === 6, 'Event #6 is top');
  assert(sortedAfterReconnect[0].uid === '2333A791', 'Event #6 belongs to Avani');

  console.log('\n--- 8. Preserving Original Fields (Requirement 6) ---');
  const sample = sortedAfterReconnect[0];
  assert(sample.event_id === 6, 'event_id preserved');
  assert(sample.timestamp === 1730302040, 'timestamp preserved');
  assert(sample.event === 'Check-in', 'event preserved');
  assert(sample.status === 'IN', 'status preserved');
  assert(sample.name === 'Avani krishna', 'name preserved');
  assert(sample.session === 2, 'session preserved');

  console.log(`\n========================================`);
  console.log(`ALL ${passed} VALIDATION TESTS PASSED!`);
  console.log(`========================================\n`);
}).catch(err => {
  console.error('Test execution failed with error:', err);
  process.exit(1);
});


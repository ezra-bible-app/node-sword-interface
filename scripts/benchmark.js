// Benchmark script for performance profiling
// Run with: node --prof scripts/benchmark.js
// Then process: node --prof-process isolate-*.log > profile.txt
const NodeSwordInterface = require('../index.js');

async function runBenchmark() {
    const nsi = new NodeSwordInterface();
    
    // Check if KJV is installed
    const localModules = nsi.getAllLocalModules();
    const kjvInstalled = localModules.some(m => m.name === 'KJV');
    
    if (!kjvInstalled) {
        console.log('KJV module is not installed. Please install it first.');
        console.log('Available modules:', localModules.map(m => m.name).join(', '));
        return;
    }
    
    nsi.enableMarkup();
    
    const iterations = 3;
    const times = [];
    let verseCount = 0;
    
    console.log(`Running ${iterations} iterations of getBibleText("KJV") with markup enabled...`);
    console.log('Use --prof flag to generate V8 profiler output\n');
    
    for (let i = 0; i < iterations; i++) {
        const start = process.hrtime.bigint();
        const verses = nsi.getBibleText('KJV');
        const end = process.hrtime.bigint();
        
        const durationMs = Number(end - start) / 1_000_000;
        times.push(durationMs);
        verseCount = verses.length;
        
        console.log(`  Iteration ${i + 1}: ${durationMs.toFixed(2)} ms`);
    }
    
    const avg = times.reduce((a, b) => a + b, 0) / times.length;
    const min = Math.min(...times);
    const max = Math.max(...times);
    
    console.log('\nBenchmark complete!');
    console.log(`  Verses processed: ${verseCount}`);
    console.log(`  Average time: ${avg.toFixed(2)} ms`);
    console.log(`  Min: ${min.toFixed(2)} ms, Max: ${max.toFixed(2)} ms`);
}

runBenchmark().catch(console.error);

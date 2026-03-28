const createLayoutEngine = require('./web/dist/layout_engine.js');

createLayoutEngine().then(Module => {
    console.log("WASM Module Loaded.");
    
    // Setup functions
    const initEngine = Module.cwrap('initEngine', 'void', []);
    const renderFrame = Module.cwrap('renderFrame', 'number', ['number']);
    
    console.log("Calling initEngine()...");
    initEngine();
    
    // Test the JSON layout execution locally
    console.log("Injecting JSON layout payload to activate widgets...");
    const applyLayout = Module.cwrap('applyLayout', 'string', ['string']);
    const applyMockData = Module.cwrap('applyMockData', 'void', ['string']);
    const fs = require('fs');
    const layoutStr = fs.readFileSync('modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.json', 'utf8');
    const dataStr = fs.readFileSync('tools/layoutsim/mock_data/nationalRailBoard.json', 'utf8');
    applyMockData(dataStr);
    const result = applyLayout(layoutStr);
    console.log("Layout Parser Result:", result);
    
    console.log("Calling renderFrame(0) Post-Injection...");
    let ptr;
    try {
        ptr = renderFrame(0);
    } catch(e) {
        console.error("FATAL WASM EXCEPTION CAUGHT!");
        console.error(e.stack);
        process.exit(1);
    }
    console.log("Memory Pointer returned:", ptr);
    
    if (ptr === 0) {
        console.error("Pointer is NULL. Exiting.");
        process.exit(1);
    }
    
    const view = new Uint8ClampedArray(Module.HEAPU8.buffer, ptr, 256 * 64 * 4);
    
    let sum = 0;
    let nonZeroCount = 0;
    for(let i=0; i<view.length; i+=4) {
        // Red channel is basically everything we need to check
        if (view[i] > 0) {
            sum += view[i];
            nonZeroCount++;
        }
    }
    
    console.log("Total non-black pixels rendered:", nonZeroCount);
    console.log("Sum of R channels:", sum);
    
}).catch(err => {
    console.error("Fatal Error initializing WASM engine:", err);
});

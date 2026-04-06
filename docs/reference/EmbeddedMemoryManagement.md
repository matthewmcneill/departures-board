# Embedded Memory Management Guide

This document discusses the approach to memory management specifically for the `departures-board` project, which operates on an ESP32 embedded platform running FreeRTOS. It outlines the constraints, different memory types available, strategies for allocation, and concrete best practices.

## 1. Memory Management in Embedded Systems

Memory management in embedded systems differs fundamentally from desktop and server environments. 

- **Severe Resource Constraints:** Microcontrollers like the ESP32 possess kilobytes or megabytes of RAM, rather than the gigabytes seen on desktops. Every byte must be accounted for.
- **Deterministic Behavior & Reliability:** The software acts as an appliance expected to run for months or years without interruption (24/7 uptime). Unpredictable latency when allocating memory or unpredictable crashes from exhaustion are unacceptable.
- **Fragmentation Vulnerabilities:** Without virtual memory, the heap can become heavily fragmented. Over time, repeatedly allocating and freeing chunks of memory can leave the heap splintered, causing subsequent allocations to fail despite there being enough total free memory. This leads to silent failures and system resets.

## 2. ESP32 Memory Architecture

The ESP32 platform offers multiple discrete memory regions, each with unique characteristics and restrictions.

### SRAM (Internal Data Memory)
- **Role:** High-speed RAM accessed directly by the CPU. This is where your global variables, stack frames, and the internal heap reside.
- **Limitations:** Typically under 520 KB in total, mapped out among FreeRTOS, WiFi stacks, and user applications.
  ```cpp
  // Global variables reside in SRAM Data segments
  int activeConnections = 0; 
  
  void setup() {
      // Standard dynamically allocated objects reside in the SRAM Heap
      std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();
  }
  ```

### IRAM (Instruction RAM) & ISR Safety
- **Role:** Memory that executes code directly at high speed. It guarantees 0 latency access relative to caching on flash.
- **Use Case:** Critical for Interrupt Service Routines (ISRs) and highly time-sensitive core systems.
- **The `volatile` Keyword:** Any variable modified inside a hardware interrupt MUST be marked `volatile` to prevent compiler cache-optimizations.
  ```cpp
  // Flag shared between the ISR and the main loop MUST be volatile
  volatile bool dataReady = false; 
  
  // ISR function is placed explicitly in high-speed IRAM
  void IRAM_ATTR onHardwareInterrupt() { 
      dataReady = true; 
  }
  ```

### NVS (Non-Volatile Storage)
- **Role:** Flash-backed key-value store. Evaluated often through the `Preferences` library in Arduino layers, but fully accessible via native ESP-IDF C headers.
- **Use Case:** Small, scalar persistence like saving WiFi credentials, API tokens, or short user configurations.
- **Limitations:** Limited write cycles (flash wear). **Never** write high-frequency telemetry or frequently changing variables to NVS.
  
  **Using Arduino `Preferences` Wrapper:**
  ```cpp
  #include <Preferences.h>
  Preferences pref;
  
  // false = read/write mode
  pref.begin("my-app", false);
  pref.putString("apiKey", "secure_token_123");
  pref.end();
  ```

  **Using Native ESP-IDF C API:**
  ```cpp
  #include "nvs_flash.h"
  #include "nvs.h"

  void saveNativeNVS() {
      // 1. Ensure NVS is initialized (usually occurs once during boot sequence)
      nvs_flash_init();
      
      nvs_handle_t handle;
      // 2. Open namespace in read/write mode
      if (nvs_open("my-app", NVS_READWRITE, &handle) == ESP_OK) {
          // 3. Write data to cache map
          nvs_set_str(handle, "apiKey", "secure_token_123");
          
          // 4. Force physical commit to flash memory wear-leveling layer
          nvs_commit(handle); 
          
          // 5. Release handle
          nvs_close(handle);
      }
  }
  ```

### Flash Memory
- **Role:** Permanent storage for compiled bytecode (`PROGMEM`) and persistent file structures (LittleFS/SPIFFS).
- **Use Case:** Storing large assets, fonts, layouts, and serialized JSON UI configuration files.
  ```cpp
  // Stored permanently in flash code space (PROGMEM), saving precious SRAM
  const char staticText[] PROGMEM = "This string is read directly from Flash memory caches";
  
  // Or file system access via LittleFS
  #include <LittleFS.h>
  File configFile = LittleFS.open("/config.json", "r");
  ```

### PSRAM (External/Pseudo RAM)
- **Role:** Massive bulk memory accessible over SPI (typically 4MB+ depending on the ESP32 module).
- **Use Case:** Huge dynamic application buffers, full-screen frame buffers, or huge JSON deserialization trees. It is slower than internal SRAM.
  ```cpp
  // Manually requesting a buffer explicitly on the external SPI RAM chip
  uint8_t* hugeBuffer = (uint8_t*)heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM);
  if (hugeBuffer) {
      heap_caps_free(hugeBuffer);
  }
  
  // Or via compiler macro (if PSRAM mapping is enabled in menuconfig)
  EXT_RAM_ATTR uint8_t psramBuffer[1048576]; 
  ```

### RTC Memory
- **Role:** Extra-low power memory kept alive during deep sleep cycles.
- **Use Case:** Survives CPU halts and wake-ups.
  ```cpp
  // This variable survives deep sleep reboots. Normal SRAM variables are completely lost.
  RTC_DATA_ATTR int wakeJumpCount = 0;
  
  void setup() {
      wakeJumpCount++; // Tracks how many times we woke from deep sleep
      esp_deep_sleep_start();
  }
  ```

### Stack vs. Heap in FreeRTOS
Unlike desktop apps, FreeRTOS assigns every single task its own isolated `Stack`. The `Heap` is a shared global resource across all tasks. Exceeding a stack boundary instantly triggers an unrecoverable system abort.
  ```cpp
  void myTask(void *pvParameters) {
      // Allocated safely inside this specific Task's isolated Stack limits
      int stackVal = 10; 
      
      // Pulled from the shared global FreeRTOS Heap (Use RAII instead of raw pointers!)
      int* heapVal = new int(20); 
      delete heapVal;
  }
  ```

---

## 3. Approaches to Memory Allocation & Storage Writes

Understanding how to claim RAM, as well as how to persist data to specific storage domains, is critical. Below are the functional code examples for each methodology.

### RAM Allocation Paradigms

1. **Static / Global Allocation**
   - Memory bounds and addresses are determined at compile time (placed in BSS/Data segments).
   - **Pros:** 100% safe. No runtime overhead, impossible to leak, deterministic.
   - **Cons:** Memory is permanently held for the lifetime of the application.
   ```cpp
   // Globally allocated space. Exists for the entire lifetime of the firmware execution.
   static uint8_t globalNetworkBuffer[2048];
   
   void runNetworkTask() {
       // Only initialized once. Retains its memory bounds continuously.
       static bool isNetworkConfigured = false; 
   }
   ```

2. **Stack Allocation**
   - Automatic allocation of local variables within a function scope.
   - **Pros:** Extremely fast and cleans itself up automatically when the function exits.
   - **Cons:** Limited by the FreeRTOS task stack size. Allocating massive buffers here triggers immediate crashes.
   ```cpp
   void processLocalData() {
       // Pushed explicitly onto the FreeRTOS task stack
       uint8_t localBuffer[256]; 
       
       // ... processing logic ...
       
       // Memory is instantly released automatically when processLocalData() returns. 
       // No manual free is required.
   }
   ```

3. **`malloc()` / `free()` / `new` / `delete`**
   - Bypasses managed safety, placing the burden of cleanup and life-cycle on the developer. High risk of memory leaks and fragmentation. Avoid completely.
   ```cpp
   void discouragedAllocation() {
       // C-Style (Discouraged): Allocating raw blocks on the heap
       uint8_t* rawBuffer = (uint8_t*)malloc(1024);
       if (rawBuffer != nullptr) {
           // If we return early before this free(), we have permanently leaked memory
           free(rawBuffer); 
       }
   
       // C++-Style (Discouraged): Building objects on the heap using pointers
       MyController* controller = new MyController();
       controller->doWork();
       // If doWork() throws an exception or fails out, 'delete' is skipped. Fatal leak.
       delete controller; 
   }
   ```

### Writing to Specialized Storage Spaces

When pushing data out of RAM, use the following API constraints:

- **Writing to NVS**:
  Used exclusively for small, infrequent configurations.
  ```cpp
  #include "nvs_flash.h"
  #include "nvs.h"

  void persistCredentialsNative() {
      nvs_handle_t handle;
      if (nvs_open("my-app", NVS_READWRITE, &handle) == ESP_OK) {
          nvs_set_str(handle, "apiKey", "12345");
          nvs_commit(handle); // CRITICAL: Flushes to physical flash
          nvs_close(handle);
      }
  }
  ```

- **Writing to Flash Memory (LittleFS)**:
  Used for larger config files, images, or layouts. Stream the data directly to the file object rather than building a massive string payload in the heap first.
  ```cpp
  #include <LittleFS.h>
  
  void persistConfig(const JsonDocument& doc) {
      // Allocate the filesystem object and explicitly open in write mode
      File f = LittleFS.open("/config.json", "w");
      if (f) {
          // Serialize directly out to flash memory bypassing a heap String buffer
          serializeJson(doc, f); 
          
          // Explicitly close the file handler and flush to disk
          f.close();
      }
  }
  ```

- **Explicitly Allocating PSRAM**:
  If the ESP32 is not configured to automatically spill the heap into PSRAM via `menuconfig`, you must manually direct massive buffers to it using ESP-IDF memory capabilities:
  ```cpp
  void processHugePayload() {
      // Allocate 1 MB directly in external SPI RAM
      uint8_t* hugeBuffer = (uint8_t*)heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM);
      
      if (hugeBuffer) {
          // Execute processing...
          
          // You MUST manually release the external memory capability
          heap_caps_free(hugeBuffer);
      }
  }
  ```

---

## 4. Modern C++ Smart Pointer Management (RAII)

Resource Acquisition Is Initialization (RAII) is the project's mandated standard for heap usage. Instead of relying on human discipline to match every `new` with a `delete`, RAII wraps heap memory inside a lightweight stack-based object. When the stack object goes out of scope, the C++ compiler automatically calls the destructor, which safely releases the heap memory for you.

### `std::unique_ptr<T>` (Exclusive Ownership)
The `unique_ptr` guarantees that **exactly one** owner is responsible for the memory at any given time. It carries *zero runtime performance overhead* compared to a raw pointer. 

**Why use `std::make_unique<T>()` over `new`?**
Using `std::make_unique` is highly preferred because it is exception-safe. If an error occurs halfway through allocating sub-components, `make_unique` ensures the memory is correctly cleaned up before throwing, whereas raw `new` wrapped later in a unique pointer can theoretically leak during evaluation.

```cpp
#include <memory>

void safeProcess() {
    // 1. Memory is securely mapped to the heap. Ownership defaults to this local function block.
    std::unique_ptr<MyController> ctrl = std::make_unique<MyController>();
    
    // 2. You interact with it exactly like a normal pointer
    ctrl->executeWorkflow();
    
    // 3. You can safely return early!
    if (ctrl->hasErrors()) return; 
    
    // 4. As soon as the function hits the final brace (or returns early), 'ctrl' leaves scope.
    // The C++ compiler has secretly injected `delete ctrl` for you. Zero leaks possible.
}
```

### Passing and Moving Ownership (`std::move`)
Because a `unique_ptr` enforces exclusive ownership, **you cannot copy it**. If you try to pass it to another function using standard pass-by-value, the code will fail to compile. To hand the memory lock over to someone else, you must *intentionally* move it.

```cpp
void takeFullOwnership(std::unique_ptr<MyController> incomingPtr) {
    // This function is now the exclusive owner. When it exits, the memory is destroyed.
    incomingPtr->executeWorkflow();
}

void setupTask() {
    auto masterCtrl = std::make_unique<MyController>();
    
    // ERROR: Compiles fail here! You cannot duplicate the ownership lock.
    // takeFullOwnership(masterCtrl); 
    
    // SUCCESS: Intentionally transfer the ownership lock over.
    takeFullOwnership(std::move(masterCtrl)); 
    
    // DANGER: 'masterCtrl' is now a nullptr! Do not use it after a move.
}
```

### Other Key Parts of the Scheme

If `unique_ptr` exclusively locks memory, how do other systems interact with it?

1. **Borrowing via Raw Observational Pointers (`ptr.get()`)**
   If an architecture component merely needs to *look* at or *use* data without claiming ownership, pass it as a raw pointer or reference. A raw pointer is not intrinsically evil—it is only evil when it represents *ownership* requiring a `delete`.
   ```cpp
   // The display function borrows a look natively, but the appContext still owns the memory
   void renderScreen(MyController* borrowedRef) {
       borrowedRef->draw();
       // We DO NOT call delete here!
   }
   
   void main() {
       auto mainCtrl = std::make_unique<MyController>();
       renderScreen(mainCtrl.get()); // Extract the raw pointer safely to act as an observer
   }
   ```

2. **`std::shared_ptr<T>` (Shared Ownership)**
   A `shared_ptr` uses reference counting. It keeps an internal tally of how many systems currently hold the pointer, and only calls `delete` when the count hits `0`. 
   * **Warning:** In ESP32 systems, `shared_ptr` carries a massive performance penalty because of the thread-safe "control block" required to track the reference count. Do not use this unless the architectural design explicitly requires unpredictable multiple-ownership lifetimes.

3. **`std::weak_ptr<T>` (The Observer)**
   Used in conjunction with `shared_ptr`. It allows a system to observe data *without* incrementing the reference count. It is commonly used to break circular dependencies (where two objects hold a `shared_ptr` to each other, meaning their reference counts never hit `0`, causing a permanent unrecoverable leak).

---

## 5. General Recommendations & Best Practices

To ensure maximum uptime and stability, abide by the following rules:

1. **Default to Stack and Static:** If the size is known at compile time, avoid the heap entirely. 
2. **Never Use Naked `new` / `delete`:** Use `std::make_unique<T>()` and `std::unique_ptr<T>`. Let the compiler write your `delete` logic.
3. **Strict Bounds Checking:** Never allocate memory dynamically without bounds checking. Always validate input sizes before assigning arrays or `malloc` lengths.
4. **Memory Synchronization (FreeRTOS):** You are strictly prohibited from accessing shared memory resources across different task threads without synchronization primitives. Use FreeRTOS Mutexes (`xSemaphoreCreateMutex`), Queues, or Semaphores to lock access to global data.
5. **Pre-allocate Large Buffers:** Allocate the necessary buffers once during the boot phase. Thrashing the heap by allocating and destroying ruins memory layouts.
6. **Beware of Strings:** Frequent naive concatenations of `String` or `std::string` fragments the heap violently. Use stream buffers (`snprintf`) instead.

---

## 6. Application to the `departures-board` Codebase

The `departures-board` uses the following specific implementations aligned with these principles:

- **The RAII Migration:** Hand-rolled pointers are being removed. Interface injection (like `iDataSource`, `boardController`) now exclusively receive dependencies wrapped in `std::unique_ptr`.
- **Memory-Heavy JSON Deserialization:** The parsing of massive National Rail or RDM data sets via `ArduinoJson` is the most intensive memory bottleneck. Sizing the `JsonDocument` is highly deliberate.
- **Data Source Object Ownership:** `appContext` dictates the lifetime and owns the memory of controllers. Display boards operate by borrowing references bounds-checked by the owner via observational references `.get()`.

---

## 7. FreeRTOS Memory Profiling and Debugging

When testing or hunting a crash, leverage native FreeRTOS diagnosis functions:

- **Free Global Heap:** Use `esp_get_free_heap_size()` to log the available memory. Sudden, repeated drops across iterations flag a high risk of a memory leak.
- **High-Water Marks:** Identify if you are close to blowing out a task's stack size by logging `uxTaskGetStackHighWaterMark(NULL)`. If this number approaches `0`, you need to increase the FreeRTOS task size or move local buffers to the heap.

---

## 8. Resource Impact Assessments for New Features

Because we operate in a highly constrained environment, any future contribution or `.agents/workflows/plan-start.md` architecture implementation MUST formally assess its expected hardware strain. 

When drafting new implementation plans, developers must include a **"Resource Impact Assessment"** section that validates:
- **Memory Impact:** What is the estimated load on Flash, RAM (Heap), and FreeRTOS task Stacks?
- **Power Impact:** Does this delay deep sleep modes or run heavy wireless duty cycles?
- **Security Impact:** Does this logic write sensitive tokens, and are they protected correctly?

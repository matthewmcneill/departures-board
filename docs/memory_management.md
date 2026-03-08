# Memory Management on ESP32

## Rationale: `String` vs `char[]` Buffers

This document explains the memory management strategy used in the `departures-board` project, particularly focusing on why we use fixed-size C-style strings (`char[]`) instead of the Arduino `String` class in our core API clients.

### The Hidden Danger of Heap Fragmentation

When you use the `String` class, especially continuous concatenation (e.g., `myString += newChar;` or `String result = a + b;`) inside a fast-running parsing loop, the ESP32 performs dynamic memory allocation under the hood. 

1. It requests a block of memory from the heap.
2. It copies the old data into it.
3. It tacks on the new data.
4. It frees the old, smaller block of memory.

Over a few hours or days, this constant allocating and freeing punches microscopic holes all over the heap. 

If you check the Serial console and see `Free Heap: 150,000 bytes`, everything might look healthy. But the danger isn't the *total* free memory; it's the **Max Free Block** size. When the ESP32 attempts to establish a secure HTTPS connection (TLS), it often needs a single, contiguous, unbroken block of 16KB to 32KB of RAM. If your 150KB of free heap is actually fragmented into thousands of tiny 500-byte pieces, the TLS connection will suddenly fail with an "Out of Memory" exception, causing the board to either freeze or reboot loop.

Removing `String` objects from the hottest paths of the code (like the XML/JSON parsers that run thousands of times per API fetch) ensures the heap remains pristine.

### Benefits of our Approach (`char[]` buffers)

We adopted fixed-size `char[]` buffers for the API clients:

- **Zero Heap Fragmentation:** Fixed `char[]` buffers are allocated on the stack (when temporary) or in the global data space (if static/global). They never touch the dynamic heap, entirely eliminating fragmentation from the parsing loops.
- **Predictability:** The memory footprint of the API clients becomes completely deterministic. The application will use exactly the same amount of memory on its 1000th API fetch as it did on its first.
- **Speed:** Native C operations like `snprintf` or `strncpy` execute significantly faster than `String` concatenations because they bypass the memory manager.

### Tradeoffs and Downsides

While fixed buffers solve fragmentation, they come with their own challenges:

- **Risk of Buffer Overflows/Truncation:** The `String` class automatically resizes if a string gets longer. If an API returns a 50-character station name and our `char platformName[30]` is only 30 bytes, we must truncate it. If we use unsafe functions like `strcpy`, it will overwrite adjacent memory and instantly crash the ESP32.
- **Wasted Memory Base-load:** If we create a `char destination[128]` to safely hold the absolute longest train destination we expect, but 99% of trains are just going to "London", we are permanently tying up 128 bytes of RAM when we only needed 7 bytes.
- **Developer Overhead:** Native C-strings lack the convenient concatenation operators (`+`) of `String` class items. Constructing complex URL query strings or JSON payloads requires careful use of `snprintf` and manual size calculations.

### Implementation Guidelines

To prevent buffer overflow crashes, we strictly adhere to the following rules when working with character arrays:

1. **Never use `strcpy` or `strcat` blindly.** Always use boundary-aware alternatives like `strncpy` and `strncat`.
2. **Use `snprintf` for formatting.** When constructing strings, `snprintf(buffer, sizeof(buffer), ...)` ensures that no more bytes are written than the buffer can hold. It silently truncates overflowing data instead of corrupting memory.
3. **Always ensure null termination.** Functions like `strncpy` might leave strings without a null-terminator if the source is equal to or larger than the destination. Always explicitly set `buffer[sizeof(buffer) - 1] = '\0';` if using them directly (or rely on `snprintf` which guarantees it).
4. **Choose reasonable buffer sizes.** We deliberately over-allocate for common values (e.g., 64 bytes for a station name, 256 bytes for an error message) while remaining well within the available free RAM. The wasted memory of a large static buffer is an acceptable trade-off for zero heap fragmentation and guaranteed long-term uptime.

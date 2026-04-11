# Upstream Configuration Migration Strategy

This document outlines a long-term strategy for cleanly handling configuration schemas from the monolithic upstream `gadec-uk/departures-board` repository without polluting our own v3.0 `ConfigManager` logic with mounting legacy debt.

## Background constraints
Our fork relies on explicitly versioned configurations (e.g., `"version": 2.6`) to manage migrations sequentially. The upstream repository doesn't write a version number to `config.json`. Instead, the upstream parsing logic infers state linearly based on the presence or absence of keys (e.g., treating `"tube": true` as legacy vs `"mode": 1`). 

Currently, our `ConfigManager::loadConfig()` merges the parsing of our modern config, our legacy config, *and* upstream's config into a single massive control flow by assuming missing versions are `1.0f`. As upstream evolves, this will become unmaintainable.

## Proposed Architectural Approach: The Epoch Translation Matrix

To maintain compatibility going forward, we should decouple **Upstream Translation** from **Internal Migration**. 

### 1. The Epoch Sniffer (Detection)
We introduce a pre-flight heuristic analyzer, `detectConfigEpoch(JsonDocument& doc)`. Since upstream lacks a version key, we fingerprint the JSON structure to determine exactly which "epoch" (point in time) the upstream configuration comes from.

```cpp
float detectConfigEpoch(JsonObject doc) {
    // 1. Is it our fork?
    if (doc.containsKey("version")) {
        return doc["version"].as<float>();
    }

    // 2. Unversioned. Determine the Upstream Epoch via fingerprinting
    // Heuristic A: Gadec's original v1.x (Boolean tube mode)
    if (doc.containsKey("tube") && !doc.containsKey("mode")) {
        return EPOCH_GADEC_V1; // e.g., 1.0f
    }
    
    // Heuristic B: Gadec's modern flat schema (Integer modes, flat CRS)
    if (doc.containsKey("mode") && doc.containsKey("crs")) {
        return EPOCH_GADEC_V2; // e.g., 1.1f
    }

    // Fallback
    return EPOCH_UNKNOWN; 
}
```

### 2. The Translation Layer (Upstream -> Latest Format)
Once an upstream epoch is identified, we pass it through an isolated translation function rather than mixing the parsing logic. This function reads the upstream `JsonObject` and explicitly maps it into our "Latest" state-of-the-art internal format (e.g., `v2.6f+`), directly populating the current structures like the `boards[]` array and nested `feeds` objects. By targeting the latest schema, we ensure that new functionality added by upstream in parallel does not get dropped by trying to force it into our older `v2.0` schema.

*By doing this, we can remove dozens of legacy fallback keys (like `altCrs`, `tubeLat`, `busFilter`) from the core `loadConfig()` parser.* The main parser only needs to know about our modern schema.

### 3. Internal Migrations Bypass
By translating directly from the Upstream Epoch into our *latest* internal version (`v2.6f+`), we cleanly bypass our internal sequential pipelines (`v2.0 -> v2.2 -> v2.5` etc). The `configVersion` will be implicitly set to the latest version immediately. This means our internal migration steps remain strictly dedicated for upgrading *our* older forks, and are never risked or polluted by upstream payloads.

## Developer Guide: How to Handle Future Upstream Updates

When the original author inevitably adds new features and changes his configuration:
1. **Analyze the diff:** Inspect upstream's `writeDefaultConfig()` to see the new keys.
2. **Define a new Epoch:** Add a new heuristic to `detectConfigEpoch()` (e.g., `if (doc.containsKey("new_feature")) return EPOCH_GADEC_V3;`).
3. **Write the mapping:** Add an isolated mapping block in the translator function to pipe `new_feature` into our modern schema.
4. **Deploy:** The next user who performs an OTA update from the original developer's firmware to ours will automatically have their config fingerprinted, mapped directly to the latest v2.6+ schema without breaking a sweat, and our core parser remains perfectly clean.

## User Review Required

Does this decoupled *Sniff -> Translate to Latest* pipeline align with the level of resilience you are looking for? Should we begin separating the legacy fallback fields (`altCrs`, `tubeId` etc.) out of `configManager.cpp`'s main parser and into an explicit `.agents/workflows/` or separate class implementation for architectural cleanliness?

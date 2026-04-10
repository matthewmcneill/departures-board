# Architecture Decision Record: PlatformIO MCP Library Parsing

## Context
When executing the `listInstalledLibraries` verb via the PlatformIO MCP, the original code statically asserted that `pio lib list --json-output` returned an explicitly formatted subset matching the registry `searchLibraries` array output. In reality:
1. `pio lib list --json-output` returns a Record mapping environments to arrays of dependency structures (`{ "/path/to/env": [...] }`).
2. Locally installed library metadata entirely omit the `id` field, which was previously strictly mandated by `z.number()`.

## Decisions Made
- Updated `platformio-mcp/src/types.ts` to expose `LibrariesObjectSchema = z.record(z.string(), z.array(LibraryInfoSchema))`.
- Lowered strictly bound `id` in `LibraryInfoSchema` to `id?: number` since local dependencies do not map to remote registry IDs.
- Converted `src/tools/libraries.ts` target verb to parse against `z.union([LibrariesArraySchema, LibrariesObjectSchema])`.
- Aggregated and deduplicated the resulting payloads across environments into a flattened subset to preserve front-end dashboard simplicity without breaking Zod schema constraints. 

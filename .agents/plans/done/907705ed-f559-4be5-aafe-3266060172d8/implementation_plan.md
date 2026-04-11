# Persistent Integrated Terminals for Dev Tasks

The goal is to provide a more integrated experience for managing long-running background tasks (Serial Monitor, Dev Server) by leveraging VS Code's built-in Task system. This allows the user to have persistent terminal tabs within their IDE that they can easily see and interact with.

## Proposed Changes

### VS Code Integration

#### [NEW] [tasks.json](.vscode/tasks.json)
Create a VS Code tasks configuration defining two long-running tasks:
1. **Watch Serial Monitor**: Runs `pio device monitor` with auto-selection of the environment.
2. **Start Dev Server**: Runs the Python development server for the layout simulator.

### Workflows

#### [NEW] [monitor.md](.agents/workflows/monitor.md)
Update the `/monitor` workflow to guide the user to the VS Code task, or trigger it via the `code` CLI (if possible) or informative message.

#### [NEW] [dev-server.md](.agents/workflows/dev-server.md)
Update the `/dev-server` workflow to start the integrated server task.


## Verification Plan

### Manual Verification
1. Run `/monitor` and verify a new Terminal window opens with the `pio device monitor`.
2. Run `/dev-server` and verify a new Terminal window opens running the Python server.
3. Run `/flash-test` and verify it builds, flashes, and then spawns a new monitor window.

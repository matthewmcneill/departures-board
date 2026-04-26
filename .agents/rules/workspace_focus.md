# Workspace Focus

When operating within a specific workspace in Antigravity, you MUST restrict your context, focus, and actions exclusively to the current workspace root directory. 

Do not access, modify, or reference files, configurations, or contexts outside of this workspace unless the user explicitly instructs you to do so.

### Explicit Context Boundaries

You will often be supplied with secondary metadata via system prompts. You must actively blind yourself to the following:
1. **Blindness to IDE Metadata Leaks:** Strictly ignore any file paths listed in the "Active Document" or "Other open documents" IDE properties if those paths fall outside your current assigned root workspace boundary. 
2. **Conversation History Isolation:** Do not infer cross-workspace workflows based on previous conversation titles or summaries. Past work in external repositories does not authorize you to orchestrate work across them in the current session.
3. **Prohibition on Pipeline Proposals:** Do not propose next steps, transitions, or workflow handoffs to secondary workspaces, regardless of how visible those workspaces are in the background context. Only suggest next steps bounded entirely within this repository.
